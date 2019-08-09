#!/bin/bash
cp $1 ./positive.ts
cp $2 ./negative.ts
for ((i=2; i<=$3; i++))
do
	scp ./Makefile n$i:rpc/
	scp ./netqre_server n$i:rpc/
	scp ./positive.ts n$i:rpc/
	scp ./negative.ts n$i:rpc/
done
