#!/usr/bin/ruby
def pick_rand_index(probabilities)
  r = rand(0.0..1.0)
  i = 0
  sum_p = 0.0
  probabilities.each do | p |
    sum_p += p
    if sum_p >= r then
      break
    end
    i += 1
  end
  return i
end

$flow_id_idx = [2, 3, 4, 5, 6]

def new_flow?(flow, pkt)
  if flow.empty? then return false end
  $flow_id_idx.each do |idx|
    if flow[-1][idx] != pkt[idx]
      return true
		end
  end
	return false
end


class Counter
  def initialize(bar, func)
    @bar = bar
    @func = func
    @counter = 0
  end

  def tick
    @counter += 1
    if @counter == @bar
      @func.call
      @counter = 0
    end
  end
end

class Trace

  attr_accessor :num_field, :num_flow, :field_types, :flows

  ## ========== constructor ===========

  # copy initializer
  def initialize(src)
    @num_field = src.num_field
    @num_flow = src.num_flow
    @field_types = src.field_types
    @flows = src.flows.clone
  end

  # do nothing, just a placeholder that allows lazy initialization later
  def initialize
    @flows = []
  end

  # lazy initializer, first set metadata
  def reset_meta(num_field, num_flow, field_types)
    @num_field = num_field
    @num_flow = num_flow
    @field_types = field_types
    @flows = []
  end

  # then insert flows one by one
  def push_flow(f)
    @flows << f
  end

  # or entirely read from file
  def from_file(file_name)
    file = File.open(file_name)
    fin = file.each_line
    @num_flow, @num_field = fin.next.split(' ').map{|s| s.to_i}
    @field_types = fin.next.split(' ').map{|s| s.to_i}
    flow = []
    loop do
      s = fin.next
      pkt = s.split(' ').map{|s| s.to_i}
      if pkt.size == 0 || new_flow?(flow, pkt)
        @flows << flow
        flow = []
      end
      if pkt.size != 0
        flow << pkt
      end
    end
		@num_flow = @flows.size
  end

  ## ========= modifier ==========

  # merge 2 traces into 1 with probability of taking next flow from each trace
  #
  def merge(traces, probabilities)
    @num_field = traces[0].num_field
    @num_flow = traces.map{ |t| t.flows.size }.sum
    @field_types = traces[0].field_types
    @flows = []
    counters = traces.map{ |x| 0 }
    while @flows.size < @num_flow do
      i = 0
      loop do
        i = pick_rand_index(probabilities)
        break if counters[i] < traces[i].flows.size
      end
      @flows << traces[i].flows[counters[i]]
      counters[i] += 1
    end
  end

  # keep only the first `len` flows
  def trunc(len)
		if @num_flow < len
			puts "Warning: truncate to longer!\n"
		end
    @flows = @flows[0..(len-1)]
    @num_flow = len
  end

  ## ========== dump ===========
  # sample size = how many flows each trace(data point for training and testing) contains

  # dump metadata & flows separately
  def print_meta(sample_size, fp)
    fp.call "#{@num_flow / sample_size} #{@num_field}"
    fp.call @field_types.map{ |x| x.to_s }.join(' ')
  end

  def print_samples(sample_size, fp)
    ct = Counter.new(sample_size, ->{ fp.call("") } )
    @flows.each do |f|
      f.each do |pkt|
        fp.call pkt.map{ |x| x.to_s }.join(' ')
      end
      ct.tick
    end
  end

  # dump everything to a file
  def to_file(sample_size, file_name)
    s = ""
    appendln = -> (suffix) { s += suffix + "\n" }
    print_meta(sample_size, appendln)
    print_samples(sample_size, appendln)
    File.open(file_name, 'w') { |f| f.write s }
  end
end

$input_files = ["./tokenstreams/ddos-test.ts", "./tokenstreams/neg-test.ts"]
$output_file_prefix = "./tokenstreams/mixed0.8-ddos-test"
$output_file_suffix = ".ts"
$mix_rate = [ 0.2, 0.8 ]
$sample_sizes = [ 5, 10, 20, 50 ]

srcs = $input_files.map do |name|
  t = Trace.new
  t.from_file(name)
	puts t.num_flow
  t
end

dst = Trace.new
if srcs.size == 1 then
	dst = srcs[0]
else
	dst.merge(srcs, $mix_rate)
end

dst.trunc(1000)
$sample_sizes.each do |size|
  name = $output_file_prefix + size.to_s + $output_file_suffix
  dst.to_file(size, name)
end
