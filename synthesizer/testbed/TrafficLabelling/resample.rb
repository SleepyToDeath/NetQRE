
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

class Trace

  def reset_meta(num_field, num_flow, field_types)
    @num_field = num_field
    @num_flow = num_flow
    @field_types = field_types
    @flows = []
  end

  def push_flow(f)
    @flows << f
  end

  def from_file(file_name)
    
  end

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

  def print_meta(fp)
    fp "#{@num_field} #{@num_flow}"
    fp @field_types.map{ |x| x.to_s }.join(' ')
  end

  def print_sample(size, fp)
    @flows.each do |f|
      fp f.map{ |x| x.to_s }.join(' ')
    end
  end
end

