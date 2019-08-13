#include "client.hpp"
#include "interface.hpp"
#include <iostream>

using std::getline;
using Rubify::string;
using std::cout;

std::unique_ptr<GeneralInterpreter> GeneralProgram::interpreter; 

Netqre::NetqreParser parser;

/* netqre_client program.netqre test_data.ts server_list.txt */
int main(int argc, char *argv[]) 
{
	ifstream fin_p(argv[1]); // program
	vector< vector<string> > codes;
	while (fin_p.good())
	{
		string code_raw;
		getline(fin_p, code_raw);
		if (!code_raw.empty())
		{
			auto tmp = code_raw.split(" ");
			string indistinguishable_is_negative = tmp[-1];
			tmp.pop_back();
			string threshold = tmp[-1];
			tmp.pop_back();
			vector<string> code_parsed;
			code_parsed.push_back(tmp.join(""));
			code_parsed.push_back(threshold);
			code_parsed.push_back(indistinguishable_is_negative);
			codes.push_back(code_parsed);
//			puts(parser.parse(code_parsed[0])->to_s());
		}
	}

	ifstream fin_s(argv[3]); // server list
	int server_count;
	fin_s >> server_count;
	vector<string> servers;
	vector<int> ports;
	for (int i = 0; i<server_count; i++)
	{
		string tmp_s;
		int tmp_p;
		fin_s >> tmp_s;
		fin_s >> tmp_p;
		servers.push_back(tmp_s);
		ports.push_back(tmp_p);
	}

	GeneralProgram::interpreter = unique_ptr<GeneralInterpreter>(new NetqreInterpreterInterface(servers, ports));
	unique_ptr<GeneralInterpreter>& interpreter = GeneralProgram::interpreter;

	NetqreExample test_data;
	test_data.from_file(argv[2], "");
//	puts(test_data.positive_token.size());
//	puts(test_data.negative_token.size());

	cerr<<"Running queries!\n";
	cerr<<"Progress:\n"
	<<"|--------------------------------------------------|\n"
	<<"|=";
	int total = test_data.positive_token.size();

	/* fill the cache */
	auto total_handle = std::static_pointer_cast<NetqreExampleHandle>(test_data.to_handle());
	codes.each( [&](int index, auto code) {
		total_handle->threshold = code[1].to_i();
		total_handle->indistinguishable_is_negative = (code[2].to_i() == 1);
		interpreter->test(code[0], total_handle);
	});

	/* true run */
	test_data.shatter().each( [&] (int index, auto e) {
		auto test_handle = std::static_pointer_cast<NetqreExampleHandle>(e->to_handle(index));
		codes.each( [&](auto code) {
			test_handle->threshold = code[1].to_i();
			total_handle->indistinguishable_is_negative = (code[2].to_i() == 1);
			cout<< interpreter->test(code[0], test_handle).pos_accuracy << " ";
		});
		cout << endl;
		if (index * 50 / total != (index-1) * 50 / total)
			cerr<<"=";
	});
	cerr<<"|\n";
}

