cp ../../synthesizer/testbed/positive.ts ./
cp ../../synthesizer/testbed/negative.ts ./
for i in {2..5}
do
	scp ./Makefile n$i:rpc/
	scp ./netqre_server n$i:rpc/
	scp ./positive.ts n$i:rpc/
	scp ./negative.ts n$i:rpc/
done
