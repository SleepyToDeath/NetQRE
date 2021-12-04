
$input_files = ["input1", "input2"]
$mix_rate = [ 0.3, 0.7 ]
$sample_sizes = [ 5, 10, 20, 50, 100 ]

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
    @num_field, @num_flow = fin.next.split(' ').map{|s| s.to_i}
    @field_types = fin.next.split(' ').map{|s| s.to_i}
    flow = []
    loop do
      s = fin.next
      if s.split(' ').size == 0
        @flows << flow
        flow = []
      else
        flow << fin.next.split(' ').map{|s| s.to_i}
      end
    end
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
      do
        i = pick_rand_index(probabilities)
      while counters[i] < traces[i].flows.size
      @flows << traces[i].flows[counters[i]]
      counters[i] += 1
    end
  end

  # keep only the first `len` flows
  def trunc(len)

  end

  ## ========== dump ===========
  # sample size = how many flows each trace(data point for training and testing) contains

  # dump metadata & flows separately
  def print_meta(fp)
    fp.call "#{@num_field} #{@num_flow}"
    fp.call @field_types.map{ |x| x.to_s }.join(' ')
  end

  def print_samples(sample_size, fp)
    ct = Counter.new(sample_size, ->{ fp.call("") } )
    @flows.each do |f|
      fp.call f.map{ |x| x.to_s }.join(' ')
      ct.tick
    end
  end

  # dump everything to a file
  def to_file(sample_size, file_name)
  end
end

