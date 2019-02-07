#include <rpc/server.h>
#include <iostream>
#include "client.hpp"
#include "../../synthesizer/lang/netqre/interface.hpp"

std::unique_ptr<GeneralInterpreter> GeneralProgram::interpreter = nullptr;

Netqre::Interpreter interpreter;
Netqre::NetqreParser parser;

int main(int argc, char *argv[]) {
	rpc::server srv(Netqre::SERVICE_PORT + (argv[3][0] - '0') * 10 + argv[3][1] - '0');
	auto e = prepare_examples_from_pcap(string(argv[1]), string(argv[2]),3);

	srv.bind(Netqre::SERVICE_NAME, [&](std::string code, bool example_positive, int example_index) {
//		cout<<"Request received\n";
		auto ast = parser.parse(code);
		auto m = interpreter.interpret(ast);
		unique_ptr<Netqre::IntValue> ans;
		if (example_positive)
		{
			auto s = e->positive_token[example_index];
			ans = m->process(s);
		}
		else
		{
			auto s = e->negative_token[example_index];
			ans = m->process(s);
		}

		Netqre::IntValueMsg ret;
		ret.lower = ans->lower;
		ret.upper = ans->upper;
//		cout<<"Request handled\n";
		return ret;
	});

//	srv.run();

	srv.async_run(Netqre::THREAD_COUNT);

	while(true)
	{
		std::this_thread::sleep_for(std::chrono::seconds(100));
	}

	return 0;
}

