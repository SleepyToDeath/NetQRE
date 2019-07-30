INCLUDE_PATH = /home/sleepytodeath/other_projects/rubify

CC = g++

SRC = ./client.hpp
SRC += ../*pp
SRC += ../../data-transducer/*pp
SRC += ../../synthesizer/core/*pp
SRC += ../../synthesizer/lang/general/*.hpp
SRC += ../../synthesizer/lang/general/json_wrapper.cpp
SRC += ../../synthesizer/lang/general/json11/*pp
SRC += ../network_tokenizer/feature_vector.hpp
SRC += ../../general-lang/*pp
SRC += $(INCLUDE_PATH)/rubify.cpp

SAVING_PATH = ../../synthesizer/testbed/mode-P/
CONFIG_PATH = ../../synthesizer/lang/netqre/
SERVER_FILE = mode-P_server_list.txt
QUERY_FILE = query.qre
DATA_FILE = positive.ts
RESULT_FILE = result_vector

all:
	$(CC) -I$(INCLUDE_PATH) -o netqre_server server.cpp $(SRC) -lrpc -lpthread -lpcap -g --std=c++14
	$(CC) -I$(INCLUDE_PATH) -o netqre_client test_client.cpp $(SRC) -lrpc -lpthread -lpcap -g --std=c++14

debug:
	$(CC) -o netqre_server $(SRC) -lrpc -lpthread -lpcap -g --std=c++14

test0:
	./netqre_server ../../synthesizer/testbed/small_valid_tcp.pcap ../../synthesizer/testbed/large_random_tcp.pcap 23333 &

run-server:
	-killall netqre_server
	./netqre_server $(SAVING_PATH)$(DATA_FILE) $(SAVING_PATH)$(DATA_FILE) 23333 &

run: run-server
	./netqre_client $(SAVING_PATH)$(QUERY_FILE) $(SAVING_PATH)$(DATA_FILE) $(CONFIG_PATH)$(SERVER_FILE) > $(SAVING_PATH)$(RESULT_FILE)

run-test: run-server
	./netqre_client ./testbed/test1/test.netqre ./testbed/test1/01.ts ./testbed/test1/server_list.txt

test1:
	./netqre_server ./positive.ts ./negative.ts 23333 &
	./netqre_server ./positive.ts ./negative.ts 23334 &
	./netqre_server ./positive.ts ./negative.ts 23335 &
	./netqre_server ./positive.ts ./negative.ts 23336 &
	./netqre_server ./positive.ts ./negative.ts 23337 &
	./netqre_server ./positive.ts ./negative.ts 23338 &
	./netqre_server ./positive.ts ./negative.ts 23339 &
	./netqre_server ./positive.ts ./negative.ts 23340 &
	./netqre_server ./positive.ts ./negative.ts 23341 &
	./netqre_server ./positive.ts ./negative.ts 23342 &
	./netqre_server ./positive.ts ./negative.ts 23343 &
	./netqre_server ./positive.ts ./negative.ts 23344 &
	./netqre_server ./positive.ts ./negative.ts 23345 &
	./netqre_server ./positive.ts ./negative.ts 23346 &
	./netqre_server ./positive.ts ./negative.ts 23347 &
	./netqre_server ./positive.ts ./negative.ts 23348 &
                                                
test:                                           
	./netqre_server ./positive.ts ./negative.ts 23333 &
	./netqre_server ./positive.ts ./negative.ts 23334 &
	./netqre_server ./positive.ts ./negative.ts 23335 &
	./netqre_server ./positive.ts ./negative.ts 23336 &
	./netqre_server ./positive.ts ./negative.ts 23337 &
	./netqre_server ./positive.ts ./negative.ts 23338 &
	./netqre_server ./positive.ts ./negative.ts 23339 &
	./netqre_server ./positive.ts ./negative.ts 23340 &
	./netqre_server ./positive.ts ./negative.ts 23341 &
	./netqre_server ./positive.ts ./negative.ts 23342 &
	./netqre_server ./positive.ts ./negative.ts 23343 &
	./netqre_server ./positive.ts ./negative.ts 23344 &
	./netqre_server ./positive.ts ./negative.ts 23345 &
	./netqre_server ./positive.ts ./negative.ts 23346 &
	./netqre_server ./positive.ts ./negative.ts 23347 &
	./netqre_server ./positive.ts ./negative.ts 23348 &
	./netqre_server ./positive.ts ./negative.ts 23349 &
	./netqre_server ./positive.ts ./negative.ts 23351 &
	./netqre_server ./positive.ts ./negative.ts 23352 &
	./netqre_server ./positive.ts ./negative.ts 23353 &
	./netqre_server ./positive.ts ./negative.ts 23354 &
	./netqre_server ./positive.ts ./negative.ts 23355 &
	./netqre_server ./positive.ts ./negative.ts 23356 &
	./netqre_server ./positive.ts ./negative.ts 23357 &
	./netqre_server ./positive.ts ./negative.ts 23358 &
	./netqre_server ./positive.ts ./negative.ts 23359 &
	./netqre_server ./positive.ts ./negative.ts 23360 &
	./netqre_server ./positive.ts ./negative.ts 23361 &
	./netqre_server ./positive.ts ./negative.ts 23362 &
	./netqre_server ./positive.ts ./negative.ts 23363 &
	./netqre_server ./positive.ts ./negative.ts 23364 &
	./netqre_server ./positive.ts ./negative.ts 23365 &

dtest:
	gdb -ex run --args ./netqre_server ../../synthesizer/testbed/small_valid_tcp.pcap ../../synthesizer/testbed/large_random_tcp.pcap

kill:
	killall -9 netqre_server
