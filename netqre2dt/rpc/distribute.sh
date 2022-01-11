#!/bin/bash
cp $1 ./positive_train.ts
cp $2 ./negative_train.ts
#cp $3 ./positive_test.ts
#cp $4 ./negative_test.ts
for ((i=$5; i<=$6; i++));
do
	hname=dedos$(printf "%02d" $i)
	scp -P 2324 ./Makefile $hname:rpc/
	scp -P 2324 ./netqre_server $hname:rpc/
	scp -P 2324 ./positive_train.ts $hname:rpc/
	scp -P 2324 ./negative_train.ts $hname:rpc/
#	scp -P 2324 ./positive_test.ts $hname:rpc/
#	scp -P 2324 ./negative_test.ts $hname:rpc/
done
