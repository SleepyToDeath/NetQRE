all: parser

g: parser-g

SRC = parser.h syntax.h parser.cpp test_main.cpp 
SRC += interpreter.cpp interpreter.h op.hpp op.cpp
SRC += ../data-transducer/*.h ../data-transducer/*.cpp

SRCC = parser.cpp test_main.cpp 
SRCC += interpreter.cpp op.hpp op.cpp
SRCC += ../data-transducer/*.cpp

CC = g++

parser: $(SRC)
	$(CC) $(SRCC) -O2 -o parser -std=c++11

parser-g: $(SRC)
	$(CC) -g -o parser $(SRCC) -std=c++11

test:
	cat ./testbed/basic2.netqre | ./parser

clean:
	rm parser
