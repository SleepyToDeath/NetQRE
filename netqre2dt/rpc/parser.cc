#include "../parser.h"
#include <iostream>
#include <fstream>

int main(int argc, char* argv[])
{
	Netqre::NetqreParser parser;
	Rubify::string program;
	std::ifstream fin(argv[1]); // grammar
	std::getline(fin, program);
	parser.parse(program);
	std::cerr<<std::endl;
}

