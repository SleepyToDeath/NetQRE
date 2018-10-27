#include "search_graph.h"
#include <experimental/random>
#include <algorithm>
#include <cmath>
#include <unordered_set>
//#define VERBOSE_MODE

using std::endl;
using std::cout;

SearchGraph::SearchGraph(int depth_threshold, 
				int batch_size, 
				int explore_rate,
				int answer_count, 
				shared_ptr<IESyntaxLeftHandSide> starting_symbol, 
				shared_ptr<RedundancyPlan> rp ) {
	this->batch_size = batch_size;
	this->answer_count = answer_count;
	this->depth_threshold = depth_threshold;
	this->starting_symbol = starting_symbol;
	this->explore_rate = explore_rate;
	this->rp = rp;
}

std::vector<shared_ptr<IESyntaxTree> > SearchGraph::search_top_level_v2(shared_ptr<IEExample> examples) {
	return enumerate_random_v2(examples);
}

std::vector< shared_ptr<IESyntaxTree> > SearchGraph::enumerate_random_v2(shared_ptr<IEExample> examples) {
	std::vector<shared_ptr<IESyntaxTree> > this_round;
	std::vector<shared_ptr<IESyntaxTree> > buffer;
	std::vector<shared_ptr<IESyntaxTree> > answer;
	std::unordered_set<shared_ptr<SyntaxTree>, HashSyntaxTree, CmpSyntaxTree > visited;

	int answer_counter = 0;
	int search_counter = 0;
	int helper_counter = 0;
	double progress = 0;
	double total_drop = 0;
	double complete_drop = 0;

	int depth = depth_threshold;
	{
		shared_ptr<IESyntaxTree> s = std::static_pointer_cast<IESyntaxTree>(SyntaxTree::factory->get_new(shared_ptr<SyntaxTreeNode>(new SyntaxTreeNode(starting_symbol))));
		s->weight = 1;
		this_round.push_back(s);
		visited.clear();
		visited.insert(s);
		while (this_round.size()>0)
		{
			std::vector<shared_ptr<IESyntaxTree> > candidate;
			auto tmp = shared_ptr<SyntaxTree::Queue>(new SyntaxTree::Queue());
			int counter = 0;
			int done = -1;
			/* start this round */
			for (int i=0; i<this_round.size(); i++)
			{
				candidate.clear();
				tmp->q.clear();
				shared_ptr<IESyntaxTree> current = this_round[i];
				/* explore new nodes */
				#ifdef VERBOSE_MODE
				std::cout<<"[Source!]"<<current->get_complexity()<<" | "<<current->to_string()<<std::endl;
				#endif
				if (current->multi_mutate(current, depth, tmp))
				{
					/* push to buffer */
					for (int j=0; j<tmp->q.size(); j++)
					{
						auto explored = std::static_pointer_cast<IESyntaxTree>(tmp->q[j]);
						#ifdef VERBOSE_MODE
						std::cout<<"[New!!!!]"<<explored->get_complexity()<<" | "<<explored->to_string()<<std::endl;
						#endif
						search_counter++;
						if ((explored != nullptr) && (visited.count(explored) == 0))
						{
							visited.insert(explored);

							auto simplified = rp->filter(explored, examples);
							/* not redundant and not repeating */
							if (simplified == explored)
							{
								candidate.push_back(simplified);
								helper_counter++;
								#ifdef VERBOSE_MODE
								std::cout<<"[New!]"<<simplified->to_string()<<std::endl;;
								#endif
							}
							else if ((simplified != nullptr) && (visited.count(simplified) == 0))
							{
								candidate.push_back(simplified);
								visited.insert(simplified);
								#ifdef VERBOSE_MODE
								std::cout<<"[Simplified!]"<<simplified->get_complexity()<<" | "<<simplified->to_string()<<std::endl;
								#endif
							}
							else if (simplified != nullptr)
							{
								#ifdef VERBOSE_MODE
								std::cout<<"[Repeated!]"<<endl;
								#endif
							}
							else
							{
								#ifdef VERBOSE_MODE
								std::cout<<"[Redundant!]"<<endl;
								#endif
							}
						}
					}

					/* check new program */
					for (int j=0; j<candidate.size(); j++)
					{
						bool flag_acc = true;
						if (!candidate[j]->to_program()->accept(examples))
						{
							#ifdef VERBOSE_MODE
							std::cout<<"[Rejected]"<< candidate[j]->to_string()<<std::endl;
							#endif
							total_drop += 1.0;
							if (candidate[j]->is_complete())
								complete_drop += 1.0;
							flag_acc = false;
						}
						if (flag_acc)
						{
							this_round.push_back(candidate[j]);
							counter++;
							#ifdef VERBOSE_MODE
							std::cout<<"[Accepted]"<< candidate[j]->to_string()<< " [Updated Complexity] "<<candidate[j]->get_complexity()<<std::endl;
							#endif
							if (candidate[j]->is_complete())
							{
								answer_counter ++;
								std::cout<<"ANSWER FOUND: "<<candidate[j]->to_string()<<" | "<<candidate[j]->get_complexity()<<std::endl;
								answer.push_back(candidate[j]);
								this_round.pop_back();
								if (answer_counter == answer_count)
								{
									return answer;
								}
							}
						}
						else
						{
							progress += candidate[j]->weight;
						}
					}
				}
				/* only explore batch_size new nodes each time */
				done = i;
				if (counter >= batch_size)
					break;
			}

			/* gather all candidates in buffer in LRU order */
			for (int k=done+1; k<this_round.size(); k++)
			{
				buffer.push_back(this_round[k]);

				{
					int l = std::experimental::randint(0,(int)buffer.size()-1);
					auto tmp = buffer.back();
					buffer.back() = buffer[l];
					buffer[l] = tmp;
				}
			}
	
			std::sort(buffer.begin(), buffer.end(), compare_syntax_tree_complexity);

			if (buffer.size()>0)
			{
				/*
				for (int i=0; i<buffer.size(); i++)
					cout<<buffer[i]->get_complexity()<<" | ";
				cout<<endl;
				*/
				/* output progress */
				std::cout<<"Progress: "<<progress*100.0<<"%"<<"   |   ";
				std::cout<<"Ending drop rate: "<<(complete_drop/total_drop)*100.0<<"%"<<"   |   ";
				std::cout<<"Buffer size: "<<buffer.size()<<"   |   ";
				std::cout<<"Answers found: "<<answer_counter<<std::endl;
				if (buffer.size()>2)
				{
					int index = std::experimental::randint(0,(int)buffer.size()-1);
					std::cout<<"One current sample: "<<(buffer[index]->to_string())<<" | #"<<buffer[index]->get_complexity()<<std::endl;
				}
				else
					std::cout<<"One current sample: "<<(buffer[0]->to_string())<<" | #"<<buffer[0]->get_complexity()<<std::endl;
				std::cout<<"Programs searched: "<<search_counter<<" | "<<helper_counter<<std::endl;
				#ifdef VERBOSE_MODE
				for (int i=0; i<buffer.size();i++)
					cout<<buffer[i]->get_complexity()<<" | ";
				#endif
				std::cout<<std::endl<<endl;;
			}

			this_round.clear();
			if (buffer.size() <= batch_size/explore_rate)
			{
				while(!buffer.empty())
				{
					this_round.push_back(buffer.back());
					buffer.pop_back();
				}
			}
			else
			{
				for (int k=0; k<batch_size/explore_rate; k++)
				{
					/* randomly skip out of optimal */
					/*
					{
						int l = std::experimental::randint(0,(int)buffer.size()-1);
						double ratio = ((double)l) / ((double)buffer.size());
						ratio = pow(ratio, 100);
						l = buffer.size() * (1.0 - ratio);
						std::cout<<"[RND]"<<buffer.size()-l<<std::endl;;
						if (l>=buffer.size())
							l = buffer.size()-1;
						auto tmp = buffer.back();
						buffer.back() = buffer[l];
						buffer[l] = tmp;
					}
					*/
					this_round.push_back(buffer.back());
					buffer.pop_back();
				}
			}
		}
	}

	return answer;

}

