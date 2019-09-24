
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

		csv_enum = csv.each
		csv_enum.next
		counter = 0
		last_time = 0
		@cache = []
		
		cap.each_packet do |raw|
			break unless counter < $pos_prepare_num

			flag = csv_enum.next[1].to_i
			if (flag == 1) then
				counter += 1
				if (counter % 10000 == 0) then STDERR.puts counter end
				flow = FFlow.new(nil, nil, nil)
				if (counter >= $pos_skip) then
					time = Time.at(raw.time)
					pkt = PacketFu::Packet.parse raw.data
					pp = (pkt.is_ip? or pkt.is_ipv6?) ? PPacket.new(time.to_i, pkt) : PPacket.new(time.to_i)
					if last_time != 0 then
						pp.time_since_last = time - last_time
					end
					last_time = time
					flow.pkts << pp
				end
				@cache << flow
			end
		end

		STDERR.puts "#{counter} positive packets collected"

		cap.close

		@cache_enum = @cache.each
	end

	def next_flow
		return @cache_enum.next
	end
end

class KitsuneSourceNeg < DDataSource
	def init_source
    csv = CSV.read($ki_csv)
		cap = PCAPRUB::Pcap.open_offline($ki_pcap)

		csv_enum = csv.each
		csv_enum.next
		counter = 0
		last_time = 0
		@cache = []

		cap.each_packet do |raw|
			break unless counter < $neg_prepare_num

			flag = csv_enum.next[1].to_i
			if (flag == 0) then
				counter += 1
				if (counter % 10000 == 0) then STDERR.puts counter end
				flow = FFlow.new(nil, nil, nil)
				if (counter >= $neg_skip) then
					time = Time.at(raw.time)
					pkt = PacketFu::Packet.parse raw.data
					pp = (pkt.is_ip? or pkt.is_ipv6?) ? PPacket.new(time.to_i, pkt) : PPacket.new(time.to_i)
					if last_time != 0 then
						pp.time_since_last = time - last_time
					end
					last_time = time
					flow.pkts << pp
				end
				@cache << flow
			end
		end

		STDERR.puts "#{counter} negative packets collected"

		cap.close

		@cache_enum = @cache.each
	end

	def next_flow
		return @cache_enum.next
	end
end

