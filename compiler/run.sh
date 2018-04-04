./para_test offline ~/equinix-chicago.dirB.20150219-135900.UTC.anon.pcap 1 1 1 > run_log

for (( c=2; c<=12; c++ )) do
    ./para_test offline ~/equinix-chicago.dirB.20150219-135900.UTC.anon.pcap 1 1 $c >> run_log
done
