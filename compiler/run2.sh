./heavy_hitter_src_qre_parallel offline ~/equinix-chicago.dirB.20150219-135900.UTC.anon.pcap 1 1 1 > run3_log

for (( c=2; c<=12; c++ )) do
    ./heavy_hitter_src_qre_parallel offline ~/equinix-chicago.dirB.20150219-135900.UTC.anon.pcap 1 1 $c >> run3_log
done
