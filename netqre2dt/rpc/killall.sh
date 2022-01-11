!/bin/bash
make kill
for ((i=$1; i<=$2; i++))
do
	ssh -p2324 dedos$(printf "%02d" $i) "cd rpc; make kill" & 
done
