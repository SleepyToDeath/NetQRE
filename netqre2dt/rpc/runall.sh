#!/bin/bash
for ((i=2; i<=$1; i++))
do
	ssh n$i "cd rpc; make test" & 
done
