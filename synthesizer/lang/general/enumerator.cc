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
	/*
	ifstream fin_c(argv[2]); // config
	ifstream fin_s(argv[3]); // server list
	*/

	
	/* [!] LANGUAGE SPECIFIC */
	/*========== netqre implementation =============*/
	GeneralProgram::interpreter = unique_ptr<GeneralInterpreter>(
				new NetqreInterpreterInterface(vector<string>(), vector<int>()));
	provide_( [](string name) -> shared_ptr<GeneralExampleHandle> {
		if (name == "new_example_handle")
			return shared_ptr<NetqreExampleHandle>(new NetqreExampleHandle());
		else
			return require_(shared_ptr<GeneralExampleHandle>, name) ;
	});
	/*=============================================*/


	/*========== read config =============*/
	/*
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
	*/

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

	parser->generate_input_dependent_syntax(nullptr);
	/*=============================================*/

	shared_ptr<IESyntaxTree> starting = std::static_pointer_cast<IESyntaxTree>(
		SyntaxTree::factory->get_new(make_shared<SyntaxTreeNode>(parser->root)));

	auto seed = vector<shared_ptr<IESyntaxTree> >(1, starting);

	SearchGraph::enum_pure(seed, VisitPool());

}
