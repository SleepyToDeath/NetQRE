#include "general.hpp"
#include "../../core/search_graph.h"
#include "merge_search.hpp"
//#include "../network_tokenizer/tcp_ip.hpp"
#include <vector>
#include <iostream>
#include <fstream>

using std::ifstream;
using std::endl;
using std::string;
using std::shared_ptr;
using std::unique_ptr;
using std::cin;
using std::cout;
using std::vector;

/*========== regex implementation =============*/
/*
#include "../regex/regex_interpreter.hpp"
std::unique_ptr<GeneralInterpreter> GeneralProgram::interpreter = unique_ptr<GeneralInterpreter>(new RegexInterpreter());
*/
/*=============================================*/

/*========== netqre implementation =============*/
#include "../netqre/interface.hpp"
std::unique_ptr<GeneralInterpreter> GeneralProgram::interpreter; 
int MergeSearch::force_search_factor;
/*=============================================*/

/*
	3 arguments
	./synthesizer grammar_file example_file config_file
*/
int main(int argc, char *argv[]) {

//	test_tcp_ip_parser(argv[1]);
//	return 0;

//	test_interpretor();
//	return 0;

	auto name_pos = string(argv[2]); // example pos
	auto name_neg = string(argv[3]); // example neg
	int threshold = 3;
//	std::cin>>threshold;
	auto examples = prepare_training_set_from_pcap(name_pos, name_neg, threshold);
	auto test_set = prepare_test_set_from_pcap(name_pos, name_neg, threshold);
	test_set->pos_offset = examples->positive_token.size();
	test_set->neg_offset = examples->negative_token.size();

	ifstream fin_g(argv[1]); // grammar
//	ifstream fin_e(argv[2]); // example
	ifstream fin_c(argv[4]); // config
	ifstream fin_s(argv[5]); // server list

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

	auto parser = shared_ptr<GeneralConfigParser>(new GeneralConfigParser());
//	shared_ptr<GeneralExample> examples = shared_ptr<GeneralExample>(new GeneralExample());
	int search_depth;
	int batch_size;
	int answer_count;
	int explore_rate;
	int threads;
	int minimal_example_size;

	/* input grammar */
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
		cout<<e<<endl;
		exit(1);
	}
	fin_g.close();

//	auto examples = prepare_examples(fin_e);

	/* input examples */
	/*
	int n_pos;
	int n_neg;
	fin_e>>n_pos;
	fin_e>>n_neg;
	for (int i=0; i<n_pos; i++)
	{
		string tmp;
		fin_e>>tmp;
		examples->positive.push_back(tmp);
	}
	for (int i=0; i<n_neg; i++)
	{
		string tmp;
		fin_e>>tmp;
		examples->negative.push_back(tmp);
	}

	fin_e.close();
	*/

	/* input config */
	fin_c>>search_depth;
	fin_c>>batch_size;
	fin_c>>explore_rate;
	fin_c>>answer_count;
	fin_c>>threads;
	fin_c>>minimal_example_size;
	fin_c>>MergeSearch::force_search_factor;

	/* prepare */
	parser->generate_input_dependent_syntax(examples);

	/* do searching */
	vector<shared_ptr<GeneralSyntaxTree> > answer;
	MergeSearch search_engine;
	answer = search_engine.search(search_depth, batch_size, explore_rate, answer_count, threads, minimal_example_size, parser->root, parser->rp, examples);
//	shared_ptr<IEExample> examples_up = examples;
//	answer = graph.search_top_level_v2(examples_up);
	cout<<"===========================================\n";
	if (answer.size() == 0)
		cout<<"Not found!"<<endl;
	for (int i=0; i<answer.size(); i++)
	{
		cout<<answer[i]->to_program()->accept(examples)<<endl;
		test_set->threshold = examples->threshold;
		test_set->indistinguishable_is_negative = examples->indistinguishable_is_negative;
		auto res = GeneralProgram::interpreter->test(answer[i]->to_string(), test_set);
		cout<<"Positive accuracy: "<<res.pos_accuracy<<endl;
		cout<<"Negative accuracy: "<<res.neg_accuracy<<endl;
		cout<<endl;
	}
}
