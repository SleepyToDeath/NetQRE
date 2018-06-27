#include "search_graph.h"
#include <experimental/random>

SearchGraph::SearchGraph(int depth_threshold0, SyntaxLeftHandSide* starting_symbol0, RHSToDivider* r2d0, SearchTreeCacheFactory<LNode*>* cache_pool0) {
	depth_threshold = depth_threshold0;
	starting_symbol = starting_symbol0;
	cache_pool = cache_pool0;
	r2d = r2d0;
}

SyntaxTree* SearchGraph::search_top_level(std::vector<ExampleType*> example) {
	std::vector<LNode*> positive_constraint;
	std::vector<LNode*> negative_constraint;
	for (int i=0; i<example.size(); i++)
	{
		SearchTree* st = new SearchTree(starting_symbol, example[i], r2d, cache_pool, depth_threshold);
		st->search();
		if (example[i]->is_positive())
			positive_constraint.push_back(st->get_root());
		else
			negative_constraint.push_back(st->get_root());
#ifdef DEBUG_PRINT_8
	std::cout<<"Cache size:"<<st->get_ctxt().cache->size()<<std::endl;
#endif
	}
#ifdef DEBUG_PRINT_9
	std::cout<<"================= DFS Start ===================\n";
#endif

	SyntaxTree* ans = enumerate_random(positive_constraint, negative_constraint, 1);

#ifdef DEBUG_PRINT_9
	if (ans!=nullptr)
		std::cout<<ans->to_string()<<std::endl;
	else
		std::cout<<"Not Found!\n";
	std::cout<<"================================================\n";
#endif

	return ans;
}

SyntaxTree* SearchGraph::search_recursive(SearchTreeContext ctxt, std::vector<SearchState*> state) {
	std::vector<LNode*> positive_constraint;
	std::vector<LNode*> negative_constraint;
	for (int i=0; i<state.size(); i++)
		if (state[i]->is_positive())
			positive_constraint.push_back((*ctxt.cache)[state[i]]);
		else
			negative_constraint.push_back((*ctxt.cache)[state[i]]);
	return enumerate_random(positive_constraint, negative_constraint, 1);
}

SyntaxTree* SearchGraph::enumerate_random(std::vector<LNode*> positive_constraint, 
											std::vector<LNode*> negative_constraint, 
											int batch_size) {
	std::vector<SyntaxTree*> this_round;
	std::vector<SyntaxTree*> buffer;

//	return nullptr;

//	for (int depth = 0; depth<depth_threshold; depth++)
	int depth = depth_threshold;
	{
		SyntaxTree* s = new SyntaxTree(new SyntaxTreeNode(starting_symbol));
		this_round.push_back(s);
		bool flag_new = true;
		while (this_round.size()>0)
		{
			std::vector<SyntaxTree*> candidate;
			int counter = 0;
			int done = -1;
			for (int i=0; i<this_round.size(); i++)
			{
				candidate.clear();
				SyntaxTree* current = this_round[i];
				if (current->multi_mutate(current, depth, &candidate))
				{
					for (int j=0; j<candidate.size(); j++)
					{
						bool flag_acc = true;
						/* check positive example */
						for (int k=0; k<positive_constraint.size(); k++)
						{
							if (!positive_constraint[k]->accept(candidate[j]))
							{
								flag_acc = false;
								break;
							}
						}
						/* check negative example */
						if (flag_acc && candidate[j]->complete())
						{
							for (int k=0; k<negative_constraint.size(); k++)
								if (negative_constraint[k]->accept(candidate[j]))
								{
									flag_acc = false;
									break;
								}
						}
						if (flag_acc)
						{
#ifdef DEBUG_PRINT_5
	std::cout<<"candidate "<<j<<" : ";
	std::cout<<candidate[j]->to_string()<<"\n";
#endif
							buffer.push_back(candidate[j]);
							counter++;
							if (candidate[j]->complete())
							{
								for (int k=j+1; k<candidate.size(); k++)
									delete candidate[k];
								for (int k=0; k<this_round.size(); k++)
									delete this_round[k];
								for (int k=0; k<buffer.size()-1; k++)
									delete buffer[k];
								return candidate[j];
							}
						}
						else
						{
							delete candidate[j];
						}
					}
				}
				/* only explore batch_size new nodes each time */
				done = i;
				if (counter >= batch_size)
					break;
			}
			for (int k=0; k<=done; k++)
				delete this_round[k];
			for (int k=done+1; k<this_round.size(); k++)
				buffer.push_back(this_round[k]);
//			std::cout<<"buffer size: "<<buffer.size()<<std::endl;
			if (buffer.size() <= batch_size)
			{
				this_round = buffer;
				buffer.clear();
			}
			else
			{
				this_round.clear();
				for (int k=0; k<batch_size; k++)
				{
//					int l = std::experimental::randint(0,bsize);
					this_round.push_back(buffer.back());
					buffer.pop_back();
				}
			}
		}
	}

	return nullptr;
}
