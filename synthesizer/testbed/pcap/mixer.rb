#!/usr/bin/ruby

require 'rubygems'
require 'pcaprub'
require 'packetfu'

$feature_number = 20
$iter_vector = [1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1]
#$iter_vector = [1, 1, 1, 1, 1]

def extract_info(pkt)
  fv = Array.new($feature_number, 0)
  if pkt.is_ip?
    fv[0] = pkt.ip_src
    fv[1] = pkt.ip_dst
    fv[5] = pkt.ip_frag
    fv[6] = pkt.ip_id
    fv[7] = pkt.ip_len
    fv[8] = pkt.ip_tos
    fv[9] = pkt.ip_ttl
    if pkt.is_tcp?
      fv[2] = pkt.tcp_src
      fv[3] = pkt.tcp_dst
      fv[4] = 1
      fv[10] = pkt.tcp_flags.syn
      fv[11] = pkt.tcp_flags.ack
      fv[12] = pkt.tcp_flags.fin
      fv[13] = pkt.tcp_flags.rst
      fv[14] = pkt.tcp_flags.psh
      fv[15] = pkt.tcp_flags.urg
      fv[16] = pkt.tcp_seq
      fv[17] = pkt.tcp_win
      fv[18] = pkt.tcp_hlen
    elsif pkt.is_udp?
      fv[2] = pkt.udp_sport
      fv[3] = pkt.udp_dport
      fv[4] = 2
      fv[19] = pkt.udp_len
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

		if fv[10] > 1
			STDERR.puts "Impossible value! #{fv[8]}"
		end
  end
  return fv
end

def save_fs(fs)
  fs.each do |fv|
    puts fv.join("\t")
  end
  puts ""
end

bad_file = ARGV[0]
good_file = ARGV[1]
sample_number = ARGV[2].to_i # number of examples
sample_size = ARGV[3].to_i # number of bad packets in each example
ratio = ARGV[4].to_i # ratio of good to bad packets in each example
starting_count_bad = ARGV[5].to_i
starting_count_good = ARGV[6].to_i
# So each example contains sample_size * (ratio + 1) packets

puts "#{sample_number} #{sample_size*(ratio+1)} #{$feature_number}"
puts $iter_vector.join(' ')

trigger = false
sample_remaning = 0
counter = 0
fs = []
srand

bad_cap = PCAPRUB::Pcap.open_offline(bad_file)
good_cap = PCAPRUB::Pcap.open_offline(good_file)

enum_bad = bad_cap.to_enum

next_bad_sample = ratio

(1..starting_count_bad).each do
	enum_bad.next
end

# extract packet info
good_cap.each_packet do |raw_good|
  counter += 1
  if counter%10000 == 0
    STDERR.puts counter
		STDERR.puts raw_good.time.class
		STDERR.puts Time.at(raw_good.time).to_s
  end

	if counter == starting_count_good
		trigger = true
	end

  if trigger

		if ratio > 0
			pkt = PacketFu::Packet.parse raw_good.data
			if !pkt.is_ip?
				next
			end
			fv = extract_info(pkt)
			fs << fv
		end

		next_bad_sample -= 1

		if next_bad_sample <= 0

			raw_bad = enum_bad.next
			pkt = PacketFu::Packet.parse raw_bad
			while !pkt.is_ip? 
				raw_bad = enum_bad.next
				pkt = PacketFu::Packet.parse raw_bad
			end

			if sample_remaning == 0
				sample_remaning = sample_size
				sample_number -=1
				STDERR.puts sample_number
			end

			if sample_remaning > 0
				sample_remaning -= 1
				pkt = PacketFu::Packet.parse raw_bad
				fv = extract_info(pkt)
				fs << fv
			end

			if sample_remaning == 0
				save_fs(fs)
				STDERR.puts "sample size #{fs.size}"
				fs = []
			end

			if sample_number <= 0
				return
			end

			next_bad_sample = ratio# + rand(ratio / 10)
			#puts next_bad_sample
			
		end

  end
end

  
 
