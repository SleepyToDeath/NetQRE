#ifndef NETQRE_INTERFACE_HPP
#define NETQRE_INTERFACE_HPP

#include "client.hpp"
#include "../../general-lang/general.hpp"
#include "../interpreter.h"
#include "../parser.h"
#include <sstream>
#include <algorithm>
#include <fstream>
#include <utility>
#include <memory>


using std::min;
using std::max;
using std::ifstream;
using Rubify::map;
using Rubify::vector;
using std::max;
using Rubify::string;
using std::pair;
using std::shared_ptr;
using std::unique_ptr;

class NetqreExampleHandle: public GeneralExampleHandle {
	public:
	double threshold = 0;
	bool indistinguishable_is_negative = true; // indistinguishable == both bounds equals threshold
};

class NetqreExample: public GeneralExample{
	public:
	vector<TokenStream> positive_token;
	vector<TokenStream> negative_token;
	StreamConfig config;

	void from_file(string positive_file, string negative_file)
	{
		auto read_example = [&](vector<TokenStream>& target, string file_name)
		{
			std::ifstream fin;
			fin.open(file_name);
			if (!fin.good())
				return;

			int traces;
			fin>>traces>>config.field_number;
			
			string it_s;
			getline(fin, it_s);
			config.field_iterative = it_s.split(" ").map<bool>([](string t)->bool { return t == "1"; });

			/* read data */
			for (int i=0; i<traces; i++)
			{
				TokenStream tmps;
				for(;;)
				{
					string v_s;
					getline(fin, v_s);
					if (v_s == "")
						break;
					
					tmps.push_back(v_s.split(" ").map<long long>([](string feature)->long long { feature.to_i(); }));
				}
				target.push_back(tmps);
			}
			fin.close();
		};

		read_example(positive_token, positive_file);
		read_example(negative_token, negative_file);
	}

	shared_ptr<NetqreExample> split()
	{
		auto suf = shared_ptr<NetqreExample>(new NetqreExample());
		suf->config = config;
		int mid_pos = positive_token.size()/2;
		int mid_neg = negative_token.size()/2;
		suf->positive_token = positive_token.slice(mid_pos, positive_token.size()-mid_pos);
		suf->negative_token = negative_token.slice(mid_neg, negative_token.size()-mid_neg);
		positive_token = positive_token.slice(0, mid_pos);
		negative_token = negative_token.slice(0, mid_neg);
		return suf;
	}

	shared_ptr<NetqreExampleHandle> to_handle(int pos_offset = 0, int neg_offset = 0) {
		auto ret = shared_ptr<NetqreExampleHandle>(new NetqreExampleHandle());
		ret->positive_token = positive.map<int>( [](int index, auto s)->int {
			return index + pos_offset;
		});
		ret->negative_token = negative.map<int>( [](int index, auto s)->int {
			return index + neg_offset;
		});
		return ret;
	}

};



class NetqreInterpreterInterface: public GeneralInterpreter {
	public:
	NetqreInterpreterInterface(vector<string> servers, vector<int> ports){
		manager = new Netqre::NetqreClientManager(servers, ports);
	}

	GeneralTestResult test(string code, shared_ptr<GeneralExample> input) { 
		auto e = std::static_pointer_cast<NetqreExampleHandle>(input);
		vector<std::unique_ptr<Netqre::IntValue> > ans_pos;
		vector<std::unique_ptr<Netqre::IntValue> > ans_neg;
		GeneralTestResult res;
		int pos_counter = 0;
		int neg_counter = 0;

		manager->exec(code, e, ans_pos, ans_neg);

		for (int i=0; i<ans_pos.size(); i++)
		{
			unique_ptr<Netqre::IntValue>& ans = ans_pos[i];
			if (e->indistinguishable_is_negative)
			{
				if (ans->upper > e->threshold && ans->lower >= e->threshold)
					pos_counter ++;
			}
			else
			{
				if (ans->upper >= e->threshold && ans->lower >= e->threshold)
					pos_counter ++;
			}
		}

		for (int i=0; i<ans_neg.size(); i++)
		{
			unique_ptr<Netqre::IntValue>& ans = ans_neg[i];
			if (e->indistinguishable_is_negative)
			{
				if (ans->upper <= e->threshold && ans->lower <= e->threshold)
					neg_counter ++;
			}
			else
			{
				if (ans->upper <= e->threshold && ans->lower < e->threshold)
					neg_counter ++;
			}
		}

		cout<<"Threshold: "<<e->threshold<<endl;
		cout<<"accurate predictions:"<<pos_counter<<" "<<neg_counter<<endl;
		res.pos_accuracy = (double)pos_counter / (double)ans_pos.size();
		res.neg_accuracy = (double)neg_counter / (double)ans_neg.size();

		return res;
	}


	GeneralMatchingResult accept(AbstractCode code, bool complete,  shared_ptr<GeneralExample> input, IEConfig cfg) {
		auto e = std::static_pointer_cast<NetqreExampleHandle>(input);

		std::pair< std::string, shared_ptr<NetqreExampleHandle> > key;
		key.first = code.pos;
		key.second = e;
//		if (cache.count(key) > 0)
//			return cache[key];

		auto ast = parser.parse(code.pos);
		auto m = interpreter.interpret(ast);
		GeneralMatchingResult res;

		if (!m->valid())
		{
			res.accept = false;
			res.utility_rate = 1.0;
//			cache[key] = res;
			return res;
		}

		StreamFieldType pos_min_upper = -1;
		StreamFieldType pos_min_lower = -1;
		StreamFieldType neg_max_upper = -1;
		StreamFieldType neg_max_lower = -1;

		vector<std::unique_ptr<Netqre::IntValue> > ans_pos;
		vector<std::unique_ptr<Netqre::IntValue> > ans_neg;

		manager->exec(code.pos, e, ans_pos, ans_neg);

		for (int i=0; i<e->positive_token.size(); i++)
		{
//			auto s = e->positive_token[i];
//			auto ans = m->process(s);
			unique_ptr<Netqre::IntValue>& ans = ans_pos[i];
			if (ans->lower < 0)
			{
				res.accept = false;
				res.utility_rate = 1.0;
//				cache[key] = res;
				return res;
			}
			if (i==0)
			{
				pos_min_upper = ans->upper;
				pos_min_lower = ans->lower;
			}
			else
			{
				pos_min_upper = min(pos_min_upper, ans->upper);
				pos_min_lower = min(pos_min_lower, ans->lower);
			}
//			ans_pos.push_back(std::move(ans));
		}

		for (int i=0; i<e->negative_token.size(); i++)
		{
//			auto s = e->negative_token[i];
//			auto ans = m->process(s);
			unique_ptr<Netqre::IntValue>& ans = ans_neg[i];
			if (ans->lower < 0)
			{
				res.accept = false;
				res.utility_rate = 1.0;
//				cache[key] = res;
				return res;
			}
			if (i==0)
			{
				neg_max_upper = ans->upper;
				neg_max_lower = ans->lower;
			}
			else
			{
				neg_max_upper = max(neg_max_upper, ans->upper);
				neg_max_lower = max(neg_max_lower, ans->lower);
			}
//			ans_neg.push_back(std::move(ans));
		}

		if (!complete)
		{
			res.accept = neg_max_lower < pos_min_upper;
		}
		else
		{
			res.accept = 	(neg_max_upper < pos_min_lower) 
							|| 
					 		(
								(neg_max_upper == pos_min_lower) 
								&& 
								(
									(neg_max_lower != neg_max_upper) 
									|| 
									(pos_min_lower != pos_min_upper)
								)
							);
			if (res.accept)
			{
				e->threshold = (pos_min_lower + neg_max_upper)/2;
				if (pos_min_lower != pos_min_upper)
					e->indistinguishable_is_negative = true;
				else
					e->indistinguishable_is_negative = false;
			}
			if (res.accept && e->negative_token.size()>=100)
			{
				cout<<"Answer found:"<<code.pos<<endl<<" Threshold range:"<<neg_max_upper<<" ~ "<<pos_min_lower<<endl;
				cout<<"[output]:"<<neg_max_lower<<" ~ "<<neg_max_upper<<"     ~     "<<pos_min_lower<<" ~ "<<pos_min_upper<<endl;
				/*
				for (int i=0; i<ans_pos.size(); i++)
					cout<<"Positive output#"<<i<<": "<<ans_pos[i]->lower<<"~"<<ans_pos[i]->upper<<endl;
				for (int i=0; i<ans_neg.size(); i++)
					cout<<"Negative output#"<<i<<": "<<ans_neg[i]->lower<<"~"<<ans_neg[i]->upper<<endl;
					*/
			}
		}

		res.utility_rate = 1.0;

//		cache[key] = res;
		return res;
	}

	double extra_complexity(shared_ptr<GeneralSyntaxTree> code) {
		double complexity = 0;
		string name = code->root->get_type()->name;
		auto option = code->root->get_option();
		bool is_term = code->root->get_type()->is_term;
		auto prune_count = code->prune_count;
		if (is_term)
		{
//				complexity = -100.0;
			if ( name == "_")
				complexity = 300;
		}
		else if (option == SyntaxLeftHandSide::NoOption)
		{
			complexity = 300.0;
			if (name == "#feature_set")
				complexity = 500.0;
			if (name == "#agg_op")
				complexity = 500.0;
			if (name == "#re")
				complexity = 200.0;
			complexity -= prune_count * 200;
		}
		else
		{
			complexity = 0;
			for (int i=0; i<code->subtree.size(); i++)
				complexity += extra_complexity(subtree[i]);

			complexity += (subtree.size()-1) * 150.0;
			complexity -= prune_count * 100;

			if (name == "#predicate_set" && code->subtree.size() == 1)
				complexity += 300;
			if (name == "#predicate_set" && code->subtree.size() == 2)
				complexity -= 600;
		}
		return complexity;
	}

	vector<string> get_range(int handle, shared_ptr<GeneralExample> input)
	{
		auto e = std::static_pointer_cast<NetqreExample>(input);
		cout<<"Getting range for "<<handle<<endl;

		if  (handle == 0)
		{
			vector<string> ans;
			for(int i=0; i<e->config.field_number; i++)
			{
				if (e->config.field_iterative[i])
				{
					stringstream ss;
					string tmp;
					ss<<i;
					ss>>tmp;
					ans.push_back(tmp);
				}
			}
			return ans;
		}
		else
		{
			handle--;
		}

		vector<StreamFieldType> range_raw;
		vector<StreamFieldType> range;
		range_raw.clear();
		range.clear();

		/* collect */
		for (int i=0; i<e->positive_token.size(); i++)
			for (int j=0; j<e->positive_token[i].size(); j++)
				range_raw.push_back(e->positive_token[i][j][handle]);
		for (int i=0; i<e->negative_token.size(); i++)
			for (int j=0; j<e->negative_token[i].size(); j++)
				range_raw.push_back(e->negative_token[i][j][handle]);

		/* remove dup */
		std::sort(range_raw.begin(), range_raw.end());
		range.push_back(range_raw[0]);
		for (int i=1; i<range_raw.size(); i++)
			if (range_raw[i] != range_raw[i-1])
				range.push_back(range_raw[i]);

		/* convert to string */
		vector<string> ans;
		for (int i=0; i<range.size(); i++)
		{
			stringstream ss;
			string tmp;
			ss<<range[i];
			ss>>tmp;
			cout<<"range+= "<<tmp<<endl;
			ans.push_back(tmp);
		}

		return ans;
	}

	

	Netqre::Interpreter interpreter;
	Netqre::NetqreParser parser;
	Netqre::NetqreClientManager* manager;
};


#endif
