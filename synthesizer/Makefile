all: regex-general-test

g: regex-general-test-g

test:
	./testbed/regex lang/regex.json example/xlarge.input example/config

regex6-g: lang/regex6.cpp lang/nfa.hpp lang/nfa_skip.hpp incomplete_execution.h search_tree.h search_tree.cpp search_graph.h search_graph.cpp syntax_tree.h syntax_tree.cpp
	g++ -o regex lang/regex6.cpp incomplete_execution.cpp search_tree.cpp search_graph.cpp syntax_tree.cpp -g -std=c++11

SRC = core/*pp
SRC += core/*h
SRC += lang/general/*pp 
SRC += lang/general/*h
SRC += lang/general/json11/json11.cpp
SRC += lang/general/json11/json11.hpp

regex-general-test:
	g++ -o ./testbed/regex $(SRC) -std=c++11

regex-general-test-g:
	g++ -o ./testbed/regex -g $(SRC) -std=c++11

profile-regex:
	valgrind --tool=callgrind --dump-instr=yes ./regex

clean:
	-rm testbed/regex
