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
	ifstream fin_c(argv[2]); // config
	ifstream fin_s(argv[3]); // server list

	
	/*========== read server list =============*/
	int server_count;
	if (!fin_s.good())
	{
		cerr<< "Failed to open server list\n";
		throw "Failed to open server list\n";
	}
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
	provide_( [](string name) -> shared_ptr<GeneralExampleHandle> {
		if (name == "new_example_handle")
			return shared_ptr<NetqreExampleHandle>(new NetqreExampleHandle());
		else
			return require_(shared_ptr<GeneralExampleHandle>, name) ;
	});
	/*=============================================*/




	/*========== read config =============*/
	map<string, int> int_config;
	map<string, double> float_config;

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

	read_int_parameter(fin_c, int_config["do_test"]);
	read_int_parameter(fin_c, int_config["search_depth"]);
	read_int_parameter(fin_c, int_config["batch_size"]);
	read_int_parameter(fin_c, int_config["explore_rate"]);
	read_int_parameter(fin_c, int_config["answer_count"]);
	read_int_parameter(fin_c, int_config["threads"]);
	read_int_parameter(fin_c, int_config["minimal_example_size"]);
	read_int_parameter(fin_c, int_config["force_search_factor"]);
	read_float_parameter(fin_c, float_config["accuracy"]);
	read_float_parameter(fin_c, float_config["accuracy_exp"]);
	read_int_parameter(fin_c, int_config["give_up_count"]);
	read_int_parameter(fin_c, int_config["VERBOSE_MODE"]);

	provide_([&](string name)->int {
		return int_config[name];
	});

	provide_([&](string name)->double {
		return float_config[name];
	});

	/*=============================================*/




	/*========== read examples =============*/
	auto e_train_ = shared_ptr<NetqreExample>(new NetqreExample());
	e_train_->from_file(argv[4], argv[5]);
	auto e_train = static_pointer_cast<NetqreExampleHandle>(e_train_->to_handle());

/*
	auto e_test_ = shared_ptr<NetqreExample>(new NetqreExample());
	auto e_test = e_train;
	if (require_(int, "do_test"))
	{
		e_test_->from_file(argv[6], argv[7]);
		e_test = static_pointer_cast<NetqreExampleHandle>(e_test_->to_handle(
						e_train->positive_token.size(), e_train->negative_token.size()));
	}
	*/
	
	provide_([&](string name)->shared_ptr<NetqreExample> {
		if (name == "global_example")
			return e_train_;
		else
			return require_(shared_ptr<NetqreExample>, name);
	});
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
	provide_( [&](Rubify::string name)->std::thread::id {
		if (name == "master_id")
			return std::this_thread::get_id();
		else
			return require_(std::thread::id, name);
	});

	errputs("Master thread id:" + _S_(std::this_thread::get_id()));

	vector<shared_ptr<GeneralSyntaxTree> > answer;
	MergeSearch search_engine;
	try{
		answer = search_engine.search(parser->root, parser->rp, e_train);
	}
	catch(Rubify::string name)
	{
		cerr<<name<<endl;
		exit(1);
	}
	/*=============================================*/






	/*========== print result =============*/
	cerr<<"================== Result =========================\n";
	if (answer.size() == 0)
		cerr<<"Not found!"<<endl;
	for (int i=0; i<answer.size(); i++)
	{
		cerr<<answer[i]->to_code().pos<<endl;
		cerr<<answer[i]->to_program()->accept(e_train,{require_(double, "accuracy")})<<endl;

		cout<<answer[i]->to_code().pos<<" "<<std::fixed
						<<static_cast<int>(e_train->threshold)<<" "
						<<static_cast<int>(e_train->indistinguishable_is_negative)<<endl;

		{
			cerr<<"Training Set:"<<endl;
			auto res = GeneralProgram::interpreter->test(answer[i]->to_string(), e_train);
			cerr<<"Positive accuracy: "<<res.pos_accuracy<<endl;
			cerr<<"Negative accuracy: "<<res.neg_accuracy<<endl;
			cerr<<endl;
		}

/*
		if (require_(int, "do_test"))
		{
			cerr<<"Testing Set:"<<endl;
			e_test->threshold = e_train->threshold;
			e_test->indistinguishable_is_negative = e_train->indistinguishable_is_negative;
			auto res = GeneralProgram::interpreter->test(answer[i]->to_string(), e_test);
			cerr<<"Positive accuracy: "<<res.pos_accuracy<<endl;
			cerr<<"Negative accuracy: "<<res.neg_accuracy<<endl;
			cerr<<endl;
		}
		*/
	}
	errputs(require_(shared_ptr<NetqreExample>, "global_example")->to_s());
	/*=============================================*/
}
