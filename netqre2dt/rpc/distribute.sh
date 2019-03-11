cp ./large_random_tcp.pcap ../../synthesizer/testbed/
cp ./small_valid_tcp.pcap ../../synthesizer/testbed/
for i in {2..5}
do
	scp ./netqre_server n$i:rpc/
	scp ./large_random_tcp.pcap n$i:rpc/
	scp ./small_valid_tcp.pcap n$i:rpc/
done
