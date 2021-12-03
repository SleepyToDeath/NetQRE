#include "../parser.h"
#include "../interpreter.h"
#include "interface.hpp"
#include <iostream>
#include <fstream>

int main(int argc, char* argv[])
{
	Rubify::string program;
	std::ifstream fin(argv[1]); // grammar
	std::getline(fin, program);

	auto e_train = shared_ptr<NetqreExample>(new NetqreExample());
	auto e_test = shared_ptr<NetqreExample>(new NetqreExample());
	e_train->from_file(argv[2], argv[3]);
	e_test->from_file(argv[4], argv[5]);

	Netqre::NetqreParser parser;
	auto ast = parser.parse(program);
	Netqre::Interpreter intp;
	auto mac = intp.interpret(ast);
	mac->bind_context(e_train);
	e_test->positive_token.each( [&](auto s){
		unique_ptr<Netqre::IntValue> ans;
		ans = mac->process(s);
		std::cout<<ans->lower<<" "<<ans->upper<<std::endl;
	});
}

