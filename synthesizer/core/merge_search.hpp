#include "search_graph.h"
#include "../../general-lang/general.hpp"
#include <experimental/random>
#include <algorithm>

using std::shared_ptr;
using std::cerr;
using std::endl;

extern int total_programs_searched;

class MergeSearch {
	public:
	vector<shared_ptr<GeneralSyntaxTree> > search(
				int depth_threshold, 
				int batch_size, 
				int explore_rate, 
				int answer_count, 
				double accuracy,
				int threads,
				int minimal_example_size,
				int force_search_factor,
				shared_ptr<IESyntaxLeftHandSide> starting_symbol, 
				shared_ptr<RedundancyPlan> rp,
				shared_ptr<GeneralExampleHandle> e)
	{
		this->depth_threshold = depth_threshold;
		this->batch_size = batch_size;
		this->explore_rate = explore_rate;
		this->answer_count = answer_count;
		this->accuracy = accuracy;
		this->threads = threads;
		this->minimal_example_size = minimal_example_size;
		this->starting_symbol = starting_symbol;
		this->rp = rp;
		this->top_example = e;

		if (accuracy < 1.0)
		{
			global_constraint = shared_ptr<GeneralSolutionGroupConstraint>(new GeneralSolutionGroupConstraint());
			global_constraint->pos_weight = vector<double>(e->positive_token.size(), 1.0);
			global_constraint->neg_weight = vector<double>(e->negative_token.size(), 1.0);
			global_constraint->updater = [&](double w)->double {
				if (w == 1.0)
					return (accuracy + 1.0) / 2.0;
				else
					return w;
			};
		}
		else
		{
			global_constraint = nullptr;
		}


		std::cerr<<"Start searching! Example size:" +_S_(e->positive_token.size()) + " " +  _S_(e->negative_token.size())<<endl;
		vector<shared_ptr<GeneralSyntaxTree> > global_pool;
		collect_tree(e);
		return search_by_layer(global_pool);
	}


	private:
	
	int depth_threshold;
	int batch_size;
	int explore_rate;
	int answer_count;
	double accuracy;
	int threads;
	int minimal_example_size;
	int force_search_factor;
	shared_ptr<GeneralExampleHandle> top_example;
	shared_ptr<IESyntaxLeftHandSide> starting_symbol;
	shared_ptr<RedundancyPlan> rp;
	vector< vector<shared_ptr<GeneralExampleHandle> > > e_tree; //example tree
	vector<shared_ptr<GeneralSyntaxTree> > global_answer;
	shared_ptr<GeneralSolutionGroupConstraint> global_constraint;

/*
	void merge_tree(int layer)
	{
		e_tree[layer+1].clear();
		int last = -1;
		for (int i=0; i<e_tree[layer].size(); i++)
			if (e_tree[layer][i]->informative || layer > 0)
				if (last == -1)
					last = i;
				else
				{
					auto upper = shared_ptr<GeneralExampleHandle>(new GeneralExampleHandle());
					auto left = e_tree[layer][last];
					auto right = e_tree[layer][i];
					upper->informative = true;
					for (int j=0; j<left->positive_token.size(); j++)
						upper->positive_token.push_back(left->positive_token[j]);
					for (int j=0; j<right->positive_token.size(); j++)
						upper->positive_token.push_back(right->positive_token[j]);
					for (int j=0; j<left->negative_token.size(); j++)
						upper->negative_token.push_back(left->negative_token[j]);
					for (int j=0; j<right->negative_token.size(); j++)
						upper->negative_token.push_back(right->negative_token[j]);
					e_tree[layer+1].push_back(upper);
					last = -1;
				}
		if (last != -1)
			e_tree[layer+1].push_back(e_tree[layer][last]);
		cerr<<e_tree[layer+1].size()<<" informative data points left in layer "<<layer+1<<endl;
	}
	*/

	int collect_tree(shared_ptr<GeneralExampleHandle> e)
	{
		int depth;
		if (e->positive_token.size() <= minimal_example_size && 
			e->negative_token.size() <= minimal_example_size)
			depth = 1;
		else
		{
			auto e_left = shared_ptr<GeneralExampleHandle>(new GeneralExampleHandle());
			auto e_right = shared_ptr<GeneralExampleHandle>(new GeneralExampleHandle());

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

			int tmp_l = collect_tree(e_left);
			int tmp_r = collect_tree(e_right);
			depth = tmp_l>tmp_r?tmp_l:tmp_r;
			depth ++;
		}

		if (depth > e_tree.size())
			e_tree.push_back(vector<shared_ptr<GeneralExampleHandle> >());

		e_tree[depth-1].push_back(e);

		return depth;
	}

	vector<shared_ptr<GeneralSyntaxTree> > search_by_layer(
					vector<shared_ptr<GeneralSyntaxTree> >& global_pool)
	{
		vector<shared_ptr<GeneralSyntaxTree> > ans;
		try
		{
			for (int i=0; i<e_tree.size(); i++)
			{
				cerr<<"Size of global pool: "<<global_pool.size()<<endl;
				for (int j=0; j<e_tree[i].size(); j++)
				{	
					/* [TODO] replace magic number with parameter */
					int local_answer_count = answer_count * 
									((e_tree.size()-i) + e_tree.size()) / 
									e_tree.size(); 
					if (local_answer_count <= 0)
						local_answer_count = 1;

					double local_accuracy_pos = accuracy * (double)e_tree[i][0]->positive_token.size() / (double)e_tree.back()[0]->positive_token.size();
					double local_accuracy_neg = accuracy * (double)e_tree[i][0]->negative_token.size() / (double)e_tree.back()[0]->negative_token.size();
					double local_accuracy = std::min(local_accuracy_pos, local_accuracy_neg);
					if (local_accuracy < 0.01)
						local_accuracy = 0;
					cerr<<("Accuracy requirement:"+_S_(local_accuracy));
					cerr<<("Answer requirement:"+_S_(local_answer_count));
					real_search_single(e_tree[i][j], local_answer_count, local_accuracy, global_pool);
				}

//				merge_tree(i);

				cerr<<"Size of global pool: "<<global_pool.size()<<endl;
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
				cerr<<"Size of global pool: "<<global_pool.size()<<endl;
			}
		}
		catch (vector<shared_ptr<GeneralSyntaxTree> > _ans)
		{
			ans = _ans;
		}
		return ans;
	}

	vector<shared_ptr<GeneralSyntaxTree> > real_search_single(
					shared_ptr<GeneralExampleHandle> e, 
					int local_answer_count, 
					double local_accuracy,
					vector<shared_ptr<GeneralSyntaxTree> >& global_pool)
	{
		cerr<<"Searching! Size: "<<e->positive_token.size()<<" "<<e->negative_token.size()<<endl;

		vector<shared_ptr<GeneralSyntaxTree> > ans;
		vector<shared_ptr<GeneralSyntaxTree> > ans_wrong;

		for (int i=0; i<global_pool.size(); i++)
			if (global_pool[i]->to_program()->accept(e))
				ans.push_back(static_pointer_cast<GeneralSyntaxTree>(
								SyntaxTree::factory->get_new(global_pool[i])));
			else
				ans_wrong.push_back(static_pointer_cast<GeneralSyntaxTree>(
								SyntaxTree::factory->get_new(global_pool[i])));
	
		e->informative = true;
		if (ans.size() >= local_answer_count)
			if (std::experimental::randint(0,local_answer_count*force_search_factor) 
								== local_answer_count/2)
				local_answer_count = ans.size() + 1;
			else
				e->informative = false;


		if (ans.size() < local_answer_count)
		{
			vector<shared_ptr<IESyntaxTree> > seed;
			std::unordered_set<shared_ptr<SyntaxTree>, HashSyntaxTree, CmpSyntaxTree > eliminate;
			for (int i=0; i<ans.size(); i++)
				eliminate.insert(static_pointer_cast<GeneralSyntaxTree>(
								SyntaxTree::factory->get_new(ans[i])));
			for (int i=0; i<ans_wrong.size(); i++)
			{
				eliminate.insert(static_pointer_cast<GeneralSyntaxTree>(
								SyntaxTree::factory->get_new(ans_wrong[i])));
				for (int j=3; j<=7; j++)
				{
					seed.push_back(static_pointer_cast<GeneralSyntaxTree>(
									SyntaxTree::factory->get_new(ans_wrong[i])));
					static_pointer_cast<GeneralSyntaxTree>(seed.back())->prune(j);
				}
			}

			SearchGraph graph(depth_threshold, 
							batch_size, 
							explore_rate, 
							accuracy,
							local_answer_count-ans.size(), 
							threads, 
							starting_symbol, 
							rp);

			auto ans_tmp = graph.search_top_level_v2(e, seed, eliminate);
			cerr<<"Search done!!!\n";
			cerr<<"Size of global pool: "<<global_pool.size()<<endl;
			for (int i=0; i<ans_tmp.size(); i++)
			{
				cerr<<i<<endl;
				bool exist = false;
				for (int j=0; j<ans.size(); j++)
				{
					cerr<<j<<endl;
					if (ans[j]->equal(ans_tmp[i]))
					{
						exist = true;
						break;
					}
				}
				cerr<<"#2"<<endl;
				if (!exist)
				{
					ans.push_back(static_pointer_cast<GeneralSyntaxTree>(ans_tmp[i]));
					global_pool.push_back(static_pointer_cast<GeneralSyntaxTree>(
									SyntaxTree::factory->get_new(ans.back())));
					cerr<<"#2.5"<<endl;
					if (global_pool.back()->to_program()->accept(top_example, {accuracy}))
					{
						cerr<<"Answer found! Current task size: "<<
										e->positive_token.size()<<" "<<
										e->negative_token.size()<<endl;
						cerr<<"Size of global pool: "<<global_pool.size()<<endl;
						cerr<<"Total programs searched: "<<total_programs_searched<<endl;
						cerr<<global_pool.back()->to_string()<<endl;;
						global_answer.push_back(global_pool.back());
						if (global_answer.size() >= answer_count)
							throw global_answer;
					}
				}
				cerr<<"#3"<<endl;
			}
		}
		cerr<<"Done! Size: "<<e->positive_token.size()<<" "<<e->negative_token.size()<<endl;
		return ans;
	}

};



