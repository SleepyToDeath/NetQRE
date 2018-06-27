#ifndef SEARCH_GRAPH_H
#define SEARCH_GRAPH_H

#include "search_tree.h"

class SearchGraph {
	public:
	SearchGraph(int depth_threshold, SyntaxLeftHandSide* starting_symbol, RHSToDivider* r2d, SearchTreeCacheFactory<LNode*>* cache_pool);
	SyntaxTree* search_top_level(std::vector<ExampleType*> example);
	SyntaxTree* search_recursive(SearchTreeContext ctxt, std::vector<SearchState*> state);

	private:
	int depth_threshold;
	SyntaxLeftHandSide* starting_symbol;
	RHSToDivider* r2d;
	SearchTreeCacheFactory<LNode*>* cache_pool;

	SyntaxTree* enumerate_random(std::vector<LNode*> positive_constraint, std::vector<LNode*> negative_constraint, int batch_size);
};

#endif
