SHELL := /bin/bash

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

SRC2 = ../*pp
SRC2 += ../../data-transducer/*pp
SRC2 += $(INCLUDE_PATH)/rubify.*pp

all:
	$(CC) -I$(INCLUDE_PATH) -o netqre_server server.cc $(SRC) -lrpc -lpthread -lpcap -O2 $(STD)
#	$(CC) -I$(INCLUDE_PATH) -o netqre_client test_client.cpp $(SRC) -lrpc -lpthread -lpcap -O2 $(STD)

debug:
	$(CC) -I$(INCLUDE_PATH) -o netqre_server server.cc $(SRC) -lrpc -lpthread -lpcap -O2 $(STD)

parser:
	$(CC) -I$(INCLUDE_PATH) -o netqre_parser parser.cc $(SRC2) -lrpc -lpthread -lpcap -O2 $(STD)

exec:
	$(CC) -I$(INCLUDE_PATH) -o netqre_exec exec.cc $(SRC2) -lrpc -lpthread -lpcap -O2 $(STD)
	

run-server-local:
	echo "Staring QRE Servers Locally"
	-killall netqre_server
	sleep 1
	./netqre_server $(SAVING_PATH)$(POS_DATA_FILE) $(SAVING_PATH)$(NEG_DATA_FILE) 23333 >> server_log_1 2>&1 &
	sleep 0.1
	./netqre_server $(SAVING_PATH)$(POS_DATA_FILE) $(SAVING_PATH)$(NEG_DATA_FILE) 23334 >> server_log_2 2>&1 &
	sleep 0.1
	./netqre_server $(SAVING_PATH)$(POS_DATA_FILE) $(SAVING_PATH)$(NEG_DATA_FILE) 23335 >> server_log_3 2>&1 &
	sleep 0.1
	./netqre_server $(SAVING_PATH)$(POS_DATA_FILE) $(SAVING_PATH)$(NEG_DATA_FILE) 23336 >> server_log_4 2>&1 &
	sleep 0.1
	./netqre_server $(SAVING_PATH)$(POS_DATA_FILE) $(SAVING_PATH)$(NEG_DATA_FILE) 23337 >> server_log_4 2>&1 &
	sleep 0.1
	./netqre_server $(SAVING_PATH)$(POS_DATA_FILE) $(SAVING_PATH)$(NEG_DATA_FILE) 23338 >> server_log_4 2>&1 &
	sleep 0.1
	./netqre_server $(SAVING_PATH)$(POS_DATA_FILE) $(SAVING_PATH)$(NEG_DATA_FILE) 23339 >> server_log_4 2>&1 &
	sleep 0.1
	./netqre_server $(SAVING_PATH)$(POS_DATA_FILE) $(SAVING_PATH)$(NEG_DATA_FILE) 23340 >> server_log_4 2>&1 &
	sleep 0.1
	./netqre_server $(SAVING_PATH)$(POS_DATA_FILE) $(SAVING_PATH)$(NEG_DATA_FILE) 23341 >> server_log_4 2>&1 &
	sleep 0.1
	./netqre_server $(SAVING_PATH)$(POS_DATA_FILE) $(SAVING_PATH)$(NEG_DATA_FILE) 23342 >> server_log_4 2>&1 &
	sleep 0.1
	./netqre_server $(SAVING_PATH)$(POS_DATA_FILE) $(SAVING_PATH)$(NEG_DATA_FILE) 23343 >> server_log_4 2>&1 &
	sleep 0.1
	./netqre_server $(SAVING_PATH)$(POS_DATA_FILE) $(SAVING_PATH)$(NEG_DATA_FILE) 23344 >> server_log_4 2>&1 &
	sleep 5

LAST_SERVER = 13
FIRST_SERVER = 9

run-server:
	echo "Staring QRE Servers"
	./killall.sh $(FIRST_SERVER) $(LAST_SERVER)
	./distribute.sh $(SAVING_PATH)$(POS_DATA_FILE) $(SAVING_PATH)$(NEG_DATA_FILE) $(SAVING_PATH)$(POS_TEST_FILE) $(SAVING_PATH)$(NEG_TEST_FILE) $(FIRST_SERVER)  $(LAST_SERVER)
	./runall.sh $(FIRST_SERVER) $(LAST_SERVER)
	sleep 5

run-test: run-server
	./netqre_client ./testbed/test1/test.netqre ./testbed/test1/01.ts ./testbed/test1/server_list.txt

test:                                           
	for ((i=23333; i<=23344; i++)); \
	do \
		./netqre_server ./$(POS_DATA_FILE) ./$(NEG_DATA_FILE) $$i >> server_log &  \
		sleep 0.1 ;\
	done 

kill:
	-killall -9 netqre_server
