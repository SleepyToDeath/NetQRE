#!/bin/bash
cp $1 ./positive_train.ts
cp $2 ./negative_train.ts
cp $3 ./positive_test.ts
cp $4 ./negative_test.ts
for ((i=2; i<=$5; i++))
do
	scp ./Makefile n$i:rpc/
	scp ./netqre_server n$i:rpc/
	scp ./positive_train.ts n$i:rpc/
	scp ./negative_train.ts n$i:rpc/
	scp ./positive_test.ts n$i:rpc/
	scp ./negative_test.ts n$i:rpc/
done
