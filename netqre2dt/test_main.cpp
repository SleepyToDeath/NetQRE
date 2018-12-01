#include "parser.h"
#include <iostream>

using namespace std;

int main()
{
	string code;
	getline(cin, code);
	NetqreParser parser;
	auto ast = parser.parse(code);
}
