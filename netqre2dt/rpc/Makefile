
CC = g++

SRC = ./server.cpp
SRC += ./client.hpp
SRC += ../*pp
SRC += ../../data-transducer/*pp
SRC += ../../synthesizer/core/*pp
SRC += ../../synthesizer/lang/general/*.hpp
SRC += ../../synthesizer/lang/general/json_wrapper.cpp
SRC += ../../synthesizer/lang/general/json11/*pp
SRC += ../network_tokenizer/feature_vector.hpp
SRC += ../../general-lang/*pp

all:
	$(CC) -I/home/sleepytodeath/other_projects/rubify/ -o netqre_server $(SRC) -lrpc -lpthread -lpcap -O2 --std=c++14

debug:
	$(CC) -o netqre_server $(SRC) -lrpc -lpthread -lpcap -g --std=c++14

test0:
	./netqre_server ../../synthesizer/testbed/small_valid_tcp.pcap ../../synthesizer/testbed/large_random_tcp.pcap 23333 &

test1:
	./netqre_server ./negative.ts ./positive.ts 23333 &
	./netqre_server ./negative.ts ./positive.ts 23334 &
	./netqre_server ./negative.ts ./positive.ts 23335 &
	./netqre_server ./negative.ts ./positive.ts 23336 &
	./netqre_server ./negative.ts ./positive.ts 23337 &
	./netqre_server ./negative.ts ./positive.ts 23338 &
	./netqre_server ./negative.ts ./positive.ts 23339 &
	./netqre_server ./negative.ts ./positive.ts 23340 &
	./netqre_server ./negative.ts ./positive.ts 23341 &
	./netqre_server ./negative.ts ./positive.ts 23342 &
	./netqre_server ./negative.ts ./positive.ts 23343 &
	./netqre_server ./negative.ts ./positive.ts 23344 &
	./netqre_server ./negative.ts ./positive.ts 23345 &
	./netqre_server ./negative.ts ./positive.ts 23346 &
	./netqre_server ./negative.ts ./positive.ts 23347 &
	./netqre_server ./negative.ts ./positive.ts 23348 &

test:
	./netqre_server ./negative.ts ./positive.ts 23333 &
	./netqre_server ./negative.ts ./positive.ts 23334 &
	./netqre_server ./negative.ts ./positive.ts 23335 &
	./netqre_server ./negative.ts ./positive.ts 23336 &
	./netqre_server ./negative.ts ./positive.ts 23337 &
	./netqre_server ./negative.ts ./positive.ts 23338 &
	./netqre_server ./negative.ts ./positive.ts 23339 &
	./netqre_server ./negative.ts ./positive.ts 23340 &
	./netqre_server ./negative.ts ./positive.ts 23341 &
	./netqre_server ./negative.ts ./positive.ts 23342 &
	./netqre_server ./negative.ts ./positive.ts 23343 &
	./netqre_server ./negative.ts ./positive.ts 23344 &
	./netqre_server ./negative.ts ./positive.ts 23345 &
	./netqre_server ./negative.ts ./positive.ts 23346 &
	./netqre_server ./negative.ts ./positive.ts 23347 &
	./netqre_server ./negative.ts ./positive.ts 23348 &
	./netqre_server ./negative.ts ./positive.ts 23349 &
	./netqre_server ./negative.ts ./positive.ts 23351 &
	./netqre_server ./negative.ts ./positive.ts 23352 &
	./netqre_server ./negative.ts ./positive.ts 23353 &
	./netqre_server ./negative.ts ./positive.ts 23354 &
	./netqre_server ./negative.ts ./positive.ts 23355 &
	./netqre_server ./negative.ts ./positive.ts 23356 &
	./netqre_server ./negative.ts ./positive.ts 23357 &
	./netqre_server ./negative.ts ./positive.ts 23358 &
	./netqre_server ./negative.ts ./positive.ts 23359 &
	./netqre_server ./negative.ts ./positive.ts 23360 &
	./netqre_server ./negative.ts ./positive.ts 23361 &
	./netqre_server ./negative.ts ./positive.ts 23362 &
	./netqre_server ./negative.ts ./positive.ts 23363 &
	./netqre_server ./negative.ts ./positive.ts 23364 &
	./netqre_server ./negative.ts ./positive.ts 23365 &

dtest:
	gdb -ex run --args ./netqre_server ../../synthesizer/testbed/small_valid_tcp.pcap ../../synthesizer/testbed/large_random_tcp.pcap

kill:
	killall -9 netqre_server
