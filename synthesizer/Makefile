all: regex4

g: regex4-g

regex: lang/regex.cpp search_tree.h search_tree.cpp search_graph.h search_graph.cpp syntax_tree.h syntax_tree.cpp
	g++ -o regex lang/regex.cpp search_tree.cpp search_graph.cpp syntax_tree.cpp -O2 -std=c++11

regex-g: lang/regex.cpp search_tree.h search_tree.cpp search_graph.h search_graph.cpp syntax_tree.h syntax_tree.cpp
	g++ -o regex lang/regex.cpp search_tree.cpp search_graph.cpp syntax_tree.cpp -g -std=c++11

regex2: lang/regex2.cpp search_tree.h search_tree.cpp search_graph.h search_graph.cpp syntax_tree.h syntax_tree.cpp
	g++ -o regex lang/regex2.cpp search_tree.cpp search_graph.cpp syntax_tree.cpp -O2 -std=c++11

regex2-g: lang/regex2.cpp search_tree.h search_tree.cpp search_graph.h search_graph.cpp syntax_tree.h syntax_tree.cpp
	g++ -o regex lang/regex2.cpp search_tree.cpp search_graph.cpp syntax_tree.cpp -g -std=c++11

regex3: lang/regex3.cpp search_tree.h search_tree.cpp search_graph.h search_graph.cpp syntax_tree.h syntax_tree.cpp
	g++ -o regex lang/regex3.cpp search_tree.cpp search_graph.cpp syntax_tree.cpp -O2 -std=c++11

regex3-g: lang/regex3.cpp search_tree.h search_tree.cpp search_graph.h search_graph.cpp syntax_tree.h syntax_tree.cpp
	g++ -o regex lang/regex3.cpp search_tree.cpp search_graph.cpp syntax_tree.cpp -g -std=c++11

regex4: lang/regex4.cpp incomplete_execution.cpp incomplete_execution.h search_tree.h search_tree.cpp search_graph.h search_graph.cpp syntax_tree.h syntax_tree.cpp
	g++ -o regex lang/regex4.cpp incomplete_execution.cpp search_tree.cpp search_graph.cpp syntax_tree.cpp -O2 -std=c++11

regex4-g: lang/regex4.cpp incomplete_execution.h search_tree.h search_tree.cpp search_graph.h search_graph.cpp syntax_tree.h syntax_tree.cpp
	g++ -o regex lang/regex4.cpp incomplete_execution.cpp search_tree.cpp search_graph.cpp syntax_tree.cpp -g -std=c++11

profile-regex:
	valgrind --tool=callgrind --dump-instr=yes ./regex

clean:
	-rm regex
