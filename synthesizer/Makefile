all: netqre

g: netqre-debug

d: test-netqre-debug

p: profile-regex

train: train-netqre-distro

INCLUDE_PATH = ../../rubify

CC=g++
STD=--std=c++2a

SRC = core/*pp
#SRC += lang/network_tokenizer/*pp
SRC += core/*h
SRC += lang/general/*pp 
SRC += lang/general/*h
SRC += lang/general/json11/json11.cpp
SRC += lang/general/json11/json11.hpp

SRC2 = core/*pp
SRC2 += lang/general/*pp
SRC2 += lang/general/*cc
SRC2 += lang/general/json11/json11.cpp
SRC2 += lang/general/json11/json11.hpp
SRC2 += ../netqre2dt/network_tokenizer/feature_vector.hpp
SRC2 += ../netqre2dt/interpreter.cpp
SRC2 += ../netqre2dt/op.cpp
SRC2 += ../netqre2dt/op.hpp
SRC2 += ../netqre2dt/parser.cpp
SRC2 += ../data-transducer/*pp
SRC2 += ../general-lang/*pp
SRC2 += ../netqre2dt/rpc/client.cpp
SRC2 += $(INCLUDE_PATH)/rubify.cpp

#shared parameter
EXECUTION_PATH = ../netqre2dt/rpc/
CONFIG_PATH = ./lang/netqre/
POS_FILE = positive_train.ts
NEG_FILE = negative_train.ts
POS_TEST_FILE = positive_test.ts
NEG_TEST_FILE = negative_test.ts
SERVER_FILE_LOCAL = local_server_list.txt

#mode-p only
GRAMMAR_FILE = mode-P.json
CONFIG_FILE = mode-P.config
SERVER_FILE = server_list.txt
SAVING_PATH = ./testbed/mode-P/
QUERY_FILE = query.qre

#general netqre only
SAVING_PATH_0 = ./testbed/
GRAMMAR_FILE_0 = netqre.json
CONFIG_FILE_0 = netqre.config
SERVER_FILE_0 = server_list.txt


netqre:
	$(CC) -I$(INCLUDE_PATH) -o ./testbed/synthesizer $(SRC2) -O2 $(STD) -lrpc -pthread -lpcap

netqre-debug:
	$(CC) -I$(INCLUDE_PATH) -o ./testbed/synthesizer $(SRC2) -g $(STD) -lrpc -pthread -lpcap

train-netqre:
	cd $(EXECUTION_PATH) ; make SAVING_PATH="../../synthesizer/testbed/" run-server-local
	./testbed/synthesizer $(CONFIG_PATH)$(GRAMMAR_FILE_0) $(CONFIG_PATH)$(CONFIG_FILE_0) $(CONFIG_PATH)$(SERVER_FILE_LOCAL) $(SAVING_PATH_0)$(POS_FILE) $(SAVING_PATH_0)$(NEG_FILE) $(SAVING_PATH_0)$(POS_TEST_FILE) $(SAVING_PATH_0)$(NEG_TEST_FILE) 

train-netqre-distro:
	cd $(EXECUTION_PATH) ; make SAVING_PATH="../../synthesizer/testbed/" run-server
	./testbed/synthesizer $(CONFIG_PATH)$(GRAMMAR_FILE_0) $(CONFIG_PATH)$(CONFIG_FILE_0) $(CONFIG_PATH)$(SERVER_FILE) $(SAVING_PATH_0)$(POS_FILE) $(SAVING_PATH_0)$(NEG_FILE) $(SAVING_PATH_0)$(POS_TEST_FILE) $(SAVING_PATH_0)$(NEG_TEST_FILE) 

test-netqre-debug:
	gdb -ex run --args ./testbed/synthesizer_debug ./lang/netqre/netqre.json ./testbed/positive.ts ./testbed/negative.ts ./lang/netqre/netqre.config ./lang/netqre/netqre_server_list.txt

debug-mode-P:
	./testbed/synthesizer $(CONFIG_PATH)$(GRAMMAR_FILE) $(SAVING_PATH)$(POS_FILE) $(SAVING_PATH)$(NEG_FILE) $(CONFIG_PATH)$(CONFIG_FILE) $(CONFIG_PATH)$(SERVER_FILE)  
	# > $(SAVING_PATH)$(QUERY_FILE)

test-mode-P-local:
	echo "running execution server"
	cd $(EXECUTION_PATH) ; make run-server-local
	sleep 3
	echo "running enumerator"
	./testbed/synthesizer $(CONFIG_PATH)$(GRAMMAR_FILE) $(SAVING_PATH)$(POS_FILE) $(SAVING_PATH)$(NEG_FILE) $(CONFIG_PATH)$(CONFIG_FILE) $(CONFIG_PATH)$(SERVER_FILE_LOCAL) > $(SAVING_PATH)$(QUERY_FILE)


test-mode-P:
	echo "running execution server"
	cd $(EXECUTION_PATH) ; make run-server
	sleep 3
	echo "running enumerator"
	./testbed/synthesizer $(CONFIG_PATH)$(GRAMMAR_FILE) $(SAVING_PATH)$(POS_FILE) $(SAVING_PATH)$(NEG_FILE) $(CONFIG_PATH)$(CONFIG_FILE) $(CONFIG_PATH)$(SERVER_FILE) > $(SAVING_PATH)$(QUERY_FILE)

clean:
	-rm testbed/regex
	-rm testbed/synthesizer
	-rm testbed/synthesizer_debug









# ==================================== out dated ================================= 

r: 
	$(CC) -o ./example/gen ./example/gen.cpp -O2

regex-general-test:
	$(CC) -o ./testbed/regex $(SRC) -O2 -std=c++11 -pthread -lpcap

regex-general-test-g:
	$(CC) -o ./testbed/regex -g $(SRC) -std=c++11 -pthread -lpcap

profile-regex:
	valgrind --tool=callgrind --dump-instr=yes ./testbed/regex lang/regex.json example/xlarge.input example/config

test-pwd:
	$(CC) -o ./example/gen ./example/gen.cpp -O2
	./example/gen | tee example/pwd.input
	./testbed/regex lang/regex.json example/pwd.input example/config

testall:
	./testbed/synthesizer ./lang/netqre/netqre.json ./testbed/small_valid_tcp.pcap ./testbed/large_random_tcp.pcap ./lang/netqre/netqre.config ./lang/netqre/netqre_server_list.txt > testbed/log1 &
	./testbed/synthesizer ./lang/netqre/netqre.json ./testbed/small_valid_tcp.pcap ./testbed/large_random_tcp.pcap ./lang/netqre/netqre.config ./lang/netqre/netqre_server_list.txt > testbed/log2 &
	cd ./testbed; /bin/bash ./check_result.sh 2

kill:
	killall synthesizer

test-reg:
	./testbed/regex lang/regex_range_test.json example/pwd.input example/config

test-c:
	./testbed/regex lang/common_string.json example/xlarge2.input example/config

test-r:
	./example/gen > ./example/random.input
	./testbed/regex lang/regex.json example/random.input example/config
	cat example/random.input

regex6-g: lang/regex6.cpp lang/nfa.hpp lang/nfa_skip.hpp incomplete_execution.h search_tree.h search_tree.cpp search_graph.h search_graph.cpp syntax_tree.h syntax_tree.cpp
	g++-8 -o regex lang/regex6.cpp incomplete_execution.cpp search_tree.cpp search_graph.cpp syntax_tree.cpp -g -std=c++11


