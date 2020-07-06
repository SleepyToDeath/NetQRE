SHELL := /bin/bash

INCLUDE_PATH = ../../../rubify

CC = g++
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

#SAVING_PATH = ../../synthesizer/testbed/mode-P/
CONFIG_PATH = ../../synthesizer/lang/netqre/
SERVER_FILE = mode-P_server_list.txt
SERVER_FILE_LOCAL = local_server_list.txt
QUERY_FILE = query.qre
UNLABELED_DATA_FILE = unlabeled.ts
POS_DATA_FILE = positive_train.ts
NEG_DATA_FILE = negative_train.ts
POS_TEST_FILE = positive_test.ts
NEG_TEST_FILE = negative_test.ts
RESULT_FILE = result_vector

all:
	$(CC) -I$(INCLUDE_PATH) -o netqre_server server.cpp $(SRC) -lrpc -lpthread -lpcap -O2 $(STD)
#	$(CC) -I$(INCLUDE_PATH) -o netqre_client test_client.cpp $(SRC) -lrpc -lpthread -lpcap -O2 $(STD)

debug:
	$(CC) -I$(INCLUDE_PATH) -o netqre_server server.cpp $(SRC) -lrpc -lpthread -lpcap -g $(STD)

test0:
	./netqre_server ../../synthesizer/testbed/small_valid_tcp.pcap ../../synthesizer/testbed/large_random_tcp.pcap 23333 &




run-server-local:
	echo "Staring QRE Servers Locally"
	-killall netqre_server
	./netqre_server $(SAVING_PATH)$(POS_DATA_FILE) $(SAVING_PATH)$(NEG_DATA_FILE) $(SAVING_PATH)$(POS_TEST_FILE) $(SAVING_PATH)$(NEG_TEST_FILE) 23333 >> server_log_1 2>&1 &
	sleep 0.1
	./netqre_server $(SAVING_PATH)$(POS_DATA_FILE) $(SAVING_PATH)$(NEG_DATA_FILE) $(SAVING_PATH)$(POS_TEST_FILE) $(SAVING_PATH)$(NEG_TEST_FILE) 23334 >> server_log_1 2>&1 &
	sleep 0.1
	./netqre_server $(SAVING_PATH)$(POS_DATA_FILE) $(SAVING_PATH)$(NEG_DATA_FILE) $(SAVING_PATH)$(POS_TEST_FILE) $(SAVING_PATH)$(NEG_TEST_FILE) 23335 >> server_log_1 2>&1 &
	sleep 0.1
	./netqre_server $(SAVING_PATH)$(POS_DATA_FILE) $(SAVING_PATH)$(NEG_DATA_FILE) $(SAVING_PATH)$(POS_TEST_FILE) $(SAVING_PATH)$(NEG_TEST_FILE) 23336 >> server_log_1 2>&1 &
	sleep 5

run-server-exec-local:
	echo "Staring QRE Servers Locally"
	-killall netqre_server
	./netqre_server $(SAVING_PATH)$(UNLABELED_DATA_FILE) $(SAVING_PATH)$(UNLABELED_DATA_FILE) 23333 >> server_log &
	sleep 0.1
	./netqre_server $(SAVING_PATH)$(UNLABELED_DATA_FILE) $(SAVING_PATH)$(UNLABELED_DATA_FILE) 23334 >> server_log &
	sleep 0.1
	./netqre_server $(SAVING_PATH)$(UNLABELED_DATA_FILE) $(SAVING_PATH)$(UNLABELED_DATA_FILE) 23335 >> server_log &
	sleep 0.1
	./netqre_server $(SAVING_PATH)$(UNLABELED_DATA_FILE) $(SAVING_PATH)$(UNLABELED_DATA_FILE) 23336 >> server_log &
	sleep 5

LAST_SERVER = 6

run-server:
	echo "Staring QRE Servers"
	./killall.sh $(LAST_SERVER)
	./distribute.sh $(SAVING_PATH)$(POS_DATA_FILE) $(SAVING_PATH)$(NEG_DATA_FILE) $(SAVING_PATH)$(POS_TEST_FILE) $(SAVING_PATH)$(NEG_TEST_FILE) $(LAST_SERVER)
	./runall.sh $(LAST_SERVER)
	sleep 5

run-server-exec:
	echo "Staring QRE Servers"
	./killall.sh $(LAST_SERVER)
	./distribute.sh $(SAVING_PATH)$(UNLABELED_DATA_FILE) $(SAVING_PATH)$(UNLABELED_DATA_FILE) $(LAST_SERVER)
	./runall.sh $(LAST_SERVER)
	sleep 5

run-local: run-server-exec-local
	./netqre_client $(SAVING_PATH)$(QUERY_FILE) $(SAVING_PATH)$(UNLABELED_DATA_FILE) $(CONFIG_PATH)$(SERVER_FILE_LOCAL) > $(SAVING_PATH)$(RESULT_FILE)

run: run-server-exec
	./netqre_client $(SAVING_PATH)$(QUERY_FILE) $(SAVING_PATH)$(UNLABELED_DATA_FILE) $(CONFIG_PATH)$(SERVER_FILE) > $(SAVING_PATH)$(RESULT_FILE)

run-test: run-server
	./netqre_client ./testbed/test1/test.netqre ./testbed/test1/01.ts ./testbed/test1/server_list.txt

test:                                           
	for ((i=23333; i<=23333; i++)); \
	do \
		valgrind --tool=callgrind ./netqre_server ./$(POS_DATA_FILE) ./$(NEG_DATA_FILE) ./$(POS_TEST_FILE) ./$(NEG_TEST_FILE) $$i >> server_log &  \
		sleep 0.1 ;\
	done 

dtest:
	gdb -ex run --args ./netqre_server ../../synthesizer/testbed/small_valid_tcp.pcap ../../synthesizer/testbed/large_random_tcp.pcap

kill:
	-killall -9 netqre_server
