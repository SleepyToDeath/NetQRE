all: regex-general-test

g: regex-general-test-g

p: profile-regex

r: 
	g++ -o ./example/gen ./example/gen.cpp -O2

test-pwd:
	g++ -o ./example/gen ./example/gen.cpp -O2
	./example/gen | tee example/pwd.input
	./testbed/regex lang/regex.json example/pwd.input example/config

test:
	./testbed/regex lang/regex_range_test.json example/pwd.input example/config

test-c:
	./testbed/regex lang/common_string.json example/xlarge2.input example/config

test-r:
	./example/gen > ./example/random.input
	./testbed/regex lang/regex.json example/random.input example/config
	cat example/random.input

regex6-g: lang/regex6.cpp lang/nfa.hpp lang/nfa_skip.hpp incomplete_execution.h search_tree.h search_tree.cpp search_graph.h search_graph.cpp syntax_tree.h syntax_tree.cpp
	g++ -o regex lang/regex6.cpp incomplete_execution.cpp search_tree.cpp search_graph.cpp syntax_tree.cpp -g -std=c++11

SRC = core/*pp
#SRC += lang/network_tokenizer/*pp
SRC += core/*h
SRC += lang/general/*pp 
SRC += lang/general/*h
SRC += lang/general/json11/json11.cpp
SRC += lang/general/json11/json11.hpp

regex-general-test:
	g++ -o ./testbed/regex $(SRC) -O2 -std=c++11 -pthread -lpcap

regex-general-test-g:
	g++ -o ./testbed/regex -g $(SRC) -std=c++11 -pthread -lpcap

profile-regex:
	valgrind --tool=callgrind --dump-instr=yes ./testbed/regex lang/regex.json example/xlarge.input example/config

clean:
	-rm testbed/regex
