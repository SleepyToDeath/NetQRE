all: netqre

g: netqre-debug

d: test-netqre-debug

p: profile-regex

test: test-netqre

r: 
	g++-8 -o ./example/gen ./example/gen.cpp -O2

test-pwd:
	g++-8 -o ./example/gen ./example/gen.cpp -O2
	./example/gen | tee example/pwd.input
	./testbed/regex lang/regex.json example/pwd.input example/config

test-netqre:
	./testbed/synthesizer ./lang/netqre/netqre.json ./testbed/small_valid_tcp.pcap ./testbed/large_random_tcp.pcap ./lang/netqre/netqre.config

test-netqre-debug:
	gdb -ex run --args ./testbed/synthesizer_debug ./lang/netqre/netqre.json ./testbed/small_valid_tcp.pcap ./testbed/large_random_tcp.pcap ./lang/netqre/netqre.config

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
SRC2 += lang/netqre/*pp
SRC2 += ../netqre2dt/network_tokenizer/feature_vector.hpp
SRC2 += ../netqre2dt/interpreter.cpp
SRC2 += ../netqre2dt/op.cpp
SRC2 += ../netqre2dt/op.hpp
SRC2 += ../netqre2dt/parser.cpp
SRC2 += ../data-transducer/*pp

regex-general-test:
	g++-8 -o ./testbed/regex $(SRC) -O2 -std=c++11 -pthread -lpcap

regex-general-test-g:
	g++-8 -o ./testbed/regex -g $(SRC) -std=c++11 -pthread -lpcap

netqre:
	g++-8 -o ./testbed/synthesizer $(SRC2) -O2 --std=c++11 -lrpc -pthread -lpcap

netqre-debug:
	g++-8 -o ./testbed/synthesizer_debug $(SRC2) -g --std=c++11 -lrpc -pthread -lpcap

profile-regex:
	valgrind --tool=callgrind --dump-instr=yes ./testbed/regex lang/regex.json example/xlarge.input example/config

clean:
	-rm testbed/regex
	-rm testbed/synthesizer
	-rm testbed/synthesizer_debug
