#include "search_graph.h"
#include <experimental/random>

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
#ifdef DEBUG_PRINT_8
	std::cout<<"Cache size:"<<st->get_ctxt().cache->size()<<std::endl;
#endif
	}
#ifdef DEBUG_PRINT_9
	std::cout<<"================= Algorithm Start ===================\n";
#endif

	SyntaxTree* ans = enumerate(constraint);

#ifdef DEBUG_PRINT_9
	std::cout<<ans->to_string()<<std::endl;
	std::cout<<"================= Random Algorithm Start ===================\n";
#endif

	SyntaxTree* ans2 = enumerate_random(constraint, 10000);

#ifdef DEBUG_PRINT_9
	std::cout<<ans2->to_string()<<std::endl;
	std::cout<<"================= Naive Search Start ===================\n";
#endif

	SyntaxTree* ans3 = enumerate_naive(ans);

#ifdef DEBUG_PRINT_9
	std::cout<<ans3->to_string()<<std::endl;
	std::cout<<"===================== End =======================\n";
#endif
	return ans;
}

SyntaxTree* SearchGraph::search_recursive(SearchTreeContext ctxt, std::vector<SearchState*> state) {
	std::vector<LNode*> constraint;
	for (int i=0; i<state.size(); i++)
		constraint.push_back((*ctxt.cache)[state[i]]);
	return enumerate(constraint);
}

SyntaxTree* SearchGraph::enumerate_naive(SyntaxTree* answer) {
	std::vector<LNode*> constraint;
	std::vector<SyntaxTree*> this_round;
	std::vector<SyntaxTree*> next_round;

	constraint.clear();

	int depth = depth_threshold;
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
#ifdef DEBUG_PRINT_3
	std::cout<<"candidate size: "<<candidate.size()<<"\n";
#endif

					for (int j=0; j<candidate.size(); j++)
					{
#ifdef DEBUG_PRINT_3
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
#ifdef DEBUG_PRINT_3
	std::cout<<"Acc!\n";
#endif
							flag_new = true;
							next_round.push_back(candidate[j]);
							if (candidate[j]->complete() && answer->equal(candidate[j]))
							{
#ifdef DEBUG_PRINT_3
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
#ifdef DEBUG_PRINT_3
	std::cout<<"Rej!\n";
#endif
							delete candidate[j];
						}
					}
				}
			}
			for (int k=0; k<this_round.size(); k++)
				delete this_round[k];
#ifdef DEBUG_PRINT_3
	std::cout<<"This round size: "<<this_round.size()<<std::endl;
#endif
			this_round = next_round;
			next_round.clear();
		}
	}

	return nullptr;
}

SyntaxTree* SearchGraph::enumerate(std::vector<LNode*> constraint) {
	std::vector<SyntaxTree*> this_round;
	std::vector<SyntaxTree*> next_round;

//	return nullptr;

//	for (int depth = 0; depth<depth_threshold; depth++)
	int depth = depth_threshold;
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
#ifdef DEBUG_PRINT_2
	std::cout<<"candidate size: "<<candidate.size()<<"\n";
#endif

					for (int j=0; j<candidate.size(); j++)
					{
#ifdef DEBUG_PRINT_2
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
#ifdef DEBUG_PRINT_2
	std::cout<<"Acc!\n";
#endif
							flag_new = true;
							next_round.push_back(candidate[j]);
							if (candidate[j]->complete())
							{
#ifdef DEBUG_PRINT_2
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
#ifdef DEBUG_PRINT_2
	std::cout<<"Rej!\n";
#endif
							delete candidate[j];
						}
					}
				}
			}
			for (int k=0; k<this_round.size(); k++)
				delete this_round[k];
#ifdef DEBUG_PRINT_4
	std::cout<<"This round size: "<<this_round.size()<<std::endl;
#endif
			this_round = next_round;
			next_round.clear();
		}
	}

	return nullptr;
}

SyntaxTree* SearchGraph::enumerate_random(std::vector<LNode*> constraint, int batch_size) {
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
