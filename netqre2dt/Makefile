all: parser

g: parser-g

SRC = parser.h syntax.h parser.cpp test_main.cpp 
SRC += interpreter.cpp interpreter.h op.hpp op.cpp
SRC += ../data-transducer/*.h ../data-transducer/*.cpp

parser: $(SRC)
	g++ -O2 -o parser $(SRC) -std=c++11

parser-g: $(SRC)
	g++ -g -o parser $(SRC) -std=c++11

test:
	cat ./testbed/basic.netqre | ./parser

clean:
	rm parser
