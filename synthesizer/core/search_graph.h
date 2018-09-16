#ifndef SEARCH_GRAPH_H
#define SEARCH_GRAPH_H

#include "syntax_tree.h"
#include "incomplete_execution.h"

class SearchGraph {
	public:

	/* incomplete execution version */
	SearchGraph(int depth_threshold, int batch_size, int answer_count, shared_ptr<IESyntaxLeftHandSide> starting_symbol);
	std::vector<shared_ptr<IESyntaxTree> > search_top_level_v2(shared_ptr<IEExample> example);

	private:
	int depth_threshold;
	int batch_size;
	int answer_count;
	shared_ptr<SyntaxLeftHandSide> starting_symbol;

	/* incomplete execution version */
	std::vector< shared_ptr<IESyntaxTree> > enumerate_random_v2(shared_ptr<IEExample> examples);
};

#endif