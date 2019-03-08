#include "general.hpp"
#include "../../core/search_graph.h"
#include "../netqre/interface.hpp"
#include <experimental/random>

using std::shared_ptr;

extern int total_programs_searched;

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
		this->top_example = e;
		e->pos_offset = 0;
		e->neg_offset = 0;
		vector<shared_ptr<GeneralSyntaxTree> > global_pool;
		collect_tree(e);
		return search_by_layer(global_pool);
	}

	static int force_search_factor;

	private:
	
	int depth_threshold;
	int batch_size;
	int explore_rate;
	int answer_count;
	int threads;
	int minimal_example_size;
	shared_ptr<NetqreExample> top_example;
	shared_ptr<IESyntaxLeftHandSide> starting_symbol;
	shared_ptr<RedundancyPlan> rp;
	vector< vector<shared_ptr<NetqreExample> > > e_tree; //example tree

	int collect_tree(shared_ptr<NetqreExample> e)
	{
		int depth;
		if (e->positive_token.size() <= minimal_example_size && e->negative_token.size() <= minimal_example_size)
			depth = 1;
		else
		{
			auto e_left = shared_ptr<NetqreExample>(new NetqreExample());
			auto e_right = shared_ptr<NetqreExample>(new NetqreExample());

			if (e->positive_token.size() <= minimal_example_size)
			{
				e_left->pos_offset = e->pos_offset;
				e_left->positive_token = e->positive_token;
				e_right->pos_offset = e->pos_offset;
				e_right->positive_token = e->positive_token;
			}
			else
			{
				e_left->pos_offset = e->pos_offset;
				for (int i=0; i< e->positive_token.size()/2; i++)
					e_left->positive_token.push_back(e->positive_token[i]);
				e_right->pos_offset = e->pos_offset + e_left->positive_token.size();
				for (int i=e->positive_token.size()/2; i< e->positive_token.size(); i++)
					e_right->positive_token.push_back(e->positive_token[i]);
			}

			if (e->negative_token.size() <= minimal_example_size)
			{
				e_left->neg_offset = e->neg_offset;
				e_left->negative_token = e->negative_token;
				e_left->neg_offset = e->neg_offset;
				e_right->negative_token = e->negative_token;
			}
			else
			{
				e_left->neg_offset = e->neg_offset;
				for (int i=0; i< e->negative_token.size()/2; i++)
					e_left->negative_token.push_back(e->negative_token[i]);
				e_right->neg_offset = e->neg_offset + e_left->negative_token.size();
				for (int i=e->negative_token.size()/2; i< e->negative_token.size(); i++)
					e_right->negative_token.push_back(e->negative_token[i]);
			}

			int tmp_l = collect_tree(e_left);
			int tmp_r = collect_tree(e_right);
			depth = tmp_l>tmp_r?tmp_l:tmp_r;
			depth ++;
		}

		if (depth > e_tree.size())
			e_tree.push_back(vector<shared_ptr<NetqreExample> >());

		e_tree[depth-1].push_back(e);

		return depth;
	}

	std::vector<shared_ptr<GeneralSyntaxTree> > search_by_layer(vector<shared_ptr<GeneralSyntaxTree> >& global_pool)
	{
		vector<shared_ptr<GeneralSyntaxTree> > ans;
		try
		{
			for (int i=0; i<e_tree.size(); i++)
			{
				cout<<"Size of global pool: "<<global_pool.size()<<endl;
				for (int j=0; j<e_tree[i].size(); j++)
				{	
					int local_answer_count = answer_count * (e_tree.size()-i) / e_tree.size();
					if (local_answer_count <= 0)
						local_answer_count = 1;
					real_search_single(e_tree[i][j], local_answer_count, global_pool);
				}

				cout<<"Size of global pool: "<<global_pool.size()<<endl;
				vector<shared_ptr<GeneralSyntaxTree> > tmp;
				for (int k=0; k<global_pool.size(); k++)
				{
					int acc_count =0;
					for (int j=0; j<e_tree[i].size(); j++)
						if (global_pool[k]->to_program()->accept(e_tree[i][j]))
							acc_count ++;
					if (acc_count >= e_tree[i].size() / 2)
						tmp.push_back(global_pool[k]);
				}
				global_pool = tmp;
				cout<<"Size of global pool: "<<global_pool.size()<<endl;
			}
		}
		catch (std::vector<shared_ptr<GeneralSyntaxTree> > _ans)
		{
			ans = _ans;
		}
		return ans;
	}

	std::vector<shared_ptr<GeneralSyntaxTree> > real_search_single(shared_ptr<NetqreExample> e, int local_answer_count, vector<shared_ptr<GeneralSyntaxTree> >& global_pool)
	{
		cout<<"Searching! Size: "<<e->positive_token.size()<<" "<<e->negative_token.size()<<endl;

		vector<shared_ptr<GeneralSyntaxTree> > ans;
		vector<shared_ptr<GeneralSyntaxTree> > ans_wrong;

		for (int i=0; i<global_pool.size(); i++)
			if (global_pool[i]->to_program()->accept(e))
				ans.push_back(static_pointer_cast<GeneralSyntaxTree>(SyntaxTree::factory->get_new(global_pool[i])));
			else
				ans_wrong.push_back(static_pointer_cast<GeneralSyntaxTree>(SyntaxTree::factory->get_new(global_pool[i])));
	
		if (ans.size() >= local_answer_count)
			if (std::experimental::randint(0,local_answer_count*force_search_factor) == local_answer_count/2)
				local_answer_count = ans.size() + 1;

		if (ans.size() < local_answer_count)
		{
			vector<shared_ptr<IESyntaxTree> > seed;
			std::unordered_set<shared_ptr<SyntaxTree>, HashSyntaxTree, CmpSyntaxTree > eliminate;
			for (int i=0; i<ans.size(); i++)
				eliminate.insert(static_pointer_cast<GeneralSyntaxTree>(SyntaxTree::factory->get_new(ans[i])));
			for (int i=0; i<ans_wrong.size(); i++)
			{
				eliminate.insert(static_pointer_cast<GeneralSyntaxTree>(SyntaxTree::factory->get_new(ans_wrong[i])));
				for (int j=3; j<=7; j++)
				{
					seed.push_back(static_pointer_cast<GeneralSyntaxTree>(SyntaxTree::factory->get_new(ans_wrong[i])));
					static_pointer_cast<GeneralSyntaxTree>(seed.back())->prune(j);
				}
			}

			SearchGraph graph(depth_threshold, batch_size, explore_rate, local_answer_count-ans.size(), threads, starting_symbol, rp);
			auto ans_tmp = graph.search_top_level_v2(e, seed, eliminate);
			cout<<"Search done!!!\n";
			cout<<"Size of global pool: "<<global_pool.size()<<endl;
			for (int i=0; i<ans_tmp.size(); i++)
			{
				cout<<i<<endl;
				bool exist = false;
				for (int j=0; j<ans.size(); j++)
				{
					cout<<j<<endl;
					if (ans[j]->equal(ans_tmp[i]))
					{
						exist = true;
						break;
					}
				}
				cout<<"#2"<<endl;
				if (!exist)
				{
					ans.push_back(static_pointer_cast<GeneralSyntaxTree>(ans_tmp[i]));
					global_pool.push_back(static_pointer_cast<GeneralSyntaxTree>(SyntaxTree::factory->get_new(ans.back())));
					cout<<"#2.5"<<endl;
					if (global_pool.back()->to_program()->accept(top_example))
					{
						cout<<"Answer found! Current task size: "<<e->positive_token.size()<<" "<<e->negative_token.size()<<endl;
						cout<<"Size of global pool: "<<global_pool.size()<<endl;
						cout<<"Total programs searched: "<<total_programs_searched<<endl;
						cout<<global_pool.back()->to_string()<<endl;;
						throw vector<shared_ptr<GeneralSyntaxTree> >(1, global_pool.back());
					}
				}
				cout<<"#3"<<endl;
			}
		}
		cout<<"Done! Size: "<<e->positive_token.size()<<" "<<e->negative_token.size()<<endl;
		return ans;
	}













/* ======================================================= Old Version Below ====================================================== */

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
				cout<<"Search done!!!\n";
				for (int i=0; i<ans_tmp.size(); i++)
				{
					cout<<i<<endl;
					bool exist = false;
					for (int j=0; j<ans.size(); j++)
					{
						cout<<j<<endl;
						if (ans[j]->equal(ans_tmp[i]))
						{
							exist = true;
							break;
						}
					}
					cout<<"#2"<<endl;
					if (!exist)
					{
						ans.push_back(static_pointer_cast<GeneralSyntaxTree>(ans_tmp[i]));
						global_pool.push_back(static_pointer_cast<GeneralSyntaxTree>(SyntaxTree::factory->get_new(ans.back())));
						cout<<"#2.5"<<endl;
						if (global_pool.back()->to_program()->accept(top_example))
						{
							cout<<"Answer found! Current task size: "<<e->positive_token.size()<<" "<<e->negative_token.size()<<endl;
							cout<<"Size of global pool: "<<global_pool.size()<<endl;
							cout<<"Total programs searched: "<<total_programs_searched<<endl;
							throw string("Answer Found!\n");
						}
					}
					cout<<"#3"<<endl;
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
					ans_wrong[i]->prune(5);
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
					if (global_pool.back()->to_program()->accept(top_example))
					{
						cout<<"Answer found! Current task size: "<<e->positive_token.size()<<" "<<e->negative_token.size()<<endl;
						cout<<"Size of global pool: "<<global_pool.size()<<endl;
						cout<<"Total programs searched: "<<total_programs_searched<<endl;
						throw string("Answer Found!\n");
					}
				}
			}


			cout<<"Done! Size: "<<e->positive_token.size()<<" "<<e->negative_token.size()<<endl;
			cout<<"Size of global pool: "<<global_pool.size()<<endl;
			cout<<"Total programs searched: "<<total_programs_searched<<endl;

			return ans;
		}
	}
};

