#include "general.hpp"
#include "../../core/search_graph.h"
#include "../../core/merge_search.hpp"
#include <iostream>
#include <fstream>
#include "rubify.hpp"

using std::ifstream;
using std::endl;
using std::shared_ptr;
using std::unique_ptr;
using std::cin;
using std::cout;
using std::cerr;


/* [!] LANGUAGE SPECIFIC */
/*========== netqre implementation =============*/
#include "../../../netqre2dt/rpc/interface.hpp"
std::unique_ptr<GeneralInterpreter> GeneralProgram::interpreter; 
/*=============================================*/

/*
*/
int main(int argc, char *argv[]) {

	ifstream fin_g(argv[1]); // grammar
	ifstream fin_c(argv[4]); // config
	ifstream fin_s(argv[5]); // server list

	
	/*========== read server list =============*/
	int server_count;
	if (!fin_s.good())
		throw "Failed to open server list\n";
	fin_s >> server_count;
	cerr<<"Server number: "<<server_count<<endl;
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
	/*=============================================*/


	/* [!] LANGUAGE SPECIFIC */
	/*========== netqre implementation =============*/
	GeneralProgram::interpreter = unique_ptr<GeneralInterpreter>(
				new NetqreInterpreterInterface(servers, ports));
	/*=============================================*/




	/*========== read config =============*/
	int do_test;
	int search_depth;
	int batch_size;
	int answer_count;
	int explore_rate;
	int threads;
	int minimal_example_size;
	int force_search_factor;
	double accuracy;

	auto read_int_parameter = [](ifstream& fin, int& param) {
		string s;
		getline(fin, s);
		param = s.split(":")[-1].split(" ")[-1].to_i();
	};

	auto read_float_parameter = [](ifstream& fin, double& param) {
		string s;
		stringstream ss;
		getline(fin, s);
		ss << s.split(":")[-1].split(" ")[-1];
		ss >> param;
	};

	read_int_parameter(fin_c, do_test);
	read_int_parameter(fin_c, search_depth);
	read_int_parameter(fin_c, batch_size);
	read_int_parameter(fin_c, explore_rate);
	read_int_parameter(fin_c, answer_count);
	read_int_parameter(fin_c, threads);
	read_int_parameter(fin_c, minimal_example_size);
	read_int_parameter(fin_c, force_search_factor);
	read_float_parameter(fin_c, accuracy);
	/*=============================================*/




	/*========== read examples =============*/
	auto name_pos = string(argv[2]); // example pos
	auto name_neg = string(argv[3]); // example neg

	auto e_train_ = shared_ptr<NetqreExample>(new NetqreExample());
	e_train_->from_file(argv[2], argv[3]);
	auto e_test_ = e_train_;
	if (do_test)
		e_test_ = e_train_->split();
	
	auto e_train = static_pointer_cast<NetqreExampleHandle>(e_train_->to_handle());
	auto e_test = e_train;
	if (do_test)
		e_test = static_pointer_cast<NetqreExampleHandle>(e_test_->to_handle(
					e_train->positive_token.size(), 
					e_train->negative_token.size()));
	/*=============================================*/




	/*========== read grammar =============*/
	auto parser = shared_ptr<GeneralConfigParser>(new GeneralConfigParser());

	string string_g;
	while (!fin_g.eof())
	{
		string tmp;
		fin_g >> tmp;
		string_g.append(tmp);
	}
	shared_ptr<GJson> json_g = shared_ptr<GJson>(new GJson(string_g));
	try {
		parser->parse_config(json_g);
	}
	catch (string e)
	{
		cerr<<e<<endl;
		exit(1);
	}
	fin_g.close();

	parser->generate_input_dependent_syntax(e_train_);
	/*=============================================*/





	/*========== do searching =============*/
	vector<shared_ptr<GeneralSyntaxTree> > answer;
	MergeSearch search_engine;
	answer = search_engine.search(
				search_depth, 
				batch_size, 
				explore_rate, 
				answer_count, 
				accuracy,
				threads, 
				minimal_example_size, 
				force_search_factor, 
				parser->root, 
				parser->rp, 
				e_train);
	/*=============================================*/






	/*========== print result =============*/
	cerr<<"================== Result =========================\n";
	if (answer.size() == 0)
		cerr<<"Not found!"<<endl;
	for (int i=0; i<answer.size(); i++)
	{
		cout<<answer[i]->to_code().pos;
		cerr<<answer[i]->to_program()->accept(e_train)<<endl;

		if (do_test)
		{
			e_test->threshold = e_train->threshold;
			e_test->indistinguishable_is_negative = e_train->indistinguishable_is_negative;
			auto res = GeneralProgram::interpreter->test(answer[i]->to_string(), e_test);
			cerr<<"Positive accuracy: "<<res.pos_accuracy<<endl;
			cerr<<"Negative accuracy: "<<res.neg_accuracy<<endl;
			cerr<<endl;
		}
	}
	/*=============================================*/
}
