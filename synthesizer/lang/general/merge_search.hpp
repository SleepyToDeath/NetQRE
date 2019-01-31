#include "general.hpp"
#include "../../core/search_graph.h"
#include "../netqre/interface.hpp"

using std::shared_ptr;

class MergeSearch
{
	public:
	std::vector<shared_ptr<GeneralSyntaxTree> > search(
				int depth_threshold, 
				int batch_size, 
				int explore_rate, 
				int answer_count, 
				int threads,
				int minimal_example_size,
				shared_ptr<IESyntaxLeftHandSide> starting_symbol, 
				shared_ptr<RedundancyPlan> rp,
				shared_ptr<NetqreExample> e)
	{
		this->depth_threshold = depth_threshold;
		this->batch_size = batch_size;
		this->explore_rate = explore_rate;
		this->answer_count = answer_count;
		this->threads = threads;
		this->minimal_example_size = minimal_example_size;
		this->starting_symbol = starting_symbol;
		this->rp = rp;
		vector<shared_ptr<GeneralSyntaxTree> > global_pool;
		return real_search(e, global_pool);
	}

	private:
	
	int depth_threshold;
	int batch_size;
	int explore_rate;
	int answer_count;
	int threads;
	int minimal_example_size;
	shared_ptr<IESyntaxLeftHandSide> starting_symbol;
	shared_ptr<RedundancyPlan> rp;


	std::vector<shared_ptr<GeneralSyntaxTree> > real_search(shared_ptr<NetqreExample> e, vector<shared_ptr<GeneralSyntaxTree> >& global_pool)
	{
		if (e->positive_token.size() <= minimal_example_size && e->negative_token.size() <= minimal_example_size)
		{
			/* base case */
			cout<<"Searching! Size: "<<e->positive_token.size()<<" "<<e->negative_token.size()<<endl;

			vector<shared_ptr<GeneralSyntaxTree> > ans;

			for (int i=0; i<global_pool.size(); i++)
				if (global_pool[i]->to_program()->accept(e))
					ans.push_back(static_pointer_cast<GeneralSyntaxTree>(SyntaxTree::factory->get_new(global_pool[i])));

			if (ans.size() < answer_count)
			{
				SearchGraph graph(depth_threshold, batch_size, explore_rate, answer_count, threads, starting_symbol, rp);
				vector<shared_ptr<IESyntaxTree> > seed;
				std::unordered_set<shared_ptr<SyntaxTree>, HashSyntaxTree, CmpSyntaxTree > eliminate;
				auto ans_tmp = graph.search_top_level_v2(e, seed, eliminate);
				for (int i=0; i<ans_tmp.size(); i++)
				{
					bool exist = false;
					for (int j=0; j<ans.size(); j++)
					{
						if (ans[j]->equal(ans_tmp[i]))
						{
							exist = true;
							break;
						}
					}
					if (!exist)
					{
						ans.push_back(static_pointer_cast<GeneralSyntaxTree>(ans_tmp[i]));
						global_pool.push_back(static_pointer_cast<GeneralSyntaxTree>(SyntaxTree::factory->get_new(ans.back())));
					}
				}
			}
			cout<<"Done! Size: "<<e->positive_token.size()<<" "<<e->negative_token.size()<<endl;
			return ans;
		}
		else
		{
			/* divide */
			auto e_left = shared_ptr<NetqreExample>(new NetqreExample());
			auto e_right = shared_ptr<NetqreExample>(new NetqreExample());

			if (e->positive_token.size() <= minimal_example_size)
			{
				e_left->positive_token = e->positive_token;
				e_right->positive_token = e->positive_token;
			}
			else
			{
				for (int i=0; i< e->positive_token.size()/2; i++)
					e_left->positive_token.push_back(e->positive_token[i]);
				for (int i=e->positive_token.size()/2; i< e->positive_token.size(); i++)
					e_right->positive_token.push_back(e->positive_token[i]);
			}

			if (e->negative_token.size() <= minimal_example_size)
			{
				e_left->negative_token = e->negative_token;
				e_right->negative_token = e->negative_token;
			}
			else
			{
				for (int i=0; i< e->negative_token.size()/2; i++)
					e_left->negative_token.push_back(e->negative_token[i]);
				for (int i=e->negative_token.size()/2; i< e->negative_token.size(); i++)
					e_right->negative_token.push_back(e->negative_token[i]);
			}

			auto ans_left = real_search(e_left, global_pool);
			auto ans_right = real_search(e_right, global_pool);

			/* conquer */
			std::vector<shared_ptr<GeneralSyntaxTree> > ans;
			auto ans_wrong = ans;
			auto ans_raw = ans_left;
			for (int i=0; i<ans_right.size(); i++)
			{
				bool exist = false;
				for (int j=0; j<ans_left.size(); j++)
					if (ans_left[j]->equal(ans_right[i]))
					{
						exist = true;
						break;
					}
				if (!exist)
					ans_raw.push_back(ans_right[i]);
			}

			for (int i=0; i<ans_raw.size(); i++)
				if (ans_raw[i]->to_program()->accept(e))
					ans.push_back(ans_raw[i]);
				else
					ans_wrong.push_back(ans_raw[i]);

			cout<<"Searching! Size: "<<e->positive_token.size()<<" "<<e->negative_token.size()<<endl;

			if (ans.size() < answer_count)
			{
				vector<shared_ptr<IESyntaxTree> > seed;
				std::unordered_set<shared_ptr<SyntaxTree>, HashSyntaxTree, CmpSyntaxTree > eliminate;
				for (int i=0; i<ans_wrong.size(); i++)
				{
					eliminate.insert(static_pointer_cast<GeneralSyntaxTree>(SyntaxTree::factory->get_new(ans_wrong[i])));
					seed.push_back(ans_wrong[i]);
					ans_wrong[i]->prune();
				}

				SearchGraph graph(depth_threshold, batch_size, explore_rate, answer_count - ans.size(), threads, starting_symbol, rp);
				auto ans_fixed = graph.search_top_level_v2(e, seed, eliminate);
				for (int i=0; i<ans_fixed.size(); i++)
					ans.push_back(static_pointer_cast<GeneralSyntaxTree>(ans_fixed[i]));
			}

			for (int i=0; i<ans.size(); i++)
			{
				bool exist = false;
				for (int j=0; j<global_pool.size(); j++)
					if (ans[i]->equal(global_pool[j]))
					{
						exist = true;
						break;
					}
				if (!exist)
				{
					global_pool.push_back(static_pointer_cast<GeneralSyntaxTree>(SyntaxTree::factory->get_new(ans[i])));
				}
			}


			cout<<"Done! Size: "<<e->positive_token.size()<<" "<<e->negative_token.size()<<endl;

			return ans;
		}
	}
};

