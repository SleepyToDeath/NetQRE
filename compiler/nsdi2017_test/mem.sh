./heavy_hitter_src_qre offline ~/equinix-chicago.dirB.20150219-135900.UTC.anon.pcap 1 1 1
ps aux | grep heavy_hitter | grep pcap | awk '{print $2}' | xargs sudo python ps_mem.py -p
