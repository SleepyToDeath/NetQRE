#include "parser.h"
#include <iostream>

using namespace std;

int main()
{
	string code;
	getline(cin, code);
	Netqre::NetqreParser parser;
	auto ast = parser.parse(code);
}
