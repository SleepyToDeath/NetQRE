#include <rpc/server.h>
#include <iostream>
#include <string>
#include "client.hpp"
#include "../../synthesizer/lang/netqre/interface.hpp"

std::unique_ptr<GeneralInterpreter> GeneralProgram::interpreter = nullptr;

Netqre::Interpreter interpreter;
Netqre::NetqreParser parser;

int main(int argc, char *argv[]) {
	rpc::server srv(std::stoi(argv[3], nullptr));
	int packet_batch_size = 100;
//	auto e_train = prepare_training_set_from_pcap(string(argv[1]), string(argv[2]),3, 8);
//	auto e_test = prepare_test_set_from_pcap(string(argv[1]), string(argv[2]),3, 8);
//	auto e_train = shared_ptr<NetqreExample>(new NetqreExample());
//	auto e_test = shared_ptr<NetqreExample>(new NetqreExample());
	auto e_train = shared_ptr<NetqreExample>(new NetqreExample());
	e_train->from_file(argv[1], argv[2]);
	auto e_test = e_train->split();
	
//	prepare_example_from_pcap_one(string(argv[1]), string(argv[2]), packet_batch_size, e_train, e_test);

	srv.bind(Netqre::SERVICE_NAME, [&](std::string code, bool example_positive, int example_index) {
//		cout<<"Request received\n";
		auto ast = parser.parse(code);
		auto m = interpreter.interpret(ast);
		unique_ptr<Netqre::IntValue> ans;
		if (example_positive)
		{
			if (example_index < e_train->positive_token.size())
			{
				auto s = e_train->positive_token[example_index];
				ans = m->process(s);
			}
			else
			{
				auto s = e_test->positive_token[example_index - e_train->positive_token.size()];
				ans = m->process(s);
			}
		}
		else
		{
			if (example_index < e_train->negative_token.size())
			{
				auto s = e_train->negative_token[example_index];
				ans = m->process(s);
			}
			else
			{
				auto s = e_test->negative_token[example_index - e_train->negative_token.size()];
				ans = m->process(s);
			}
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

