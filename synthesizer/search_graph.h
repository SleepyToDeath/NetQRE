#ifndef SEARCH_GRAPH_H
#define SEARCH_GRAPH_H

#include "search_tree.h"

class SearchGraph {
	public:
	SearchGraph(int depth_threshold, SyntaxLeftHandSide* starting_symbol, RHSToDivider* r2d);
	SyntaxTree* search_top_level(std::vector<ExampleType*> example);
	SyntaxTree* search_recursive(SearchTreeContext ctxt, std::vector<SearchState*> state);

	private:
	int depth_threshold;
	SyntaxLeftHandSide* starting_symbol;
	RHSToDivider* r2d;

	SyntaxTree* search(std::vector<LNode*> constraint);
};

#endif
