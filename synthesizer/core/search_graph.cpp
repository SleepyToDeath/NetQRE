#include "search_graph.h"
#include <experimental/random>
#include <algorithm>

SearchGraph::SearchGraph(int depth_threshold, int batch_size, int answer_count, shared_ptr<IESyntaxLeftHandSide> starting_symbol) {
	this->batch_size = batch_size;
	this->answer_count = answer_count;
	this->depth_threshold = depth_threshold;
	this->starting_symbol = starting_symbol;
}

std::vector<shared_ptr<IESyntaxTree> > SearchGraph::search_top_level_v2(shared_ptr<IEExample> examples) {
	return enumerate_random_v2(examples);
}

std::vector< shared_ptr<IESyntaxTree> > SearchGraph::enumerate_random_v2(shared_ptr<IEExample> examples) {
	std::vector<shared_ptr<IESyntaxTree> > this_round;
	std::vector<shared_ptr<IESyntaxTree> > buffer;
	std::vector<shared_ptr<IESyntaxTree> > answer;
	int answer_counter = 0;

	double progress = 0;

	double total_drop = 0;
	double complete_drop = 0;
//	for (int depth = 0; depth<depth_threshold; depth++)
	int depth = depth_threshold;
	{
//		std::cout<<"Depth:"<<depth<<std::endl;
		auto s = shared_ptr<IESyntaxTree> (new IESyntaxTree(shared_ptr<SyntaxTreeNode>(new SyntaxTreeNode(starting_symbol))));
		s->weight = 1;
		this_round.push_back(s);
		while (this_round.size()>0)
		{
			std::vector<shared_ptr<IESyntaxTree> > candidate;
			shared_ptr<SyntaxTree::Queue> tmp;
			int counter = 0;
			int done = -1;
			bool flag_deadend = false;
			for (int i=0; i<this_round.size(); i++)
			{
				candidate.clear();
				tmp->q.clear();
				shared_ptr<IESyntaxTree> current = this_round[i];
				if (current->multi_mutate(current, depth, tmp))
				{
					for (int j=0; j<tmp->q.size(); j++)
					{
						candidate.push_back(shared_ptr<IESyntaxTree>(new IESyntaxTree(tmp->q[j])));
//						std::cout<<tmp[j]->weight<<" "<<candidate[j]->weight<<std::endl;;
					}
					for (int j=0; j<candidate.size(); j++)
					{
						bool flag_acc = true;
						std::cout<<candidate[j]->to_string()<<std::endl;
						if (!candidate[j]->to_program()->accept(examples))
						{
							total_drop += 1.0;
							if (candidate[j]->is_complete())
								complete_drop += 1.0;
							flag_acc = false;
							break;
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
			std::vector<shared_ptr<IESyntaxTree> > buffer2;
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
						auto tmp = buffer.back();
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
