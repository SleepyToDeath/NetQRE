#!/usr/bin/ruby

require './ppacket'
require './cicids2017'
require './kitsune_csv'

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

    @neg_source.init_source
    @pos_source.init_source

    @pos_handle = pos_source.next_flow.pkts.each
    @neg_handle = neg_source.next_flow.pkts.each

    @config = config

    @counter = 0
		@feature_types = []

    skip
  end
	
	def set_feature_types ( types )
		@feature_types = types
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
			step
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

	def step
    (1..$pos_step_size).each do |_|
      @pos_handle = @pos_source.next_flow.pkts.each
		end
    (1..$neg_step_size).each do |_|
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
    puts "#{@config.sample_count} #{@feature_types.length}"
    puts @feature_types.join(" ")
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

#pos_ratio, neg_ratio, batch, count
small_simple_pure_pos = MixerConfig.new(1, 0, 1, 10)
small_simple_pure_neg = MixerConfig.new(0, 1, 1, 10)

single_pure_pos = MixerConfig.new(1, 0, 1, 50)
combined_pure_pos = MixerConfig.new(1, 0, 20, 50)
single_pure_neg = MixerConfig.new(0, 1, 1, 50)
combined_pure_neg = MixerConfig.new(0, 1, 20, 50)

combined_few_pos = MixerConfig.new(1, 0, 1, 3)

dummy_source = DDataSource.new

bulk_pure_pos = MixerConfig.new(1, 0, $batch_size, $sample_count)
bulk_pure_neg = MixerConfig.new(0, 1, $batch_size, $sample_count)


#================ for CICIDS2017 =================

=begin
#PortScan test
$pos_csv = 
'./csv/Friday-WorkingHours-Afternoon-PortScan.pcap_ISCX.pos.csv'
$pos_pcap = [
'./dataset/Friday-WorkingHours.pcap.split22'
]
$wanted_type = 0
$batch_size = 0
$pos_skip = 10000
$neg_skip = 0
$neg_step_size = 0
$max_flow_length = 1000
pos_source = CICIDS2017Source.new
neg_source = dummy_source
combined_pure_pos = MixerConfig.new(1, 0, 10, 150)
mx = Mixer.new(combined_pure_pos, pos_source, neg_source)
=end

=begin
#PortScan train
$pos_csv = 
'./csv/Friday-WorkingHours-Afternoon-PortScan.pcap_ISCX.pos.csv'
$pos_pcap = [
'./dataset/Friday-WorkingHours.pcap.split22'
]
$wanted_type = 0
$batch_size = 0
$pos_skip = 0
$neg_skip = 0
$neg_step_size = 0
$max_flow_length = 1000
pos_source = CICIDS2017Source.new
neg_source = dummy_source
combined_pure_pos = MixerConfig.new(1, 0, 10, 150)
mx = Mixer.new(combined_pure_pos, pos_source, neg_source)
=end

=begin
#DDoS test
$pos_csv = 
'./csv/Friday-WorkingHours-Afternoon-DDos.pcap_ISCX.pos.csv'
$pos_pcap = [
'./dataset/Friday-WorkingHours.pcap.split25'
]
$wanted_type = 0
$batch_size = 0
$pos_skip = 0
$neg_skip = 0
$neg_step_size = 0
$max_flow_length = 1000
pos_source = CICIDS2017Source.new
neg_source = dummy_source
combined_pure_pos = MixerConfig.new(1, 0, 10, 150)
mx = Mixer.new(combined_pure_pos, pos_source, neg_source)
=end

=begin
#DDoS train
$pos_csv = 
'./csv/Friday-WorkingHours-Afternoon-DDos.pcap_ISCX.pos.csv'
$pos_pcap = [
'./dataset/Friday-WorkingHours.pcap.split24'
]
$wanted_type = 0
$batch_size = 0
$pos_skip = 0
$neg_skip = 0
$neg_step_size = 0
$max_flow_length = 1000
pos_source = CICIDS2017Source.new
neg_source = dummy_source
combined_pure_pos = MixerConfig.new(1, 0, 10, 150)
mx = Mixer.new(combined_pure_pos, pos_source, neg_source)
=end

=begin
#Bot test
$pos_csv = 
'./csv/Friday-WorkingHours-Morning.pcap_ISCX.pos.csv'
$pos_pcap = [
'./dataset/Friday-WorkingHours.pcap.split11',
'./dataset/Friday-WorkingHours.pcap.split12',
'./dataset/Friday-WorkingHours.pcap.split13',
'./dataset/Friday-WorkingHours.pcap.split14',
'./dataset/Friday-WorkingHours.pcap.split15',
'./dataset/Friday-WorkingHours.pcap.split16',
'./dataset/Friday-WorkingHours.pcap.split17',
'./dataset/Friday-WorkingHours.pcap.split18'
]
$wanted_type = 0
$batch_size = 0
$pos_skip = 500
$neg_skip = 0
$neg_step_size = 0
$max_flow_length = 1000
pos_source = CICIDS2017Source.new
neg_source = dummy_source
combined_pure_pos = MixerConfig.new(1, 0, 10, 100)
mx = Mixer.new(combined_pure_pos, pos_source, neg_source)
=end

=begin
#Bot train
$pos_csv = 
'./csv/Friday-WorkingHours-Morning.pcap_ISCX.pos.csv'
$pos_pcap = [
'./dataset/Friday-WorkingHours.pcap.split11',
'./dataset/Friday-WorkingHours.pcap.split12',
'./dataset/Friday-WorkingHours.pcap.split13',
'./dataset/Friday-WorkingHours.pcap.split14',
'./dataset/Friday-WorkingHours.pcap.split15',
'./dataset/Friday-WorkingHours.pcap.split16',
'./dataset/Friday-WorkingHours.pcap.split17',
'./dataset/Friday-WorkingHours.pcap.split18'
]
$wanted_type = 0
$batch_size = 0
$pos_skip = 0
$neg_skip = 0
$neg_step_size = 0
$max_flow_length = 1000
pos_source = CICIDS2017Source.new
neg_source = dummy_source
combined_pure_pos = MixerConfig.new(1, 0, 10, 50)
mx = Mixer.new(combined_pure_pos, pos_source, neg_source)
=end

=begin
#SSH patator test
$pos_csv = 
'./csv/Tuesday-WorkingHours.pcap_ISCX.pos.csv'
$pos_pcap = [
'./dataset/Tuesday-WorkingHours.pcap.split32',
'./dataset/Tuesday-WorkingHours.pcap.split33'
]
$wanted_type = 1
$batch_size = 0
$pos_skip = 1000
$neg_skip = 0
$neg_step_size = 0
$max_flow_length = 1000
pos_source = CICIDS2017Source.new
neg_source = dummy_source
combined_pure_pos = MixerConfig.new(1, 0, 10, 100)
mx = Mixer.new(combined_pure_pos, pos_source, neg_source)
=end

=begin
#SSH patator train
$pos_csv = 
'./csv/Tuesday-WorkingHours.pcap_ISCX.pos.csv'
$pos_pcap = [
'./dataset/Tuesday-WorkingHours.pcap.split32',
'./dataset/Tuesday-WorkingHours.pcap.split33'
]
$wanted_type = 1
$batch_size = 0
$pos_skip = 0
$neg_skip = 0
$neg_step_size = 0
$max_flow_length = 1000
pos_source = CICIDS2017Source.new
neg_source = dummy_source
combined_pure_pos = MixerConfig.new(1, 0, 10, 100)
mx = Mixer.new(combined_pure_pos, pos_source, neg_source)
=end


=begin
#FTP patator test
$pos_csv = 
'./csv/Tuesday-WorkingHours.pcap_ISCX.pos.csv'
$pos_pcap = [
'./dataset/Tuesday-WorkingHours.pcap.split25',
'./dataset/Tuesday-WorkingHours.pcap.split26'
]
$wanted_type = 0
$batch_size = 0
$pos_skip = 900
$neg_skip = 0
$neg_step_size = 0
$max_flow_length = 1000
pos_source = CICIDS2017Source.new
neg_source = dummy_source
combined_pure_pos = MixerConfig.new(1, 0, 10, 90)
mx = Mixer.new(combined_pure_pos, pos_source, neg_source)
=end


=begin
#FTP patator train
$pos_csv = 
'./csv/Tuesday-WorkingHours.pcap_ISCX.pos.csv'
$pos_pcap = [
'./dataset/Tuesday-WorkingHours.pcap.split25',
'./dataset/Tuesday-WorkingHours.pcap.split26'
]
$wanted_type = 0
$batch_size = 0
$pos_skip = 0
$neg_skip = 0
$neg_step_size = 0
$max_flow_length = 1000
pos_source = CICIDS2017Source.new
neg_source = dummy_source
combined_pure_pos = MixerConfig.new(1, 0, 10, 90)
mx = Mixer.new(combined_pure_pos, pos_source, neg_source)
=end


=begin
#DoS Hulk test
$pos_csv = './csv/Wednesday-workingHours.pcap_ISCX.pos.csv'
$pos_pcap = ['./dataset/Wednesday-WorkingHours.pcap.split28']
$wanted_type = 2
$batch_size = 0
$pos_skip = 2000
$neg_skip = 0
$neg_step_size = 0
$max_flow_length = 1000
pos_source = CICIDS2017Source.new
neg_source = dummy_source
combined_pure_pos = MixerConfig.new(1, 0, 10, 150)
mx = Mixer.new(combined_pure_pos, pos_source, neg_source)
=end


=begin
#DoS Hulk train
$pos_csv = './csv/Wednesday-workingHours.pcap_ISCX.pos.csv'
$pos_pcap = ['./dataset/Wednesday-WorkingHours.pcap.split28']
$wanted_type = 2
$batch_size = 0
$pos_skip = 0
$neg_skip = 0
$neg_step_size = 0
$max_flow_length = 1000
pos_source = CICIDS2017Source.new
neg_source = dummy_source
combined_pure_pos = MixerConfig.new(1, 0, 10, 150)
mx = Mixer.new(combined_pure_pos, pos_source, neg_source)
=end


=begin
#slowhttps train
$pos_csv = './csv/Wednesday-workingHours.pcap_ISCX.pos.csv'
$pos_pcap = ['./dataset/Wednesday-WorkingHours.pcap.split26']
$wanted_type = 1
$batch_size = 0
$pos_skip = 0
$neg_skip = 0
$neg_step_size = 0
$max_flow_length = 1000
pos_source = CICIDS2017Source.new
neg_source = dummy_source
combined_pure_pos = MixerConfig.new(1, 0, 10, 150)
mx = Mixer.new(combined_pure_pos, pos_source, neg_source)
=end

=begin
#slowhttps test
$pos_csv = './csv/Wednesday-workingHours.pcap_ISCX.pos.csv'
$pos_pcap = ['./dataset/Wednesday-WorkingHours.pcap.split26']
$wanted_type = 1
$batch_size = 0
$pos_skip = 1600
$neg_skip = 0
$neg_step_size = 0
$max_flow_length = 1000
pos_source = CICIDS2017Source.new
neg_source = dummy_source
combined_pure_pos = MixerConfig.new(1, 0, 10, 150)
mx = Mixer.new(combined_pure_pos, pos_source, neg_source)
=end

#slowloris train
=begin
$pos_csv = './csv/Wednesday-workingHours.pcap_ISCX.pos.csv'
$pos_pcap = ['./dataset/Wednesday-WorkingHours.pcap.split25']
$wanted_type = 0
$batch_size = 0
$pos_skip = 100
$neg_skip = 0
$neg_step_size = 0
$max_flow_length = 1000
pos_source = CICIDS2017Source.new
neg_source = dummy_source
combined_pure_pos = MixerConfig.new(1, 0, 10, 150)
mx = Mixer.new(combined_pure_pos, pos_source, neg_source)
=end

=begin
#slowloris test
$pos_csv = './csv/Wednesday-workingHours.pcap_ISCX.pos.csv'
$pos_pcap = ['./dataset/Wednesday-WorkingHours.pcap.split25']
$wanted_type = 0
$batch_size = 0
$pos_skip = 1700
$neg_skip = 0
$max_flow_length = 1000
pos_source = CICIDS2017Source.new
neg_source = dummy_source
combined_pure_pos = MixerConfig.new(1, 0, 10, 150)
mx = Mixer.new(combined_pure_pos, pos_source, neg_source)
=end

#neg train
=begin
$neg_csv = './csv/Monday-WorkingHours.pcap_ISCX.csv'
$neg_pcap = [
'./dataset/Monday-WorkingHours.pcap.split10',
'./dataset/Monday-WorkingHours.pcap.split11',
'./dataset/Monday-WorkingHours.pcap.split12',
'./dataset/Monday-WorkingHours.pcap.split13',
'./dataset/Monday-WorkingHours.pcap.split14',
'./dataset/Monday-WorkingHours.pcap.split15',
]
$wanted_type = 0
$max_flow_length = 200
$pos_skip = 0
$neg_skip = 0
$neg_step_size = 0
pos_source = dummy_source
neg_source = CICIDS2017SourceNeg.new
combined_pure_neg = MixerConfig.new(0, 1, 10, 1000)
mx = Mixer.new(combined_pure_neg, pos_source, neg_source)
=end

#neg test
=begin
$neg_csv = './csv/Monday-WorkingHours.pcap_ISCX.csv'
$neg_pcap = [
'./dataset/Monday-WorkingHours.pcap.split10',
'./dataset/Monday-WorkingHours.pcap.split11',
'./dataset/Monday-WorkingHours.pcap.split12',
'./dataset/Monday-WorkingHours.pcap.split13',
'./dataset/Monday-WorkingHours.pcap.split14',
'./dataset/Monday-WorkingHours.pcap.split15',
]
$wanted_type = 0
$max_flow_length = 200
$pos_skip = 0
$neg_skip = 10100
$neg_step_size = 0
pos_source = dummy_source
neg_source = CICIDS2017SourceNeg.new
combined_pure_neg = MixerConfig.new(0, 1, 10, 1000)
mx = Mixer.new(combined_pure_neg, pos_source, neg_source)
=end



#================ for general =================
#$batch_size = 0
#$sample_count = 100
#$pos_skip = rand(665459) - $batch_size * $sample_count
#$pos_skip = 661497
#$neg_skip = rand(1334541) - $batch_size * $sample_count
#$neg_skip = 1338503

#================ for kitsune =================

STDERR.puts "Start config"

#SYN DoS neg Testing
=begin
$neg_csv = './kitsune-bak/SYN DoS_labels.csv'
$neg_csvf = './kitsune-bak/SYN DoS_dataset.csv'
$pos_skip = 0
$neg_skip = 20000
$neg_step_size = 0
$max_flow_length = 1000
pos_source = dummy_source
neg_source = KitsuneSourceNeg.new
combined_pure_neg = MixerConfig.new(0, 1, 20, 1000)
mx = Mixer.new(combined_pure_neg, pos_source, neg_source)
=end

#SYN DoS neg Training
=begin
$neg_csv = './kitsune-bak/SYN DoS_labels.csv'
$neg_csvf = './kitsune-bak/SYN DoS_dataset.csv'
$pos_skip = 0
$neg_skip = 0
$neg_step_size = 0
$max_flow_length = 1000
pos_source = dummy_source
neg_source = KitsuneSourceNeg.new
combined_pure_neg = MixerConfig.new(0, 1, 20, 1000)
mx = Mixer.new(combined_pure_neg, pos_source, neg_source)
=end

#SYN DoS Pos Testing
=begin
$pos_csv = './kitsune-bak/SYN DoS_labels.csv'
$pos_csvf = './kitsune-bak/SYN DoS_dataset.csv'
$pos_skip = 20
$neg_skip = 0
$neg_step_size = 0
$pos_step_size = 20
$max_flow_length = 1000
pos_source = KitsuneSourcePos.new
neg_source = dummy_source
combined_pure_pos = MixerConfig.new(1, 0, 20, 150)
mx = Mixer.new(combined_pure_pos, pos_source, neg_source)
=end

#SYN DoS Pos Training
=begin
$pos_csv = './kitsune-bak/SYN DoS_labels.csv'
$pos_csvf = './kitsune-bak/SYN DoS_dataset.csv'
$pos_skip = 0
$neg_skip = 0
$neg_step_size = 0
$pos_step_size = 20
$max_flow_length = 1000
pos_source = KitsuneSourcePos.new
neg_source = dummy_source
combined_pure_pos = MixerConfig.new(1, 0, 20, 150)
mx = Mixer.new(combined_pure_pos, pos_source, neg_source)
=end

STDERR.puts "Config done"
=begin
#SYN DoS Neg Training
$pos_csv = './kitsune/SYN DoS_labels.csv'
$pos_csvf = './kitsune/SYN DoS_dataset.csv'
$pos_skip = 0
$neg_skip = 0
$max_flow_length = 1000
pos_source = dummy_source
neg_source = KitsuneSourceNeg.new
combined_pure_neg = MixerConfig.new(0, 1, 20, 150)
mx = Mixer.new(combined_pure_neg, pos_source, neg_source)
=end

#$pos_prepare_num = $batch_size * $sample_count * 2 + $pos_skip
#$neg_prepare_num = $batch_size * $sample_count * 2 + $neg_skip

#pos_source = dummy_source
#neg_source = CICIDS2017SourceNeg.new

#pos_source = KitsuneSourcePos.new
#pos_source = dummy_source
#neg_source = KitsuneSourceNeg.new
#neg_source = dummy_source


mx.set_feature_types( PPacket.to_feature_type )
#mx.set_feature_types( KiPacket.to_feature_type )


mx.output

