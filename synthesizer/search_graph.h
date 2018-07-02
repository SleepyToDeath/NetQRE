#ifndef SEARCH_GRAPH_H
#define SEARCH_GRAPH_H

#include "syntax_tree.h"
#include "search_tree.h"

class SearchGraph {
	public:
	/* search tree version */
	SearchGraph(int depth_threshold, SyntaxLeftHandSide* starting_symbol, RHSToDivider* r2d, SearchTreeCacheFactory<LNode*>* cache_pool);
	SyntaxTree* search_top_level(std::vector<ExampleType*> example);
	SyntaxTree* search_recursive(SearchTreeContext ctxt, std::vector<SearchState*> state);

	/* incomplete execution version */
	SearchGraph(int depth_threshold, IESyntaxLeftHandSide* starting_symbol);
	IESyntaxTree* search_top_level_v2(std::vector<IEExample*> example);

	private:
	int depth_threshold;
	SyntaxLeftHandSide* starting_symbol;

	/* search tree version */
	RHSToDivider* r2d;
	SearchTreeCacheFactory<LNode*>* cache_pool;
	SyntaxTree* enumerate_random(std::vector<LNode*> positive_constraint, std::vector<LNode*> negative_constraint, int batch_size);

	/* incomplete execution version */
	IESyntaxTree* enumerate_random_v2(std::vector<IEExample*> examples, int batch_size);
};

#endif
