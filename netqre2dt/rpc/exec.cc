#include "../parser.h"
#include "../interpreter.h"
#include "interface.hpp"
#include <iostream>
#include <fstream>

int main(int argc, char* argv[])
{
	Rubify::string program;
	std::ifstream fin(argv[1]);

	auto exp_train = std::make_shared<NetqreExample>();
	exp_train->from_file(argv[2], argv[3]);
	auto exp_test = std::make_shared<NetqreExample>();
	exp_test->from_file(argv[4], argv[5]);


	while (std::getline(fin, program))
	{
		if (program == "")
			continue;

		std::cout << "program: " << program << std::endl;

		Netqre::NetqreParser parser;
		auto ast = parser.parse(program);
		Netqre::Interpreter intp;
		auto mac = intp.interpret(ast);
		mac->bind_context(exp_train);

		std::cout << "Positive results:" << std::endl;
		std::cout << "Total Number: " << exp_test->positive_token.size() << endl;

		exp_test->positive_token.each( [&](auto s) {
			unique_ptr<Netqre::IntValue> ans;
			ans = mac->process(s);
			std::cout<<ans->upper<<std::endl;
		});

		std::cout << "Negative results:" << std::endl;
		std::cout << "Total Number: " << exp_test->negative_token.size() << endl;

		exp_test->negative_token.each( [&](auto s) {
			unique_ptr<Netqre::IntValue> ans;
			ans = mac->process(s);
			std::cout<<ans->upper<<std::endl;
		});

		std::cout << std::endl;
	}
}

