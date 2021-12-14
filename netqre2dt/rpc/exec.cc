#include "../parser.h"
#include "../interpreter.h"
#include "interface.hpp"
#include <iostream>
#include <fstream>

int main(int argc, char* argv[])
{
	Rubify::string program;
	std::ifstream fin(argv[1]);
	std::getline(fin, program);

	auto exp = shared_ptr<NetqreExample>(new NetqreExample());
	exp->from_file(argv[2], argv[3]);

	Netqre::NetqreParser parser;
	auto ast = parser.parse(program);
	Netqre::Interpreter intp;
	auto mac = intp.interpret(ast);
	mac->bind_context(exp);

	exp->positive_token.each( [&](auto s) {
		unique_ptr<Netqre::IntValue> ans;
		ans = mac->process(s);
		std::cout<<ans->lower<<" "<<ans->upper<<std::endl;
	});
}

