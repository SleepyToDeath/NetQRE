#!/usr/bin/ruby

require './ppacket'
require './cicids2017'

class MixerConfig
  attr_reader :sample_count, :batch_size

  def initialize(pos, neg, batch, count)
    @pos_ratio = pos
    @neg_ratio = neg
    @batch_size = batch
    @sample_count = count
  end

  def next_is_pos?
    return ( rand(@pos_ratio + @neg_ratio) < @pos_ratio )
  end
end

class Mixer
  def initialize(config, pos_source, neg_source)
    @pos_source = pos_source
    @neg_source = neg_source

    @pos_source.init_source
    @neg_source.init_source

    @pos_handle = pos_source.next_flow.pkts.each
    @neg_handle = neg_source.next_flow.pkts.each

    @config = config

    @counter = 0

    skip
  end

  def output
    write_header
    (1..@config.sample_count).each do |_|
      @counter = 0
      while @counter < @config.batch_size do
        pkt = next_packet
        break unless @counter < @config.batch_size
        write_packet(pkt)
      end
      puts ""
    end
  end

  private

  def skip
    (1..$pos_skip).each do |_|
      @pos_handle = @pos_source.next_flow.pkts.each
    end
    (1..$neg_skip).each do |_|
      @neg_handle = @neg_source.next_flow.pkts.each
    end
  end

  def next_packet
    get_packet = ->(source, handle) do
      begin
        return handle, handle.next
      rescue StopIteration
        @counter += 1
        handle = source.next_flow.pkts.each
        if @counter < @config.batch_size
          return handle, handle.next
        else
          return handle, nil 
        end
      end
    end

    ret = nil
    if @config.next_is_pos? then
      @pos_handle, ret = get_packet.call(@pos_source, @pos_handle)
    else
      @neg_handle, ret = get_packet.call(@neg_source, @neg_handle)
    end
    return ret
  end

  def write_header
    puts "#{@config.sample_count} #{PPacket.to_feature_type.length}"
    puts PPacket.to_feature_type.join(" ")
  end

  def write_packet(pkt)
    puts pkt.to_feature.join(" ")
  end
end

#file names
=begin
#DDoS
$pos_csv = './csv/Friday-WorkingHours-Afternoon-DDos.pcap_ISCX.pos.csv'
$pos_pcap = ['./dataset/Friday-WorkingHours-split.pcap16']
$wanted_type = 0
=end

=begin
$pos_csv = './csv/Wednesday-workingHours.pcap_ISCX.pos.csv'
$pos_pcap = [
'./dataset/Wednesday-WorkingHours.pcap.split38'
]
$wanted_type = 3
=end

$pos_csv = './csv/Friday-WorkingHours-Afternoon-PortScan.pcap_ISCX.pos.csv'
$pos_pcap = [
'./dataset/Friday-WorkingHours.pcap.split22',
]
$wanted_type = 0

$neg_csv = './csv/Monday-WorkingHours.pcap_ISCX.csv'
$neg_pcap = ['./dataset/Monday-WorkingHours.pcap.split12']
$max_flow_length = 1000

$pos_skip = 2000
$neg_skip = 0

#pos_ratio, neg_ratio, batch, count
small_simple_pure_pos = MixerConfig.new(1, 0, 1, 10)
small_simple_pure_neg = MixerConfig.new(0, 1, 1, 10)

single_pure_pos = MixerConfig.new(1, 0, 1, 50)
combined_pure_pos = MixerConfig.new(1, 0, 20, 50)
single_pure_neg = MixerConfig.new(0, 1, 1, 50)
combined_pure_neg = MixerConfig.new(0, 1, 8, 50)

combined_few_pos = MixerConfig.new(1, 0, 1, 3)

dummy_source = DDataSource.new

pos_source = CICIDS2017Source.new
neg_source = dummy_source

#pos_source = dummy_source
#neg_source = CICIDS2017SourceNeg.new

mx = Mixer.new(combined_pure_pos, pos_source, neg_source)
mx.output

