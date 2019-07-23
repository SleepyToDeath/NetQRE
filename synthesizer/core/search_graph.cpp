#include "search_graph.h"
#include "multithread.h"
#include <experimental/random>
#include <algorithm>
#include <cmath>
#include <unordered_set>
//#define VERBOSE_MODE
//#define SILENCE_MODE
#define USE_MULTITHREAD

using std::endl;
using std::cout;
using std::unordered_set;

int total_programs_searched = 0;

SearchGraph::SearchGraph(int depth_threshold, 
				int batch_size, 
				int explore_rate,
				int answer_count, 
				int threads,
				shared_ptr<IESyntaxLeftHandSide> starting_symbol, 
				shared_ptr<RedundancyPlan> rp ) 
{
	this->batch_size = batch_size;
	this->answer_count = answer_count;
	this->depth_threshold = depth_threshold;
	this->starting_symbol = starting_symbol;
	this->explore_rate = explore_rate;
	this->threads = threads;
	this->rp = rp;
}

vector<shared_ptr<IESyntaxTree> > SearchGraph::search_top_level_v2(
	shared_ptr<IEExample> examples, 
	vector<shared_ptr<IESyntaxTree> > seed = vector<shared_ptr<IESyntaxTree> >(),
	unordered_set<shared_ptr<SyntaxTree>, HashSyntaxTree, CmpSyntaxTree > eliminate 
	= unordered_set<shared_ptr<SyntaxTree>, HashSyntaxTree, CmpSyntaxTree >() )
{
	return enumerate_random_v2(examples, seed, eliminate);
}

vector< shared_ptr<IESyntaxTree> > SearchGraph::enumerate_random_v2(
	shared_ptr<IEExample> examples, 
	vector<shared_ptr<IESyntaxTree> > seed,
	std::unordered_set<shared_ptr<SyntaxTree>, HashSyntaxTree, CmpSyntaxTree > eliminate)
{
	vector<shared_ptr<IESyntaxTree> > this_round = seed;
	vector<shared_ptr<IESyntaxTree> > buffer;
	vector<shared_ptr<IESyntaxTree> > answer;
	std::unordered_set<shared_ptr<SyntaxTree>, HashSyntaxTree, CmpSyntaxTree > visited = eliminate;

	MeansOfProduction mop;
	mop.rp = rp;
	auto thread_master = shared_ptr<MasterThread>(new MasterThread(threads, mop));
	thread_master->hire_workers();

	int answer_counter = 0;
	int search_counter = 0;
	int helper_counter = 0;
	double progress = 0;
	double total_drop = 0;
	double complete_drop = 0;

	int depth = depth_threshold;
	{
		if (seed.empty())
		{
			shared_ptr<IESyntaxTree> s = std::static_pointer_cast<IESyntaxTree>(
				SyntaxTree::factory->get_new(
					shared_ptr<SyntaxTreeNode>(new SyntaxTreeNode(starting_symbol))));
			s->weight = 1;
			this_round.push_back(s);
			visited.insert(s);
		}

		while (this_round.size()>0)
		{
			/* prepare this round */
			vector<shared_ptr<IESyntaxTree> > candidate;
			auto tmp = shared_ptr<SyntaxTree::Queue>(new SyntaxTree::Queue());
			int counter = 0;
			int done = -1;

#ifndef USE_MULTITHREAD
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
				shared_ptr<SyntaxTree> place_holder = current;
				if (current->multi_mutate(place_holder, current, depth, tmp))
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
									total_programs_searched += search_counter;
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
#else
			/* start this round -- multithread version */
			int i=0;
			while (true)
			{
				/* check if already explored enough programs in this round */
				/* if not, explore new programs */
				if (counter < batch_size && i < this_round.size())
				{
					/* prepare exploration */
					tmp->q.clear();
					shared_ptr<IESyntaxTree> current = this_round[i];

					/* explore new programs */
					shared_ptr<SyntaxTree> place_holder = current;
					if (current->multi_mutate(place_holder, current, depth, tmp))
					{
						for (int j=0; j<tmp->q.size(); j++)
						{
							auto explored = std::static_pointer_cast<IESyntaxTree>(tmp->q[j]);
							search_counter++;
							if ((explored != nullptr) && (visited.count(explored) == 0))
							{
								visited.insert(explored);
								/* send redundancy filtering tasks to workers */
								thread_master->do_filter(explored, examples);
							}
						}
					}

					/* move forward */
					done = i;
					i++;
				}

				/* exhaust all finished tasks so far */
				Mailbox msg = thread_master->find_finished_task();
				while (msg.finished_task)
				{
					/* if it is redundancy filter task */
					if (msg.type == FILTER)
					{
						auto simplified = msg.simplified;
						auto explored = msg.candidate;
						#ifdef VERBOSE_MODE
						std::cout<<"[New!!!!]"<<explored->get_complexity()<<" | "<<explored->to_string()<<std::endl;
						#endif
						#ifdef VERBOSE_MODE
						if (simplified != nullptr)
							std::cout<<"[New!]"<<simplified->to_string()<<std::endl;;
						#endif
						/* not redundant and not repeating */
						if (simplified == explored)
						{
							/* send accept checking tasks to workers */
							thread_master->do_accept(simplified, examples);
							helper_counter++;
							counter++;
						}
						/* redundant but not repeating */
						else if ((simplified != nullptr) && (visited.count(simplified) == 0))
						{
							/* send accept checking tasks to workers */
							thread_master->do_accept(simplified, examples);
							visited.insert(simplified);
						}
					}

					/* if it is oracle checking task */
					if (msg.type == ACCEPT)
					{
						auto candidate = msg.candidate;
						/* if rejected */
						if (!msg.accept)
						{
							/* drop */
							#ifdef VERBOSE_MODE
							std::cout<<"[Rejected]"<<candidate->to_string()<<" | "<<candidate->get_complexity()<<std::endl;
							#endif
							total_drop += 1.0;
							if (candidate->is_complete())
								complete_drop += 1.0;
							progress += candidate->weight;
						}
						/* if accepted */
						else
						{
							/* append to this_round */
							this_round.push_back(candidate);

							/* if answer found */
							if (candidate->is_complete())
							{
								answer_counter ++;
								std::cout<<"ANSWER FOUND: "<<candidate->to_string()<<" | "<<candidate->get_complexity()<<std::endl;
								answer.push_back(candidate);
								this_round.pop_back();
								if (answer_counter >= answer_count)
								{
									while(!thread_master->all_tasks_done())
										thread_master->find_finished_task();
									total_programs_searched += search_counter;
									return answer;
								}
							}
						}
					}

					/* try to find next finished task */
					msg = thread_master->find_finished_task();
				}

				/* if enough programs explored and all tasks submitted  */
				if ((counter >= batch_size || i == this_round.size()) && thread_master->all_tasks_done())
					break;
//				cout<<"Loop going on "<<counter<<" "<<batch_size<<endl;
			}
#endif

			/* gather all candidates */
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
	
			/* sort by complexity */
			std::sort(buffer.begin(), buffer.end(), compare_syntax_tree_complexity);

			/* print progress */
#ifndef SILENCE_MODE
			if (buffer.size()>0)
			{
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
				std::cout<<std::endl<<endl;;
			}
#endif

			/* prepare next round */
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
					/* randomly jump out of optimal */
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

	total_programs_searched += search_counter;
	return answer;

}

