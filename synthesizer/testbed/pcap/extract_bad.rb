#!/usr/bin/ruby

require 'rubygems'
require 'pcaprub'
require 'packetfu'

$feature_number = 5
#$iter_vector = [1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1]
$iter_vector = [1, 1, 1, 1, 1]

def extract_info(pkt)
  fv = Array.new($feature_number, 0)
  if pkt.is_ip?
    fv[0] = pkt.ip_src
    fv[1] = pkt.ip_dst
=begin
    fv[5] = pkt.ip_frag
    fv[6] = pkt.ip_id
    fv[7] = pkt.ip_len
    fv[8] = pkt.ip_tos
    fv[9] = pkt.ip_ttl
=end
    if pkt.is_tcp?
      fv[2] = pkt.tcp_src
      fv[3] = pkt.tcp_dst
      fv[4] = 1
=begin
      fv[10] = pkt.tcp_flags.syn
      fv[11] = pkt.tcp_flags.ack
      fv[12] = pkt.tcp_flags.fin
      fv[13] = pkt.tcp_flags.rst
      fv[14] = pkt.tcp_flags.psh
      fv[15] = pkt.tcp_flags.urg
      fv[16] = pkt.tcp_seq
      fv[17] = pkt.tcp_win
      fv[18] = pkt.tcp_hlen
=end
    elsif pkt.is_udp?
      fv[2] = pkt.udp_sport
      fv[3] = pkt.udp_dport
      fv[4] = 2
=begin
      fv[19] = pkt.udp_len
=end
    end
=begin
		if fv[0] > fv[1]
			tmp = fv[0]
			fv[0] = fv[1]
			fv[1] = tmp
			tmp = fv[2]
			fv[2] = fv[3]
			fv[3] = tmp
		end
=end
  end
  return fv
end

def save_fs(fs)
  fs.each do |fv|
    puts fv.join(' ')
  end
  puts ""
end

pcap_file = ARGV[0]
sample_number = ARGV[1].to_i
sample_size = ARGV[2].to_i
starting_src_ip = ARGV[3].split(".").map(&:to_i).pack('CCCC').unpack('N')[0]
starting_src_port = ARGV[4].to_i
starting_dst_ip = ARGV[5].split(".").map(&:to_i).pack('CCCC').unpack('N')[0]
starting_dst_port = ARGV[6].to_i
starting_timestamp = ARGV[7]

STDERR.puts "#{starting_src_ip} #{starting_src_port} #{starting_dst_ip} #{starting_dst_port}"

puts "#{sample_number} #{sample_size} #{$feature_number}"
puts $iter_vector.join(' ')

trigger = false
trigger_time = false
sample_remaning = 0
counter = 0
fs = []

cap = PCAPRUB::Pcap.open_offline(pcap_file)

# extract packet info
cap.each_packet do |raw|
  counter += 1
  if counter%10000 == 0
    STDERR.puts counter
		STDERR.puts raw.time.class
		STDERR.puts Time.at(raw.time).to_s
  end

	if Time.at(raw.time).to_s.include? starting_timestamp
		trigger_time = true
	end

	if trigger_time
		pkt = PacketFu::Packet.parse raw.data
		if pkt.is_ip?
			#puts [pkt.ip_src].pack('N').unpack('CCCC').join('.') + "->" + [pkt.ip_dst].pack('N').unpack('CCCC').join('.')
			if pkt.ip_src == starting_src_ip and pkt.ip_dst == starting_dst_ip
				port_src = 0
				port_dst = 0
				if pkt.is_tcp?
					port_src = pkt.tcp_src
					port_dst = pkt.tcp_dst
				elsif pkt.is_udp?
					port_src = pkt.udp_sport
					port_dst = pkt.udp_dport
				end
			#	if port_src == starting_src_port and port_dst == starting_dst_port
				trigger = true
			#	end
			end
		end
	end

=begin
  if Time.at(raw.time).to_s.include? starting_timestamp
    trigger = true
  end
=end

  if trigger

    if sample_remaning == 0
      sample_remaning = sample_size
      sample_number -=1
			STDERR.puts sample_number
      fs = []
    end

    if sample_remaning > 0
      sample_remaning -= 1
			pkt = PacketFu::Packet.parse raw.data
      fv = extract_info(pkt)
      fs << fv
    end

    if sample_remaning == 0
      save_fs(fs)
      if sample_number <= 0
        return
      end
    end

  end
end

  
 
