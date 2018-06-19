#ifndef SEARCH_GRAPH_H
#define SEARCH_GRAPH_H

#include "search_tree.h"

class SearchGraph {
	public:
	SearchGraph(int depth_threshold);
	SyntaxTree* search_top_level(std::vector<ExampleType> example);
	SyntaxTree* search_recursive(SearchTreeContext ctxt, std::vector<SearchState> state);
	SyntaxTree* search(SearchGraphContext ctxt);

	private:
	int depth_threshold;
};

class SearchGraphContext {
	int depth;
	SyntaxTree* t;
};

#endif
