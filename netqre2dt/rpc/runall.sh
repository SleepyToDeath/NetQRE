#!/bin/bash
make test &
for ((i=$1; i<=$2; i++))
do
	ssh -p2324 dedos$(printf "%02d" $i) "cd rpc; make test" & 
done
