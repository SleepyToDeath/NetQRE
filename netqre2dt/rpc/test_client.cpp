#include "client.hpp"
#include "interface.hpp"
#include <iostream>

using std::getline;
using Rubify::string;

std::unique_ptr<GeneralInterpreter> GeneralProgram::interpreter; 

/* netqre_client program.netqre test_data.ts server_list.txt */
int main(int argc, char *argv[]) 
{
	ifstream fin_p(argv[1]); // program
	string program;
	getline(fin_p, program);
	AbstractCode code(program, program);

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
	test_data.from_file("", argv[2]);
	auto result = interpreter->accept(code, true, test_data.to_handle());
	
}

