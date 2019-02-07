SRC = ./server.cpp
SRC += ./client.hpp
SRC += ../*pp
SRC += ../../data-transducer/*pp
SRC += ../../synthesizer/core/*pp
SRC += ../../synthesizer/lang/general/*.hpp
SRC += ../../synthesizer/lang/general/json_wrapper.cpp
SRC += ../../synthesizer/lang/general/json11/*pp
SRC += ../network_tokenizer/*

all:
	g++-8 -o netqre_server $(SRC) -lrpc -lpthread -lpcap -O2 --std=c++14

debug:
	g++-8 -o netqre_server $(SRC) -lrpc -lpthread -lpcap -g --std=c++14

test:
	./netqre_server ../../synthesizer/testbed/small_valid_tcp.pcap ../../synthesizer/testbed/large_random_tcp.pcap 00 &
	./netqre_server ../../synthesizer/testbed/small_valid_tcp.pcap ../../synthesizer/testbed/large_random_tcp.pcap 01 &
	./netqre_server ../../synthesizer/testbed/small_valid_tcp.pcap ../../synthesizer/testbed/large_random_tcp.pcap 02 &
	./netqre_server ../../synthesizer/testbed/small_valid_tcp.pcap ../../synthesizer/testbed/large_random_tcp.pcap 03 &
	./netqre_server ../../synthesizer/testbed/small_valid_tcp.pcap ../../synthesizer/testbed/large_random_tcp.pcap 04 &
	./netqre_server ../../synthesizer/testbed/small_valid_tcp.pcap ../../synthesizer/testbed/large_random_tcp.pcap 05 &
	./netqre_server ../../synthesizer/testbed/small_valid_tcp.pcap ../../synthesizer/testbed/large_random_tcp.pcap 06 &
	./netqre_server ../../synthesizer/testbed/small_valid_tcp.pcap ../../synthesizer/testbed/large_random_tcp.pcap 07 &
	./netqre_server ../../synthesizer/testbed/small_valid_tcp.pcap ../../synthesizer/testbed/large_random_tcp.pcap 08 &
	./netqre_server ../../synthesizer/testbed/small_valid_tcp.pcap ../../synthesizer/testbed/large_random_tcp.pcap 09 &
	./netqre_server ../../synthesizer/testbed/small_valid_tcp.pcap ../../synthesizer/testbed/large_random_tcp.pcap 10 &
	./netqre_server ../../synthesizer/testbed/small_valid_tcp.pcap ../../synthesizer/testbed/large_random_tcp.pcap 11 &
	./netqre_server ../../synthesizer/testbed/small_valid_tcp.pcap ../../synthesizer/testbed/large_random_tcp.pcap 12 &
	./netqre_server ../../synthesizer/testbed/small_valid_tcp.pcap ../../synthesizer/testbed/large_random_tcp.pcap 13 &
	./netqre_server ../../synthesizer/testbed/small_valid_tcp.pcap ../../synthesizer/testbed/large_random_tcp.pcap 14 &
	./netqre_server ../../synthesizer/testbed/small_valid_tcp.pcap ../../synthesizer/testbed/large_random_tcp.pcap 15 &
	./netqre_server ../../synthesizer/testbed/small_valid_tcp.pcap ../../synthesizer/testbed/large_random_tcp.pcap 16 &
	./netqre_server ../../synthesizer/testbed/small_valid_tcp.pcap ../../synthesizer/testbed/large_random_tcp.pcap 17 &
	./netqre_server ../../synthesizer/testbed/small_valid_tcp.pcap ../../synthesizer/testbed/large_random_tcp.pcap 18 &
	./netqre_server ../../synthesizer/testbed/small_valid_tcp.pcap ../../synthesizer/testbed/large_random_tcp.pcap 19 &
	./netqre_server ../../synthesizer/testbed/small_valid_tcp.pcap ../../synthesizer/testbed/large_random_tcp.pcap 20 &
	./netqre_server ../../synthesizer/testbed/small_valid_tcp.pcap ../../synthesizer/testbed/large_random_tcp.pcap 21 &
	./netqre_server ../../synthesizer/testbed/small_valid_tcp.pcap ../../synthesizer/testbed/large_random_tcp.pcap 22 &
	./netqre_server ../../synthesizer/testbed/small_valid_tcp.pcap ../../synthesizer/testbed/large_random_tcp.pcap 23 &
	./netqre_server ../../synthesizer/testbed/small_valid_tcp.pcap ../../synthesizer/testbed/large_random_tcp.pcap 24 &
	./netqre_server ../../synthesizer/testbed/small_valid_tcp.pcap ../../synthesizer/testbed/large_random_tcp.pcap 25 &
	./netqre_server ../../synthesizer/testbed/small_valid_tcp.pcap ../../synthesizer/testbed/large_random_tcp.pcap 26 &
	./netqre_server ../../synthesizer/testbed/small_valid_tcp.pcap ../../synthesizer/testbed/large_random_tcp.pcap 27 &
	./netqre_server ../../synthesizer/testbed/small_valid_tcp.pcap ../../synthesizer/testbed/large_random_tcp.pcap 28 &
	./netqre_server ../../synthesizer/testbed/small_valid_tcp.pcap ../../synthesizer/testbed/large_random_tcp.pcap 29 &
	./netqre_server ../../synthesizer/testbed/small_valid_tcp.pcap ../../synthesizer/testbed/large_random_tcp.pcap 30 &
	./netqre_server ../../synthesizer/testbed/small_valid_tcp.pcap ../../synthesizer/testbed/large_random_tcp.pcap 31 &

dtest:
	gdb -ex run --args ./netqre_server ../../synthesizer/testbed/small_valid_tcp.pcap ../../synthesizer/testbed/large_random_tcp.pcap

kill:
	killall -9 netqre_server
