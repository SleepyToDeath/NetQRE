CCC = g++
CCFLAGS= -O2
LEX = flex
LFLAGS= -8     
YACC= bison 
YFLAGS= -d -t -y -v
PARSER = parser/

RM = /bin/rm -f

default: apps

apps: heavy_hitter heavy_hitter_baseline superspreader superspreader_baseline entropy entropy_baseline bandwidth_recorder \
			heavy_hitter_timeinterval flood flood2 syn_flood syn_flood_baseline flow_num flow_num_baseline sip
dupack: apps/dupack.cpp
	g++ apps/dupack.cpp -lpcap -o dupack -I library -std=c++11 -O3

flow_num: apps/flow_num.cpp
	g++ apps/flow_num.cpp -lpcap -o flow_num -I library -std=c++0x -O3

flow_num_baseline: apps/flow_num_baseline.cpp
	g++ apps/flow_num_baseline.cpp -lpcap -o flow_num_baseline -I library -std=c++0x -O3

sip_call: apps/sip_call.cpp
	g++ apps/sip_call.cpp -lpcap -o sip_call -I library -std=c++0x -O3

sip: apps/sip.cpp
	g++ apps/sip.cpp -lpcap -o sip -I library -std=c++0x -O3

syn_flood: apps/syn_flood.cpp
	g++ apps/syn_flood.cpp -lpcap -o syn_flood -I library -std=c++0x -O3

syn_flood_baseline: apps/syn_flood_baseline.cpp
	g++ apps/syn_flood_baseline.cpp -lpcap -o syn_flood_baseline -I library -std=c++0x -O3

unique_src: apps/unique_src.cpp
	g++ apps/unique_src.cpp -lpcap -o unique_src -I library -std=c++0x -O3

flood: apps/flood.cpp
	g++ apps/flood.cpp -lpcap -o flood -I library -std=c++0x -O3

flood2: apps/flood2.cpp
	g++ apps/flood2.cpp -lpcap -o flood2 -I library -std=c++0x -O3

bandwidth_recorder: apps/bandwidth_recorder.cpp
	g++ apps/bandwidth_recorder.cpp -lpcap -o bandwidth_recorder -I library -std=c++0x -O3

heavy_hitter: apps/heavy_hitter.cpp
	g++ apps/heavy_hitter.cpp -lpcap -o heavy_hitter -I library -std=c++0x -O3

heavy_hitter_baseline: apps/heavy_hitter_baseline.cpp
	g++ apps/heavy_hitter_baseline.cpp -lpcap -o heavy_hitter_baseline -I library -std=c++0x -O3

heavy_hitter_timeinterval: apps/heavy_hitter_timeinterval.cpp
	g++ apps/heavy_hitter_timeinterval.cpp -lpcap -o heavy_hitter_timeinterval -I library -std=c++0x -O3

superspreader: apps/superspreader.cpp
	g++ apps/superspreader.cpp -lpcap -o superspreader -I library -std=c++0x -O3

superspreader_baseline: apps/superspreader_baseline.cpp
	g++ apps/superspreader_baseline.cpp -lpcap -o superspreader_baseline -I library -std=c++0x -O3

entropy: apps/entropy.cpp
	g++ apps/entropy.cpp -lpcap -o entropy -I library -std=c++0x -O3

entropy_baseline: apps/entropy_baseline.cpp
	g++ apps/entropy_baseline.cpp -lpcap -o entropy_baseline -I library -std=c++0x -O3

testexpr: ast.cpp ast.h
	${CCC} ${CCFLAGS} ast.cpp -o ast -std=c++11

parser: y.tab.o lex.yy.o ast.o netqrec.o
	${CCC} ${CCFLAGS} lex.yy.o y.tab.o ast.o netqrec.o -o parser -lfl -std=c++11  

compiler: y.tab.o lex.yy.o ast.o
	${CCC} ${CCFLAGS} lex.yy.o y.tab.o ast.o netqrec.cpp  fsm.cpp  -o compiler -lfl -std=c++11  

parser.o: ast.h netqrec.cpp
	${CCC} -c netqrec.cpp -o y.tab.o -std=c++11 

netqrec.o: ast.h netqrec.cpp
	${CCC} -c netqrec.cpp ../core2/tree.cpp ../core2/fsm.cpp -o y.tab.o -std=c++11 

ast.o: ast.cpp ast.h
	${CCC} -c ast.cpp -std=c++11

y.tab.o: grammar.y
	${YACC} ${YFLAGS} grammar.y
	${CCC} ${CCFLAGS} y.tab.c -c -std=c++11 

grammar: grammar1.y
	${YACC} ${YFLAGS} grammar1.y
	${CCC} ${CCFLAGS} y.tab.c -c -std=c++11 

lex.yy.o: lex.l
	${LEX} $(LFLAGS) lex.l
	${CCC} ${CCFLAGS} lex.yy.c -c -std=c++11 

clean:
	rm -f lex.yy.* y.tab.* *.o parser compiler
	rm -f heavy_hitter heavy_hitter_baseline superspreader superspreader_baseline entropy entropy_baseline bandwidth_recorder \
		    heavy_hitter_timeinterval flood flood2 syn_flood syn_flood_baseline flow_num flow_num_baseline sip

rc:
	make clean
	make compiler
