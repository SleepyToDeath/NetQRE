#include "search_graph.h"
#include <experimental/random>
#include <algorithm>

SearchGraph::SearchGraph(int depth_threshold0, int batch_size0, int answer_count0, SyntaxLeftHandSide* starting_symbol0, RHSToDivider* r2d0, SearchTreeCacheFactory<LNode*>* cache_pool0) {
	batch_size = batch_size0;
	answer_count = answer_count0;
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

	SyntaxTree* ans = enumerate_random(positive_constraint, negative_constraint, 100);

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

	double progress = 0;

//	for (int depth = 0; depth<depth_threshold; depth++)
	int depth = depth_threshold;
	{
		std::cout<<"Depth:"<<depth<<std::endl;
		SyntaxTree* s = new SyntaxTree(new SyntaxTreeNode(starting_symbol));
		s->weight = 1;
		this_round.push_back(s);
		while (this_round.size()>0)
		{
			std::vector<SyntaxTree*> candidate;
			int counter = 0;
			int done = -1;
			bool flag_deadend = false;
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
						std::cout<<candidate[j]->to_string()<<std::endl;
//						if (candidate[j]->get_complexity() > 1000)
//						{
//							flag_acc = false;
//						}
//						else
//						{
							for (int k=0; k<positive_constraint.size(); k++)
							{
								if (!positive_constraint[k]->accept(candidate[j]))
								{
									flag_acc = false;
									break;
								}
							}
//						}
						/* check negative example */
						if (flag_acc && candidate[j]->is_complete())
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
							std::cout<<progress<<std::endl;
							std::cout<<candidate[j]->get_complexity()<<std::endl;
#ifdef DEBUG_PRINT_5
	std::cout<<"candidate "<<j<<" : ";
	std::cout<<candidate[j]->to_string()<<"\n";
#endif
							buffer.push_back(candidate[j]);
							counter++;
							if (candidate[j]->is_complete())
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
							progress += candidate[j]->weight;
							std::cout<<progress<<std::endl;
							std::cout<<candidate[j]->get_complexity()<<std::endl;
//							flag_deadend = true;
							delete candidate[j];
						}
					}
					if (candidate.size() == 0)
					{
						flag_deadend = true;
					}
				}
				/* only explore batch_size new nodes each time */
				done = i;
				if (counter >= batch_size)
					break;
			}

			/* gather all candidates in buffer in LRU order */
			for (int k=0; k<=done; k++)
				delete this_round[k];
			std::vector<SyntaxTree*> buffer2;
			for (int k=done+1; k<this_round.size(); k++)
				buffer2.push_back(this_round[k]);
			while (!buffer.empty())
			{
				buffer2.push_back(buffer.back());
				buffer.pop_back();
			}
			buffer = buffer2;
			std::sort(buffer.begin(), buffer.end(), compare_syntax_tree);
			std::cout<<"buffer size"<<buffer.size()<<std::endl;
			/*
			if (flag_deadend)
			{
				SyntaxTree* tmp = buffer[0];
				buffer[0] = buffer.back();
				buffer.back() = tmp;
			}
			*/

			this_round.clear();
			if (buffer.size() <= batch_size)
			{
				while(!buffer.empty())
				{
					this_round.push_back(buffer.back());
					buffer.pop_back();
				}
			}
			else
			{
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


SearchGraph::SearchGraph(int depth_threshold0, int batch_size0, int answer_count0, IESyntaxLeftHandSide* starting_symbol0) {
	batch_size = batch_size0;
	answer_count = answer_count0;
	depth_threshold = depth_threshold0;
	starting_symbol = starting_symbol0;
}

std::vector<IESyntaxTree*> SearchGraph::search_top_level_v2(std::vector<IEExample*> examples) {
	return enumerate_random_v2(examples);
}

std::vector<IESyntaxTree*> SearchGraph::enumerate_random_v2(std::vector<IEExample*> examples) {
	std::vector<IESyntaxTree*> this_round;
	std::vector<IESyntaxTree*> buffer;
	std::vector<IESyntaxTree*> answer;
	int answer_counter = 0;

	double progress = 0;

	double total_drop = 0;
	double complete_drop = 0;
//	for (int depth = 0; depth<depth_threshold; depth++)
	int depth = depth_threshold;
	{
//		std::cout<<"Depth:"<<depth<<std::endl;
		IESyntaxTree* s = new IESyntaxTree(new SyntaxTreeNode(starting_symbol));
		s->weight = 1;
		this_round.push_back(s);
		while (this_round.size()>0)
		{
			std::vector<IESyntaxTree*> candidate;
			std::vector<SyntaxTree*> tmp;
			int counter = 0;
			int done = -1;
			bool flag_deadend = false;
			for (int i=0; i<this_round.size(); i++)
			{
				candidate.clear();
				tmp.clear();
				IESyntaxTree* current = this_round[i];
				if (current->multi_mutate(current, depth, &tmp))
				{
					for (int j=0; j<tmp.size(); j++)
					{
						candidate.push_back(new IESyntaxTree(tmp[j]));
//						std::cout<<tmp[j]->weight<<" "<<candidate[j]->weight<<std::endl;;
						delete tmp[j];
					}
					for (int j=0; j<candidate.size(); j++)
					{
						bool flag_acc = true;
						std::cout<<candidate[j]->to_string()<<std::endl;
						for (int k=0; k<examples.size(); k++)
						{
							if (!candidate[j]->to_program()->accept(examples[k]))
							{
								total_drop += 1.0;
								if (candidate[j]->is_complete())
									complete_drop += 1.0;
								flag_acc = false;
								break;
							}
						}
//						std::cout<<"----------------------\n";
						if (flag_acc)
						{
//							std::cout<<"Progress:"<<progress<<" | +"<<candidate[j]->weight<<std::endl;
//							std::cout<<candidate[j]->get_complexity()<<std::endl;
							buffer.push_back(candidate[j]);
							counter++;
							if (candidate[j]->is_complete())
							{
								answer_counter ++;
								std::cout<<"ANSWER FOUND: "<<candidate[j]->to_string()<<std::endl;
								answer.push_back(candidate[j]);
								buffer.pop_back();
								if (answer_counter == answer_count)
								{
									/*
									for (int k=j+1; k<candidate.size(); k++)
										delete candidate[k];
									for (int k=0; k<this_round.size(); k++)
										delete this_round[k];
									for (int k=0; k<buffer.size()-1; k++)
										delete buffer[k];
										*/
									return answer;
								}
							}
						}
						else
						{
							progress += candidate[j]->weight;
//							std::cout<<"Progress: "<<progress*100.0<<"% | +"<<candidate[j]->weight
//								<<"  |  ending drop rate: "<<(complete_drop/total_drop)*100.0<<"%"<<std::endl;
//							std::cout<<candidate[j]->get_complexity()<<std::endl;
							flag_deadend = true;
							delete candidate[j];
						}
					}
					if (candidate.size() == 0)
					{
						flag_deadend = true;
					}
				}
				/* only explore batch_size new nodes each time */
				done = i;
				if (counter >= batch_size)
					break;
			}

			/* gather all candidates in buffer in LRU order */
			for (int k=0; k<=done; k++)
				delete this_round[k];
			std::vector<IESyntaxTree*> buffer2;
			for (int k=done+1; k<this_round.size(); k++)
				buffer2.push_back(this_round[k]);
			while (!buffer.empty())
			{
				buffer2.push_back(buffer.back());
				buffer.pop_back();
			}
			buffer = buffer2;
			std::sort(buffer.begin(), buffer.end(), compare_syntax_tree);
			std::cout<<"Progress: "<<progress*100.0<<"%"<<"   |   ";
			std::cout<<"Ending drop rate: "<<(complete_drop/total_drop)*100.0<<"%"<<"   |   ";
			std::cout<<"Buffer size: "<<buffer.size()<<"   |   ";
			std::cout<<"Answers found: "<<answer_counter<<std::endl;
			std::cout<<"One current sample: "<<(buffer[std::experimental::randint(0,(int)buffer.size()-1)]->to_string())<<std::endl;
			std::cout<<std::endl;
			/*
			if (flag_deadend)
			{
				IESyntaxTree* tmp = buffer[0];
				buffer[0] = buffer.back();
				buffer.back() = tmp;
			}
			*/

			this_round.clear();
			if (buffer.size() <= batch_size)
			{
				while(!buffer.empty())
				{
					this_round.push_back(buffer.back());
					buffer.pop_back();
				}
			}
			else
			{
				for (int k=0; k<batch_size; k++)
				{
					{
						int l = std::experimental::randint(0,(int)buffer.size()/2) + buffer.size()/2;
						if (l>=buffer.size())
							l = buffer.size()-1;
						IESyntaxTree* tmp = buffer.back();
						buffer.back() = buffer[l];
						buffer[l] = tmp;
					}
					this_round.push_back(buffer.back());
					buffer.pop_back();
				}
			}
		}
	}

	return answer;

}

