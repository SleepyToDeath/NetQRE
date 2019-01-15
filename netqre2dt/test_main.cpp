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
	cout<<"#1\n";
	auto ast = parser.parse(code);
	cout<<"#2\n";
	auto m = itp.interpret(ast);
	cout<<"#3\n";
	auto mm = m;
}
