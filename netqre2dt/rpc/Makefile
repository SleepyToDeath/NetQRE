INCLUDE_PATH = ../../../rubify

CC = g++-9
STD = --std=c++2a

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
UNLABELED_DATA_FILE = unlabeled.ts
POS_DATA_FILE = positive.ts
NEG_DATA_FILE = negative.ts
RESULT_FILE = result_vector

all:
#	$(CC) -I$(INCLUDE_PATH) -o netqre_server server.cpp $(SRC) -lrpc -lpthread -lpcap -O2 $(STD)
	$(CC) -I$(INCLUDE_PATH) -o netqre_client test_client.cpp $(SRC) -lrpc -lpthread -lpcap -O2 $(STD)

debug:
	$(CC) -o netqre_server $(SRC) -lrpc -lpthread -lpcap -g $(STD)

test0:
	./netqre_server ../../synthesizer/testbed/small_valid_tcp.pcap ../../synthesizer/testbed/large_random_tcp.pcap 23333 &

LAST_SERVER = 7

run-server:
	echo "Staring QRE Servers"
	./killall.sh $(LAST_SERVER)
	./distribute.sh $(SAVING_PATH)$(POS_DATA_FILE) $(SAVING_PATH)$(NEG_DATA_FILE) $(LAST_SERVER)
	./runall.sh $(LAST_SERVER)
	sleep 5

run-server-exec:
	echo "Staring QRE Servers"
	./killall.sh $(LAST_SERVER)
	./distribute.sh $(SAVING_PATH)$(UNLABELED_DATA_FILE) $(SAVING_PATH)$(UNLABELED_DATA_FILE) $(LAST_SERVER)
	./runall.sh $(LAST_SERVER)
	sleep 5

run: run-server-exec
	./netqre_client $(SAVING_PATH)$(QUERY_FILE) $(SAVING_PATH)$(UNLABELED_DATA_FILE) $(CONFIG_PATH)$(SERVER_FILE) > $(SAVING_PATH)$(RESULT_FILE)

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
	./netqre_server ./positive.ts ./negative.ts 23333 >> server_log &
	sleep 0.1
	./netqre_server ./positive.ts ./negative.ts 23334 >> server_log &
	sleep 0.1
	./netqre_server ./positive.ts ./negative.ts 23335 >> server_log &
	sleep 0.1
	./netqre_server ./positive.ts ./negative.ts 23336 >> server_log &
	sleep 0.1
	./netqre_server ./positive.ts ./negative.ts 23337 >> server_log &
	sleep 0.1
	./netqre_server ./positive.ts ./negative.ts 23338 >> server_log &
	sleep 0.1
	./netqre_server ./positive.ts ./negative.ts 23339 >> server_log &
	sleep 0.1
	./netqre_server ./positive.ts ./negative.ts 23340 >> server_log &
	sleep 0.1
	./netqre_server ./positive.ts ./negative.ts 23341 >> server_log &
	sleep 0.1
	./netqre_server ./positive.ts ./negative.ts 23342 >> server_log &
	sleep 0.1
	./netqre_server ./positive.ts ./negative.ts 23343 >> server_log &
	sleep 0.1
	./netqre_server ./positive.ts ./negative.ts 23344 >> server_log &
	sleep 0.1
	./netqre_server ./positive.ts ./negative.ts 23345 >> server_log &
	sleep 0.1
	./netqre_server ./positive.ts ./negative.ts 23346 >> server_log &
	sleep 0.1
	./netqre_server ./positive.ts ./negative.ts 23347 >> server_log &
	sleep 0.1
	./netqre_server ./positive.ts ./negative.ts 23348 >> server_log &
	sleep 0.1
	./netqre_server ./positive.ts ./negative.ts 23349 >> server_log &
	sleep 0.1
	./netqre_server ./positive.ts ./negative.ts 23350 >> server_log &
	sleep 0.1
	./netqre_server ./positive.ts ./negative.ts 23351 >> server_log &
	sleep 0.1
	./netqre_server ./positive.ts ./negative.ts 23352 >> server_log &
	sleep 0.1
	./netqre_server ./positive.ts ./negative.ts 23353 >> server_log &
	sleep 0.1
	./netqre_server ./positive.ts ./negative.ts 23354 >> server_log &
	sleep 0.1
	./netqre_server ./positive.ts ./negative.ts 23355 >> server_log &
	sleep 0.1
	./netqre_server ./positive.ts ./negative.ts 23356 >> server_log &
	sleep 0.1
	./netqre_server ./positive.ts ./negative.ts 23357 >> server_log &
	sleep 0.1
	./netqre_server ./positive.ts ./negative.ts 23358 >> server_log &
	sleep 0.1
	./netqre_server ./positive.ts ./negative.ts 23359 >> server_log &
	sleep 0.1
	./netqre_server ./positive.ts ./negative.ts 23360 >> server_log &
	sleep 0.1
	./netqre_server ./positive.ts ./negative.ts 23361 >> server_log &
	sleep 0.1
	./netqre_server ./positive.ts ./negative.ts 23362 >> server_log &
	sleep 0.1
	./netqre_server ./positive.ts ./negative.ts 23363 >> server_log &
	sleep 0.1
	./netqre_server ./positive.ts ./negative.ts 23364 >> server_log &
	sleep 0.1

dtest:
	gdb -ex run --args ./netqre_server ../../synthesizer/testbed/small_valid_tcp.pcap ../../synthesizer/testbed/large_random_tcp.pcap

kill:
	-killall -9 netqre_server
