#!/bin/bash
for ((i=1; i<=1; i++))
do
	make test
	#ssh n$i "cd rpc; make test" & 
done
