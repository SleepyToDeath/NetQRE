#include "general.hpp"
#include "../../core/search_graph.h"
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
#include "../regex_interpreter.hpp"
std::unique_ptr<GeneralInterpreter> GeneralProgram::interpreter = unique_ptr<GeneralInterpreter>(new RegexInterpreter());
/*=============================================*/

/*
	3 arguments
	./regex grammar_file example_file config_file
*/
int main(int argc, char *argv[]) {

//	test_interpretor();
//	return 0;

	ifstream fin_g(argv[1]); // grammar
	ifstream fin_e(argv[2]); // example
	ifstream fin_c(argv[3]); // config

	auto parser = shared_ptr<GeneralConfigParser>(new GeneralConfigParser());
	shared_ptr<GeneralExample> examples = shared_ptr<GeneralExample>(new GeneralExample());
	int search_depth;
	int batch_size;
	int answer_count;

	/* input grammar */
	string string_g;
	while (!fin_g.eof())
	{
		string tmp;
		fin_g >> tmp;
		string_g.append(tmp);
	}
	shared_ptr<GJson> json_g = shared_ptr<GJson>(new GJson(string_g));
	parser->parse_config(json_g);
	fin_g.close();

	/* input examples */
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

	/* input config */
	fin_c>>search_depth;
	fin_c>>batch_size;
	fin_c>>answer_count;

	/* do searching */
	vector<shared_ptr<IESyntaxTree> > answer;
	SearchGraph graph(search_depth, batch_size, answer_count, parser->root, parser->rp);
	shared_ptr<IEExample> examples_up = examples;
	answer = graph.search_top_level_v2(examples_up);
	if (answer.size() == 0)
		cout<<"Not found!"<<endl;
	for (int i=0; i<answer.size(); i++)
		cout<<answer[i]->to_string()<<endl;
}
