#!/usr/bin/ruby

require 'rubygems'
require 'pcaprub'
require 'packetfu'

$sample_count_down = 0
$sample_interval = 10000
$save_file
$feature_number = 20
$iter_vector = [1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1]

def reset_count_down
  $sample_count_down = rand($sample_interval/100) + $sample_interval
end

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
#$save_file = File.open(ARGV[3], 'w')
puts "#{sample_number} #{sample_size} #{$feature_number}"
puts $iter_vector.join(' ')

sample_remaning = 0
counter = 0
fs = []
srand
reset_count_down

cap = PCAPRUB::Pcap.open_offline(pcap_file)

# extract packet info
cap.each_packet do |raw|
  counter += 1
  if counter%10000 == 0
    STDERR.puts counter
  end

  $sample_count_down -= 1
  if $sample_count_down == 0
    reset_count_down
    sample_remaning = sample_size
    sample_number -=1
    fs = []
  end

  if sample_remaning > 0
    sample_remaning -= 1
    pkt = PacketFu::Packet.parse raw.data
    fv = extract_info(pkt)
    fs << fv
    if sample_remaning == 0
      save_fs(fs)
      if sample_number <= 0
        return
      end
    end
  end
end

  
 
