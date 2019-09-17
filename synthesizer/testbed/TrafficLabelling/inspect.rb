#!/usr/bin/ruby
require "csv"
require "time"
require 'rubygems'

def is_afternoon(time)
	beginning_of_the_day = Time.new(time.year, time.month, time.day, time.zone)
	return time - beginning_of_the_day < 8*3600
end

def parse_time(csv)
  csv.each do |row|
		if row[0] =~ /\d/ then
			t = Time.parse(row[6])
			row[6] = t.to_i + (is_afternoon(t) ? (3600*11) : (-3600))
		end
  end
end

def row2time(row)
  return row[6]
end

def main
	csv = CSV.read(ARGV[0])
	parse_time(csv)
	types = csv.group_by{ |row| row[-1] }
	types.each do |type, rows|
		rows.sort!{|row| row[6]}
		STDERR.puts "Type: #{type}"
		STDERR.puts "Label starting time : #{rows[0][6]}"
		STDERR.puts "Label ending time   : #{rows[-1][6]}"
		STDERR.puts "Ratio: #{rows.size}/#{csv.size}"
	end

end

main
