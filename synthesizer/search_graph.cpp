#include "search_graph.h"

SearchGraph::SearchGraph(int depth_threshold0, SyntaxLeftHandSide* starting_symbol0, RHSToDivider* r2d0, SearchTreeCacheFactory<LNode*>* cache_pool0) {
	depth_threshold = depth_threshold0;
	starting_symbol = starting_symbol0;
	cache_pool = cache_pool0;
	r2d = r2d0;
}

SyntaxTree* SearchGraph::search_top_level(std::vector<ExampleType*> example) {
	std::vector<LNode*> constraint;
	for (int i=0; i<example.size(); i++)
	{
		SearchTree* st = new SearchTree(starting_symbol, example[i], r2d, cache_pool, depth_threshold);
		st->search();
		constraint.push_back(st->get_root());
	}
	return enumerate(constraint);
}

SyntaxTree* SearchGraph::search_recursive(SearchTreeContext ctxt, std::vector<SearchState*> state) {
	std::vector<LNode*> constraint;
	for (int i=0; i<state.size(); i++)
		constraint.push_back((*ctxt.cache)[state[i]]);
	return enumerate(constraint);
}

SyntaxTree* SearchGraph::enumerate(std::vector<LNode*> constraint) {
	std::vector<SyntaxTree*> this_round;
	std::vector<SyntaxTree*> next_round;

	for (int depth = 0; depth<depth_threshold; depth++)
	{
		SyntaxTree* s = new SyntaxTree(new SyntaxTreeNode(starting_symbol));
		this_round.push_back(s);
		bool flag_new = true;
		while (flag_new)
		{
			std::vector<SyntaxTree*> candidate;
			flag_new = false;
			for (int i=0; i<this_round.size(); i++)
			{
				candidate.clear();
				SyntaxTree* current = this_round[i];
				if (current->multi_mutate(current, depth, &candidate))
				{
#ifdef DEBUG_PRINT
	std::cout<<"candidate size: "<<candidate.size()<<"\n";
#endif

					for (int j=0; j<candidate.size(); j++)
					{
#ifdef DEBUG_PRINT
	std::cout<<"candidate "<<j<<" : ";
	std::cout<<candidate[j]->to_string()<<"\n";
#endif
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
#ifdef DEBUG_PRINT
	std::cout<<"Acc!\n";
#endif
							flag_new = true;
							next_round.push_back(candidate[j]);
							if (candidate[j]->complete())
							{
#ifdef DEBUG_PRINT
	std::cout<<"Gotcha!\n";
	std::cout<<"candidate "<<j<<" : ";
	std::cout<<candidate[j]->to_string()<<"\n";
#endif
								for (int k=j+1; k<candidate.size(); k++)
									delete candidate[k];
								for (int k=0; k<this_round.size(); k++)
									delete this_round[k];
								for (int k=0; k<next_round.size()-1; k++)
									delete next_round[k];
								return candidate[j];
							}
						}
						else
						{
#ifdef DEBUG_PRINT
	std::cout<<"Rej!\n";
#endif
							delete candidate[j];
						}
					}
				}
			}
			for (int k=0; k<this_round.size(); k++)
				delete this_round[k];
	std::cout<<"This round size: "<<this_round.size()<<std::endl;
			this_round = next_round;
			next_round.clear();
		}
	}

	return nullptr;
}
