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
	int flow_batch_size = 8;
	int packet_batch_size = 100;
//	std::cin>>threshold;
//	auto e_train_ = prepare_training_set_from_pcap(name_pos, name_neg, threshold, flow_batch_size);
//	auto e_test_ = prepare_test_set_from_pcap(name_pos, name_neg, threshold, flow_batch_size);
	/*
	auto e_train_ = shared_ptr<NetqreExample>(new NetqreExample());
	auto e_test_ = shared_ptr<NetqreExample>(new NetqreExample());
	prepare_example_from_pcap_one(name_pos, name_neg, packet_batch_size, e_train_, e_test_);
	*/
	auto e_train_ = shared_ptr<NetqreExample>(new NetqreExample());
	e_train_->from_file(argv[2], argv[3]);
	auto e_test_ = e_train_->split();
	

	auto e_train = shared_ptr<NetqreExampleHandle>(new NetqreExampleHandle());
	auto e_test = shared_ptr<NetqreExampleHandle>(new NetqreExampleHandle());
	e_test->pos_offset = e_train->positive_token.size();
	e_test->neg_offset = e_train->negative_token.size();
	for (int i=0; i<e_train_->positive_token.size(); i++)
		e_train->positive_token.push_back(i);
	for (int i=0; i<e_train_->negative_token.size(); i++)
		e_train->negative_token.push_back(i);
	for (int i=0; i<e_test_->positive_token.size(); i++)
		e_test->positive_token.push_back(i+e_test->pos_offset);
	for (int i=0; i<e_test_->negative_token.size(); i++)
		e_test->negative_token.push_back(i+e_test->neg_offset);

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
//	shared_ptr<GeneralExample> e_train = shared_ptr<GeneralExample>(new GeneralExample());
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

//	auto e_train = prepare_examples(fin_e);

	/* input e_train */
	/*
	int n_pos;
	int n_neg;
	fin_e>>n_pos;
	fin_e>>n_neg;
	for (int i=0; i<n_pos; i++)
	{
		string tmp;
		fin_e>>tmp;
		e_train->positive.push_back(tmp);
	}
	for (int i=0; i<n_neg; i++)
	{
		string tmp;
		fin_e>>tmp;
		e_train->negative.push_back(tmp);
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
	parser->generate_input_dependent_syntax(e_train_);

	/* do searching */
	vector<shared_ptr<GeneralSyntaxTree> > answer;
	MergeSearch search_engine;
	answer = search_engine.search(search_depth, batch_size, explore_rate, answer_count, threads, minimal_example_size, parser->root, parser->rp, e_train);
//	shared_ptr<IEExample> examples_up = e_train;
//	answer = graph.search_top_level_v2(examples_up);
	cout<<"===========================================\n";
	if (answer.size() == 0)
		cout<<"Not found!"<<endl;
	for (int i=0; i<answer.size(); i++)
	{
		cout<<answer[i]->to_program()->accept(e_train)<<endl;
		e_test->threshold = e_train->threshold;
		e_test->indistinguishable_is_negative = e_train->indistinguishable_is_negative;
		auto res = GeneralProgram::interpreter->test(answer[i]->to_string(), e_test);
		cout<<"Positive accuracy: "<<res.pos_accuracy<<endl;
		cout<<"Negative accuracy: "<<res.neg_accuracy<<endl;
		cout<<endl;
	}
}
