#include "general.hpp"
#include "../../core/search_graph.h"
#include <vector>
#include <iostream>
#include <fstream>

using std::ifstream;
using std::endl;
using std::string;
using std::shared_ptr;
using std::cin;
using std::cout;
using std::vector;

/*
	3 arguments
	./regex grammar_file example_file config_file
*/
int main(int argc, char *argv[]) {
	ifstream fin_g(argv[1]);
	ifstream fin_e(argv[2]);
	ifstream fin_c(argv[3]);

	shared_ptr<GeneralSyntaxLeftHandSide> start = shared_ptr<GeneralSyntaxLeftHandSide>(new GeneralSyntaxLeftHandSide());
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
	start->from_json(json_g, nullptr);
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
	SearchGraph graph(search_depth, batch_size, answer_count, start);
	shared_ptr<IEExample> examples_up = examples;
	answer = graph.search_top_level_v2(examples_up);
	for (int i=0; i<answer.size(); i++)
		cout<<answer[i]->to_string()<<endl;
}
