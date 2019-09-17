#!/usr/bin/ruby
require "csv"
require "time"
require 'rubygems'

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


def main
	csv = CSV.read(ARGV[0])
	types = csv.group_by{ |row| row[-1] }
	pos = []
	types.each do |type, rows|
		if not type.include? " Label" and not type.include? "BENIGN"
			pos += rows
		end
	end

	write_pos_file(pos, ARGV[0])

end

main
