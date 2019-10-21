
require "csv"
require "time"
require 'rubygems'
require './ppacket'

class KiPacket
	attr_accessor :features
	def initialize(csvf_row)
		@features = csvf_row
	end

	def to_feature
		return @features.map{ |s| s.to_f.to_i }
	end

	def self.to_feature_type
		return Array.new(115,0)
	end
end

class KitsuneSourcePos < DDataSource
	def init_source
		@csv = CSV.foreach($pos_csv)
		@csvf = CSV.foreach($pos_csvf)
		@csv.next
		@counter = 0
	end

	def next_flow
		p = nil
		l = ['0','0']
		while l[1].to_i != 1 do
			p = KiPacket.new(@csvf.next)
			l = @csv.next
			@counter += 1
			if @counter % 10000 == 0 then
				STDERR.puts @counter
			end
		end
		f = FFlow.new(nil, nil, nil)
		f.pkts << p
		return f
	end
end

class KitsuneSourceNeg < DDataSource
	def init_source
		@csv = CSV.foreach($neg_csv)
		@csvf = CSV.foreach($neg_csvf)
		@csv.next
		@counter = 0
	end

	def next_flow
		p = nil
		l = ['1','1']
		while l[1].to_i != 0 do
			p = KiPacket.new(@csvf.next)
			l = @csv.next
			@counter += 1
			if @counter % 10000 == 0 then
				STDERR.puts @counter
			end
		end
		f = FFlow.new(nil, nil, nil)
		f.pkts << p
		return f
	end
end

