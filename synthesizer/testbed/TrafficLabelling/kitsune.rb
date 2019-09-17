
require "csv"
require "time"
require 'rubygems'
require 'pcaprub'
require 'packetfu'
require './ppacket'

class KitsuneSourcePos < DDataSource
	def init_source
    csv = CSV.read($ki_csv)
		cap = PCAPRUB::Pcap.open_offline($ki_pcap)
		cap_enum = cap.each_packet
		csv_enum = csv.each
	end

	def next_flow
		last_time = 0
		while true
			flag = csv.next[0]
			raw = cap_enum.next
			if (flag == 1) then
				time = Time.at(raw.time)
				pkt = PacketFu::Packet.parse raw.data
				pp = (pkt.is_ip? or pkt.is_ipv6?) ? PPacket.new(time.to_i, pkt) : PPacket.new(time.to_i)
				if last_time != 0 then
					pp.time_since_last = time - last_time
				end
				last_time = time
				flow = FFlow.new(nil, nil, nil)
				flow.pkt << pp
				return flow
			end
		end
	end
end

class KitsuneSourceNeg < DDataSource
	def init_source
    csv = CSV.read($ki_csv)
		cap = PCAPRUB::Pcap.open_offline($ki_pcap)
		cap_enum = cap.each_packet
		csv_enum = csv.each
	end

	def next_flow
		last_time = 0
		while true
			flag = csv.next[0]
			raw = cap_enum.next
			if (flag == 0) then
				time = Time.at(raw.time)
				pkt = PacketFu::Packet.parse raw.data
				pp = (pkt.is_ip? or pkt.is_ipv6?) ? PPacket.new(time.to_i, pkt) : PPacket.new(time.to_i)
				if last_time != 0 then
					pp.time_since_last = time - last_time
				end
				last_time = time
				flow = FFlow.new(nil, nil, nil)
				flow.pkt << pp
				return flow
			end
		end
	end
end

