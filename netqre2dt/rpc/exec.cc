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

		/* get threshold part */
		auto p_t = program.split(" ");
		int threshold = std::stoi(p_t[1]);
		bool eq_is_neg = std::stoi(p_t[2]);
		auto is_pos = [&](int ans)-> bool {
			if (ans == threshold)
				return !eq_is_neg;
			else
				return ans > threshold;
		};

		/* get program part */
		program = p_t[0];

		/* initialize execution context */
		Netqre::NetqreParser parser;
		auto ast = parser.parse(program);
		Netqre::Interpreter intp;
		auto mac = intp.interpret(ast);
		mac->bind_context(exp_train);

		/* run positive */
		std::cout << "Positive results:" << std::endl;
		std::cout << "Total Number: " << exp_test->positive_token.size() << std::endl;

		int correct_count = 0;
		exp_test->positive_token.each( [&](auto s) {
			unique_ptr<Netqre::IntValue> ans;
			ans = mac->process(s);
			std::cout<<ans->upper<<", ";
			if (is_pos(ans->upper))
				correct_count++;
		});
		std::cout << std::endl;
		std::cout << "Accuracy: " << correct_count << "/" << exp_test->positive_token.size() << std::endl;

		/* run negative */
		std::cout << "Negative results:" << std::endl;
		std::cout << "Total Number: " << exp_test->negative_token.size() << endl;

		correct_count = 0;
		exp_test->negative_token.each( [&](auto s) {
			unique_ptr<Netqre::IntValue> ans;
			ans = mac->process(s);
			std::cout<<ans->upper<<", ";
			if (!is_pos(ans->upper))
				correct_count++;
		});
		std::cout << std::endl;
		std::cout << "Accuracy: " << correct_count << "/" << exp_test->negative_token.size() << std::endl;

	}
}

