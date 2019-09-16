
class DDataSource
  def init_source
    # do nothing
  end

  def next_flow
    return FFlow.new(nil, nil, nil)
  end
end


class FFlow
  attr_accessor :sig, :pkts, :est_time, :op_time, :ed_time, :attack_type
  def initialize(sig, est_time, type)
    @sig = sig
    @est_time = est_time
    @op_time = est_time
    @ed_time = est_time
    @pkts = []
    @attack_type = type
  end
end



class PPacket
  attr_accessor :pkt, :valid, :label, :time, :time_since_last,
                :src_ip, :dst_ip, :src_port, :dst_port, :protocol,
                :frag, :id, :len, :tos, :ttl,
                :syn, :ack, :fin, :rst, :psh, :urg, 
                :seq, :win
  def initialize(time, pkt = nil)
#    puts time
#    puts pkt.class

    zero_all

    @label = 0

    if pkt == nil
      @time = time
      @valid = false
      return
    end

    @valid = true
    @time = time
    @pkt = pkt
    if pkt.is_ipv6?
      @src_ip = pkt.ipv6_src
      @dst_ip = pkt.ipv6_dst
    else
      @src_ip = pkt.ip_src
      @dst_ip = pkt.ip_dst
      @frag = pkt.ip_frag
      @id = pkt.ip_id
      @len = pkt.ip_len
      @tos = pkt.ip_tos
      @ttl = pkt.ip_ttl
    end

    @protocol = 0
    if pkt.is_tcp?
      @src_port = pkt.tcp_src
      @dst_port = pkt.tcp_dst
      @protocol = 6
      @syn = pkt.tcp_flags.syn
      @ack = pkt.tcp_flags.ack
      @fin = pkt.tcp_flags.fin
      @rst = pkt.tcp_flags.rst
      @psh = pkt.tcp_flags.psh
      @urg = pkt.tcp_flags.urg
      @seq = pkt.tcp_seq
      @win = pkt.tcp_win
    elsif pkt.is_udp?
      @src_port = pkt.udp_sport
      @dst_port = pkt.udp_dport
      @protocol = 17
    end
  end

  def zero_all
  @label = 0
  @time = 0
  @time_since_last = 0
  @src_ip = 0
  @dst_ip = 0
  @src_port = 0
  @dst_port = 0
  @protocol = 0
  @frag = 0
  @id = 0
  @len = 0
  @tos = 0
  @ttl = 0
  @syn = 0
  @ack = 0
  @fin = 0
  @rst = 0
  @psh = 0
  @urg = 0
  @seq = 0
  @win = 0
  end

  def flow_signature
    return [@src_ip, @src_port, @dst_ip, @dst_port, @protocol]
  end

  def to_feature
=begin
            0,  1,
            2,  3,  4,  5,  6,
            7,  8,  9, 10, 11,
            12, 13, 14, 15, 16, 17,
            18, 19
=end
    return [@time, @time_since_last, 
            @src_ip, @src_port, @dst_ip, @dst_port, @protocol, 
            @frag, @id, @len, @tos, @ttl, 
            @syn, @ack, @fin, @rst, @psh, @urg,
            @seq, @win]
  end

  def self.to_feature_type
    return [0, 0, 
            1, 1, 1, 1, 2,
            0, 0, 0, 2, 0,
            2, 2, 2, 2, 2, 2,
            0, 0]
  end
end


