#include <rpc/server.h>
#include <iostream>
#include <string>
#include "client.hpp"
#include "interface.hpp"
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

	std::cout<<"Example size:" +_S_(example->positive_token.size()) + _S_(example->negative_token.size())<<endl;
	

	srv.bind(Netqre::SERVICE_NAME, [&](std::string code, bool example_positive, int example_index) {
		puts("Request:"+code+"["+_S_(example_index)+"]");
		auto ast = parser.parse(code);
		auto m = interpreter.interpret(ast);
		unique_ptr<Netqre::IntValue> ans;
		if (example_positive)
		{
			auto s = example->positive_token[example_index];
			ans = m->process(s);
		}
		else
		{
			auto s = example->negative_token[example_index];
			ans = m->process(s);
		}

		Netqre::IntValueMsg ret;
		ret.lower = ans->lower;
		ret.upper = ans->upper;
		puts("Reply:"+_S_(ret.lower)+" "+_S_(ret.upper));
		return ret;
	});

	srv.async_run(Netqre::THREAD_COUNT);

	while(true)
	{
		std::this_thread::sleep_for(std::chrono::seconds(100));
	}

	return 0;
}

