#include "search_graph.h"

SearchGraph::SearchGraph(int depth_threshold0, SyntaxLeftHandSide* starting_symbol0, RHSToDivider* r2d0, SearchTreeCacheFactory* cache_pool0) {
	depth_threshold = depth_threshold0;
	starting_symbol = starting_symbol0;
	cache_pool = cache_pool0;
	r2d = r2d0;
}

SyntaxTree* SearchGraph::search_top_level(std::vector<ExampleType*> example) {
	std::vector<LNode*> constraint;
	for (int i=0; i<example.size(); i++)
	{
		SearchTree* st = new SearchTree(starting_symbol, example[i], r2d, depth_threshold);
		st->search();
		constraint.push_back(st->get_root());
	}
	return search(constraint);
}

SyntaxTree* SearchGraph::search_recursive(SearchTreeContext ctxt, std::vector<SearchState*> state) {
	std::vector<LNode*> constraint;
	for (int i=0; i<state.size(); i++)
		constraint.push_back(ctxt->cache[state[i]]);
	return search(constraint);
}

SyntaxTree* enumerate(std::vector<LNode*> constraint);
	std::vector<SyntaxTree*> this_round;
	std::vector<SyntaxTree*> next_round;
	SyntaxTree* s = new SyntaxTree(new SyntaxTreeNode(starting_symbol));
	this_round.push_back(s);

	for (int depth = 0; depth<depth_threshold; depth++)
	{
		bool flag_new = true;
		while (flag_new)
		{
			std::vector<SyntaxTree*> candidate;
			flag_new = false;
			for (int i=0; i<this_round.size(); i++)
			{
				candidate.clear();
				SyntaxTree* current = this_round[i];
				if (current->multi_mutate(current, depth, candidate))
				{

					for (int j=0; j<candidate.size(); j++)
					{
						bool flag_acc = true;
						for (int k=0; k<constraint.size(); k++)
						{
							if (!constraint[k]->accept(candidate[j]))
							{
								flag_acc = false;
								break;
							}
						}
						if (flag_acc)
						{
							next_round.push_back(candidate[j]);
							if (candidate[j]->complete())
							{
								for (k=j+1; k<candidate.size(); k++)
									delete candidate[k];
								for (k=0; i<this_round.size(); k++)
									delete this_round[k];
								for (k=0; i<next_round.size(); k++)
									delete next_round[k];
								return candidate[j];
							}
						}
						else
						{
							delete candidate[j];
						}
					}
				}
			}
			for (k=0; i<this_round.size(); k++)
				delete this_round[k];
			this_round = next_round;
			next_round.clear();
		}
	}
}
