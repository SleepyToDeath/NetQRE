#!/usr/bin/ruby
require "csv"
require "time"
require 'rubygems'
require 'pcaprub'
require 'packetfu'
require './ppacket'

def parse_time(csv, is_afternoon)
  csv.each do |row|
    row[6] = Time.parse(row[6]).to_i + (is_afternoon ? (3600*11) : (-3600))
  end
end

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
      STDERR.puts counter
      STDERR.puts time.to_i
    end
  end
  return traffic
end

class FFlowSeq
  attr_accessor :seq, :ptr
  def initialize
    @seq = []
    @ptr = 0
  end
end

class TTraffic
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
        @flows[sig].seq << FFlow.new(sig, time, row[-1])
      else
        @flows[sig] = FFlowSeq.new
        @flows[sig].seq << FFlow.new(sig, time, row[-1])
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
          pkt.time_since_last = time - ff.ed_time unless ff.pkts.empty?  
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
            pkt.time_since_last = time - ff.ed_time unless ff.pkts.empty?  
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

class CICIDS2017Source < DDataSource

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

  def get_pos(csv)
    pos = []
    csv.each do |row|
      if row[0] =~ /\d/ and row[-1] != "BENIGN" then
        pos << row
      end
    end
    return pos
  end


  def init_source
    csv = CSV.read($pos_csv)
    pos = get_pos(csv)
    parse_time(pos, ($pos_csv.include? "Afternoon"))
    STDERR.puts "label starting time: #{pos.min{|row| row[6]}[6]}"
    STDERR.puts "label ending time: #{pos.max{|row| row[6]}[6]}"
#    write_pos_file(pos, $pos_csv)
    STDERR.puts "Positive: #{pos.size}/#{csv.size}"

    STDERR.puts pos.group_by{ |row| row[-1] }.map{ |k, v| k }.to_s

    traffic = []
    threads = []
    combine_lock = Mutex.new
    caps = []
    for file_name in $pos_pcap do
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
    tt = TTraffic.new(pos, traffic, 1200)

    uncovered = 0
    tt.flows.each do |_, fs|
      fs.seq.each do |ff|
        if ff.pkts.empty?
          uncovered += 1
        end
      end
    end

    STDERR.puts "#{tt.negative_count}/#{traffic.size} negative packets"
    STDERR.puts "#{tt.abnormal_count}/#{traffic.size} abnormal packets"
    STDERR.puts "#{tt.crazy_count}/#{pos.size} missing vectors"
    STDERR.puts "#{uncovered - tt.crazy_count}/#{pos.size} uncovered vectors"

    attack_flows = []

    tt.flows.each do |_, seq|
      seq.seq.each do |flow|
        attack_flows << flow
      end
    end

    @output_flows = []
    attack_flows.group_by { |flow| flow.attack_type }.each do |type, flow_bucket|
      STDERR.puts type
      @output_flows << flow_bucket.select{ |flow| not flow.pkts.empty? }
    end
      
    @source = @output_flows[$wanted_type].each 
  end
    
  def next_flow
    return @source.next
  end

end


class CICIDS2017SourceNeg < DDataSource

  def get_neg(csv)
    neg = []
    csv.each do |row|
      if row[0] =~ /\d/ and row[-1] == "BENIGN" then
        neg << row
      end
    end
    return neg
  end


  def init_source
    csv = CSV.read($neg_csv)
    neg = get_neg(csv)
    parse_time(neg, ($neg_csv.include? "Afternoon"))
    STDERR.puts "label starting time: #{neg.min{|row| row[6]}[6]}"
    STDERR.puts "label ending time: #{neg.max{|row| row[6]}[6]}"
    STDERR.puts "negative: #{neg.size}/#{csv.size}"

    STDERR.puts neg.group_by{ |row| row[-1] }.map{ |k, v| k }.to_s

    traffic = []
    threads = []
    combine_lock = Mutex.new
    caps = []
    for file_name in $neg_pcap do
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
    tt = TTraffic.new(neg, traffic, 1200)

    uncovered = 0
    tt.flows.each do |_, fs|
      fs.seq.each do |ff|
        if ff.pkts.empty?
          uncovered += 1
        end
      end
    end

    STDERR.puts "#{tt.negative_count}/#{traffic.size} positive packets"
    STDERR.puts "#{tt.abnormal_count}/#{traffic.size} abnormal packets"
    STDERR.puts "#{tt.crazy_count}/#{neg.size} missing vectors"
    STDERR.puts "#{uncovered - tt.crazy_count}/#{neg.size} uncovered vectors"

    normal_flows = []

    tt.flows.each do |_, seq|
      seq.seq.each do |flow|
        normal_flows << flow
      end
    end

    normal_flows.select!{ |flow| (not flow.pkts.empty?) and (flow.pkts.length < $max_flow_length) }

    @source = normal_flows.each 
  end
    
  def next_flow
    return @source.next
  end
end
