#include "parser.h"
#include "interpreter.h"
#include <iostream>

using namespace std;

int main()
{
	string code;
	getline(cin, code);
	Netqre::NetqreParser parser;
	Netqre::Interpreter itp;
	auto ast = parser.parse(code);
	auto m = itp.interpret(ast);
}
