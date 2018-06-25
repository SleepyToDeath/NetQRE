all: regex

regex: lang/regex.cpp search_tree.h search_tree.cpp search_graph.h search_graph.cpp syntax_tree.h syntax_tree.cpp
	g++ -o regex lang/regex.cpp search_tree.cpp search_graph.cpp syntax_tree.cpp -O2 -std=c++11

regex-g: lang/regex.cpp search_tree.h search_tree.cpp search_graph.h search_graph.cpp syntax_tree.h syntax_tree.cpp
	g++ -o regex lang/regex.cpp search_tree.cpp search_graph.cpp syntax_tree.cpp -g -std=c++11

profile-regex:
	valgrind --tool=callgrind --dump-instr=yes ./regex

clean:
	-rm regex
