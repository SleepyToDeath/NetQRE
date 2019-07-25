#include <rpc/server.h>
#include <iostream>
#include <string>
#include "client.hpp"
#include "../interpreter.h"
#include "../parser.h"

std::unique_ptr<GeneralInterpreter> GeneralProgram::interpreter = nullptr;

Netqre::Interpreter interpreter;
Netqre::NetqreParser parser;

int main(int argc, char *argv[]) {
	rpc::server srv(std::stoi(argv[3], nullptr));
	int packet_batch_size = 100;
	auto example = shared_ptr<NetqreExample>(new NetqreExample());
	example->from_file(argv[1], argv[2]);
	

	srv.bind(Netqre::SERVICE_NAME, [&](std::string code, bool example_positive, int example_index) {
		auto ast = parser.parse(code);
		auto m = interpreter.interpret(ast);
		unique_ptr<Netqre::IntValue> ans;
		if (example_positive)
		{
			auto s = e_train->positive_token[example_index];
			ans = m->process(s);
		}
		else
		{
			auto s = e_train->negative_token[example_index];
			ans = m->process(s);
		}

		Netqre::IntValueMsg ret;
		ret.lower = ans->lower;
		ret.upper = ans->upper;
		return ret;
	});

	srv.async_run(Netqre::THREAD_COUNT);

	while(true)
	{
		std::this_thread::sleep_for(std::chrono::seconds(100));
	}

	return 0;
}

