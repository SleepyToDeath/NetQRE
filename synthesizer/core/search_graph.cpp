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
using std::cerr;
using std::min;
using std::unordered_set;

int total_programs_searched = 0;

SearchGraph::SearchGraph(
				int answer_count, 
				shared_ptr<IESyntaxLeftHandSide> starting_symbol, 
				shared_ptr<RedundancyPlan> rp ) 
{
	this->batch_size = require_(int, "batch_size");
	this->search_depth = require_(int, "search_depth");
	this->explore_rate = require_(int, "explore_rate");
	this->accuracy = require_(double, "accuracy");
	this->threads = require_(int, "threads");
	this->give_up_count = require_(int, "give_up_count");
	this->VERBOSE_MODE = require_(int, "VERBOSE_MODE");
	this->starting_symbol = starting_symbol;
	this->answer_count = answer_count;
	this->rp = rp;
}

vector<shared_ptr<IESyntaxTree> > SearchGraph::search_top_level_v2(
	shared_ptr<IEExample> examples, 
	vector<shared_ptr<IESyntaxTree> > seed = vector<shared_ptr<IESyntaxTree> >(),
	VisitPool eliminate = VisitPool() )
{
	return enumerate_random_v2(examples, seed, eliminate);
}

vector< shared_ptr<IESyntaxTree> > SearchGraph::enumerate_random_v2(
	shared_ptr<IEExample> examples, 
	vector<shared_ptr<IESyntaxTree> > seed,
	VisitPool eliminate)
{
	vector<shared_ptr<IESyntaxTree> > this_round = seed;
	vector<shared_ptr<IESyntaxTree> > buffer;
	vector<shared_ptr<IESyntaxTree> > answer;
	vector<shared_ptr<IESyntaxTree> > pending_answer;
	auto visited = shared_ptr<VisitPool>(new VisitPool(eliminate));
	rp->visited = visited;

	cerr<<visited->size()<<" eliminated answers at the beginning\n";

	MeansOfProduction mop;
	mop.rp = rp;
	auto thread_master = shared_ptr<MasterThread>(new MasterThread(threads, mop));
	thread_master->hire_workers();

	int search_counter = 0;
	int helper_counter = 0;
	double progress = 0;
	double total_drop = 0;
	double complete_drop = 0;

	int depth = search_depth;

	try
	{
//		if (seed.empty())
		{
			shared_ptr<IESyntaxTree> s = std::static_pointer_cast<IESyntaxTree>(
				SyntaxTree::factory->get_new(
					shared_ptr<SyntaxTreeNode>(new SyntaxTreeNode(starting_symbol))));
			s->weight = 1;
			this_round.push_back(s);
			visited->insert(s);
		}

		while (this_round.size()>0)
		{
			/* prepare this round */
			vector<shared_ptr<IESyntaxTree> > candidate;
			auto tmp = shared_ptr<SyntaxTree::Queue>(new SyntaxTree::Queue());
			int counter = 0;
			int done = -1;
			int i=0;
			Mailbox msg;
			
			/* ==================== Definition ===================== */
			auto explore_new = [&]() {
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
							if (explored != nullptr)
							{
								/* send redundancy filtering tasks to workers */
								thread_master->do_filter(explored, examples);
							}
						}
					}

					/* move forward */
					done = i;
					i++;
				}
			};

			auto check_redundancy = [&]() {
				if (msg.type == FILTER)
				{
					auto simplified = msg.simplified;
					auto explored = msg.candidate;
					if (VERBOSE_MODE)
						std::cerr<<"[New!!!!]"<<explored->get_complexity()<<
										" | "<<explored->to_string()<<std::endl;

					if (simplified != nullptr)
					{
						if (VERBOSE_MODE)
							std::cerr<<"[New!]"<<simplified->to_string()<<std::endl;;
						visited->insert(explored);
					}
					/* not redundant and not repeating */
					if (simplified == explored)
					{
						/* send accept checking tasks to workers */
						thread_master->do_accept(simplified, examples, {accuracy});
						helper_counter++;
						counter++;
					}
					/* redundant but not repeating */
					else if ((simplified != nullptr) && (visited->count(simplified) == 0))
					{
						/* send accept checking tasks to workers */
						thread_master->do_accept(simplified, examples, {accuracy});
						visited->insert(simplified);
					}
				}
			};

			auto check_accept = [&]() {
				/* if it is oracle checking task */
				if (msg.type == ACCEPT)
				{
					auto candidate = msg.candidate;
					/* if rejected */
					if (!msg.accept)
					{
						/* drop */
						if (VERBOSE_MODE)
						std::cerr<<"[Rejected]"<<candidate->to_string()
										<<" | "<<candidate->get_complexity()<<std::endl;
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
							pending_answer.push_back(candidate);
							this_round.pop_back();
						}
					}
				}
			};

			auto finish_tasks = [&]() {
				/* exhaust all finished tasks so far */
				msg = thread_master->find_finished_task();
				while (msg.finished_task)
				{
					check_redundancy();
					check_accept();

					/* try to find next finished task */
					msg = thread_master->find_finished_task();
				}
			};

			auto run_this_round = [&]() {
				while (true)
				{
					explore_new();
					finish_tasks();

					/* if enough programs explored and all tasks submitted  */
					if ((counter >= batch_size || i == this_round.size()) 
										&& thread_master->all_tasks_done())
					break;
				}
			};




			auto gather_candidates = [&]() {
				/* gather all candidates */
				for (int k=done+1; k<this_round.size(); k++)
				{
					buffer.push_back(this_round[k]);

/*
					{
						int l = std::experimental::randint(0,(int)buffer.size()-1);
						auto tmp = buffer.back();
						buffer.back() = buffer[l];
						buffer[l] = tmp;
					}
					*/
				}

				buffer += this_round.slice(done, this_round.size() - done);
		
				/* sort by complexity */
				buffer.select_( [](const shared_ptr<IESyntaxTree>& e)->bool {
					return e->get_complexity() < 10000;
				}).sort_by_<double>( [](const shared_ptr<IESyntaxTree>& e)->double {
					return (- e->get_complexity());
				});

				errputs( _S_(buffer[0]->get_complexity()) + " ~ " + _S_(buffer[-1]->get_complexity()) );

				pending_answer.select_( [&] (const shared_ptr<IESyntaxTree>& papapa)->bool {
//					double threshold = (buffer.size() > 0) ? (buffer[buffer.size()/2]->get_complexity()) : 0;
					double threshold = 10000;
					if (papapa->get_complexity() <= threshold)
					{
						std::cerr<<"ANSWER FOUND: "<<papapa->to_string()
										<<" | "<<papapa->get_complexity()<<std::endl;
						answer.push_back(papapa);
						if (answer.size() >= answer_count)
						{
							while(!thread_master->all_tasks_done())
								thread_master->find_finished_task();
							total_programs_searched += search_counter;
							throw answer;
						}
						return false;
					}
					else
						return true;
				});
			};




			auto print_progress = [&]() {
				/* print progress */
				#ifndef SILENCE_MODE
				Rubify::do_at_interval(0, 1, [&] () {
					if (buffer.size()>0)
					{
	//					std::cerr<<"Progress: "<<progress*100.0<<"%"<<"   |   ";
						std::cerr<<"Early prune rate: "<<((total_drop-complete_drop)/total_drop)*100.0<<"%"<<"   |   ";
						std::cerr<<"Buffer size: "<<buffer.size()<<"   |   ";
						std::cerr<<"Answers found: "<<answer.size()<<std::endl;
						if (buffer.size()>2)
						{
							int index = std::experimental::randint(0,(int)buffer.size()/2);
							std::cerr<<"One current sample: "<<(buffer[index]->to_string())
											<<" | #"<<buffer[index]->get_complexity()<<std::endl;
						}
						else
							std::cerr<<"One current sample: "<<(buffer[0]->to_string())
											<<" | #"<<buffer[0]->get_complexity()<<std::endl;
						std::cerr<<"Programs searched: "<<search_counter<<" | "<<visited->size()<<" | "<<helper_counter<<" | "<<total_programs_searched<<std::endl;
						std::cerr<<std::endl<<endl;;
					}
				} );
				#endif
			};




			auto prepare_next_round = [&]() {
				/* prepare next round */
				if (search_counter > give_up_count && answer.empty())
					throw pending_answer;
				if (buffer.size() > give_up_count)
					throw (answer + pending_answer);

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
							std::cerr<<"[RND]"<<buffer.size()-l<<std::endl;;
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
			};
			/* ==================== End of Definition ================= */



			/* ====================== Run ========================= */
			run_this_round();
			gather_candidates();
			print_progress();
			prepare_next_round();
			/* ================== End of Run ====================== */
		}
	}
	catch(vector<shared_ptr<IESyntaxTree> > ans)
	{
		total_programs_searched += search_counter;
		return ans;
	}

	total_programs_searched += search_counter;
	return answer;
}
















// ================================== For Testing ===========================================

void SearchGraph::enum_pure(
	vector<shared_ptr<IESyntaxTree> > seed,
	VisitPool eliminate)
{
	vector<shared_ptr<IESyntaxTree> > this_round = seed;
	int i=0;
	while (i < this_round.size())
	{
		auto tmp = std::make_shared<SyntaxTree::Queue>();
		/* prepare exploration */
		tmp->q.clear();
		auto current = this_round[i];

		/* explore new programs */
		shared_ptr<SyntaxTree> place_holder = current;
		if (current->multi_mutate(place_holder, current, 99999, tmp))
		{
			for (int j=0; j<tmp->q.size(); j++)
			{
				auto explored = std::static_pointer_cast<IESyntaxTree>(tmp->q[j]);
				if (explored != nullptr)
				{
					this_round.push_back(explored);
					std::cout<<explored->to_string()<<std::endl;
					std::cout<<explored->marshall()<<std::endl<<std::endl;
				}
			}
		}
		i++;
	}
}
