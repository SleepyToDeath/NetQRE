#include "../parser.h"
#include "../interpreter.h"
#include <iostream>
#include <fstream>

int main(int argc, char* argv[])
{
	Rubify::string program;
	std::ifstream fin(argv[1]); // grammar
	std::getline(fin, program);

	Netqre::NetqreParser parser;
	auto ast = parser.parse(program);

	std::cerr<<std::endl;

	Netqre::Interpreter intp;
	intp.interpret(ast);
}

