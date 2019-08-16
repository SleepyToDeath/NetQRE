#!/usr/bin/ruby
require "csv"
require "time"
require 'rubygems'
require 'pcaprub'
require 'packetfu'

def ip2int (ip)
  i = 0
  ip.split('.').each{ |s| i = i * 256 + s.to_i }
  return i
end

def row2flow_signature(row)
  return [ip2int(row[1]), row[2].to_i, ip2int(row[3]), row[4].to_i, row[5].to_i]
end

def row2time(row)
  return row[6]
end

class PPacket
  attr_accessor :pkt, :time, :src_ip, :dst_ip, :src_port, :dst_port, :protocol, :valid, :label
  def initialize(time, pkt = nil)
#    puts time
#    puts pkt.class

    @label = 0

    if pkt == nil
      @valid = false
      return
    end

    @valid = true
    @time = time
    pkt = pkt
    if pkt.is_ipv6?
      @src_ip = pkt.ipv6_src
      @dst_ip = pkt.ipv6_dst
    else
      @src_ip = pkt.ip_src
      @dst_ip = pkt.ip_dst
    end
    @protocol = 0
    if pkt.is_tcp?
      @src_port = pkt.tcp_src
      @dst_port = pkt.tcp_dst
      @protocol = 6
    elsif pkt.is_udp?
      @src_port = pkt.udp_sport
      @dst_port = pkt.udp_dport
      @protocol = 17
    end
  end

  def flow_signature
    return [@src_ip, @src_port, @dst_ip, @dst_port, @protocol]
  end
end

class FFlow
  attr_accessor :sig, :pkts, :est_time, :op_time, :ed_time
  def initialize(sig, est_time)
    @sig = sig
    @est_time = est_time
    @op_time = est_time
    @ed_time = est_time
    @pkts = []
  end
end

class FFlowSeq
  attr_accessor :seq, :ptr
  def initialize
    @seq = []
    @ptr = 0
  end
end

class TTrafic
  attr_accessor :csv, :traffic, :timeout, :flows, :negative_count, :abnormal_count, :crazy_count
  def initialize(csv, traffic, timeout)
    @csv = csv
    @traffic = traffic
    @timeout = timeout
    @negative_count = 0
    @abnormal_count = 0
    @crazy_count = 0
    @flows = {}
    process_csv
    process_traffic
  end

  def process_csv
    @csv.each do |row|
      sig = row2flow_signature(row)
      time = row2time(row)
      if @flows.has_key? sig
        @flows[sig].seq << FFlow.new(sig, time)
      else
        @flows[sig] = FFlowSeq.new
        @flows[sig].seq << FFlow.new(sig, time)
      end
    end

    @flows.each do |_, fs|
      fs.seq.sort_by! { |ff| ff.est_time }
    end
  end

  def process_traffic
    @traffic.each do |pkt|
      next unless pkt.valid
      sig = pkt.flow_signature
      time = pkt.time
      if @flows.has_key? sig
        fs = @flows[sig]
        if fs.ptr >= fs.seq.size
          @abnormal_count += 1
          next
        end
        ff = fs.seq[fs.ptr]
        if (ff.est_time - pkt.time).abs < @timeout  or  (ff.ed_time - pkt.time).abs < @timeout
          ff.pkts << pkt
          ff.op_time = ff.pkts[0].time
          ff.ed_time = pkt.time
          pkt.label = 1
        elsif pkt.time < ff.est_time
          @abnormal_count += 1
          next
        else
          fs.ptr += 1
          while fs.ptr < fs.seq.size and fs.seq[fs.ptr].est_time - pkt.time >= @timeout
            @crazy_count += 1
            fs.ptr += 1
          end
          if fs.ptr >= fs.seq.size
            @abnormal_count += 1
            next
          end
          ff = fs.seq[fs.ptr]
          if (ff.est_time - pkt.time).abs < @timeout  or  (ff.ed_time - pkt.time).abs < @timeout
            ff.pkts << pkt
            ff.op_time = ff.pkts[0].time
            ff.ed_time = pkt.time
            pkt.label = 1
          end
        end
      else
        @negative_count += 1
      end
    end
  end
end

def write_pos_file(pos, file_name)
  tmp = file_name.split('.')
  tmp[-1] = "pos"
  tmp << "csv"
  new_name = tmp.join '.'

  CSV.open(new_name , "wb") do |pos_file|
    pos.each do |row|
      pos_file << row
    end
  end
end

def parse_time(csv, is_afternoon)
  csv.each do |row|
    row[6] = Time.parse(row[6]).to_i + (is_afternoon ? (3600*11) : 0)
  end
end

def get_pos(csv)
  return csv.select do |row|
    row[0] != "Flow ID" and row[-1] != "BENIGN"
  end
end

def parse_packets(cap)
  traffic = []
  counter = 0
  cap.each_packet do |raw|
    counter += 1
    time = Time.at(raw.time)
    pkt = PacketFu::Packet.parse raw.data
    pp = (pkt.is_ip? or pkt.is_ipv6?) ? PPacket.new(time.to_i, pkt) : PPacket.new(time.to_i)
    traffic << pp
    if counter % 10000 == 0
      puts counter
      puts time.to_i
    end
  end
  return traffic
end

def main
  csv = CSV.read(ARGV[0])
  pos = get_pos(csv)
  parse_time(pos, (ARGV[0].include? "Afternoon"))
  puts "label starting time: #{pos.min{|row| row[6]}[6]}"
  puts "label ending time: #{pos.max{|row| row[6]}[6]}"
  write_pos_file(pos, ARGV[0])
  puts "Positive: #{pos.size}/#{csv.size}"

  traffic = []
  threads = []
  combine_lock = Mutex.new
  caps = []
  for file_name in ARGV[1..-1] do
    caps << PCAPRUB::Pcap.open_offline(file_name)
  end
  caps.each do |cap|
    threads << Thread.new do
      this_traffic = parse_packets(cap)
      combine_lock.synchronize do
        traffic += this_traffic
      end
    end
  end
  threads.each {|t| t.join}
  traffic.sort_by! { |p| p.time }
  tt = TTrafic.new(pos, traffic, 1200)

  uncovered = 0
  tt.flows.each do |_, fs|
    fs.seq.each do |ff|
      if ff.pkts.empty?
        uncovered += 1
      end
    end
  end

  puts "#{tt.negative_count}/#{traffic.size} negative packets"
  puts "#{tt.abnormal_count}/#{traffic.size} abnormal packets"
  puts "#{tt.crazy_count}/#{pos.size} missing vectors"
  puts "#{uncovered - tt.crazy_count}/#{pos.size} uncovered vectors"
end



main
