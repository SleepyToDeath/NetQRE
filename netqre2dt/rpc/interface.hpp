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
using Rubify::string;
using Rubify::break_;
using Rubify::continue_;
using std::pair;
using std::shared_ptr;
using std::unique_ptr;
using std::cerr;

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

	vector< vector<StreamFieldType> > range;

	string to_s()
	{
		string ret = "";
		for (int handle = 0; handle < config.field_number; handle++)
			ret = ret + "Handle" +  _S_(handle) + ": " + range[handle].to_s() + "\n";
		return ret;
	}

	void collect_range() 
	{
		range.clear();

		for (int handle = 0; handle < config.field_number; handle++)
		{
			vector<StreamFieldType> range_raw;

			/* collect */
			for (int i=0; i<positive_token.size(); i++)
				for (int j=0; j<positive_token[i].size(); j++)
					range_raw.push_back(positive_token[i][j][handle]);
			for (int i=0; i<negative_token.size(); i++)
				for (int j=0; j<negative_token[i].size(); j++)
					range_raw.push_back(negative_token[i][j][handle]);

			/* remove dup */
			std::sort(range_raw.begin(), range_raw.end());
			range.push_back(vector<StreamFieldType>());
			range.back().push_back(range_raw[0]);
			for (int i=1; i<range_raw.size(); i++)
				if (range_raw[i] != range_raw[i-1])
					range.back().push_back(range_raw[i]);

		}
	}

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
			getline(fin, it_s); //eat \n from last line
			getline(fin, it_s);
			config.field_property = it_s.split(" ").map<StreamFieldProperty>([](string t)->StreamFieldProperty { 
				return static_cast<StreamFieldProperty>(t.to_i()); 
			});

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
					
					tmps.push_back(v_s.split(" ")
						.map<StreamFieldType>([](string feature)->StreamFieldType { 
							return feature.to_i(); 
						}));
				}
				target.push_back(tmps);
			}
			fin.close();
		};

		read_example(positive_token, positive_file);
		read_example(negative_token, negative_file);
		collect_range();
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

	vector<shared_ptr<NetqreExample> > shatter()
	{
		return positive_token.map<shared_ptr<NetqreExample> >( 
		[&] (auto ts) -> shared_ptr<NetqreExample> {
			auto e = shared_ptr<NetqreExample> (new NetqreExample());
			e->positive_token.push_back(ts);
			e->negative_token.clear();
			e->config = config;
			return e;
		});
	}

	shared_ptr<GeneralExampleHandle> to_handle(int pos_offset = 0, int neg_offset = 0) {
		auto ret = shared_ptr<NetqreExampleHandle>(new NetqreExampleHandle());
		ret->positive_token = positive_token.map<int>( [&](int index, auto s)->int {
			return index + pos_offset;
		});
		ret->negative_token = negative_token.map<int>( [&](int index, auto s)->int {
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

	/* [TODO] interpret from binary index to true number */
	string to_string(shared_ptr<GeneralSyntaxTree> code) {
		string s;
		if (code->root->get_type()->is_term) 
			s = code->root->get_type()->name;
		else if (code->root->get_option() == SyntaxLeftHandSide::NoOption)
			s = std::static_pointer_cast<GeneralSyntaxLeftHandSide>(code->root->get_type())->name;
		else
		{
			auto rhs = std::static_pointer_cast<GeneralSyntaxRightHandSide> (code->root->get_type()->option[code->root->get_option()]);
			int j = 0;
			for (int i=0; i<rhs->subexp_full.size(); i++)	{
				if (rhs->subexp_full[i]->is_functional()) {
					s = s + (code->subtree[j]->to_string());
					j++;
				}
				else {
					s = s + (rhs->subexp_full[i]->name);
				}
			}
		}
		return s;
	}

	GeneralTestResult test(string code, shared_ptr<GeneralExample> input) { 
		auto e = std::static_pointer_cast<NetqreExampleHandle>(input);
		vector<std::unique_ptr<Netqre::IntValue> > ans_pos;
		vector<std::unique_ptr<Netqre::IntValue> > ans_neg;
		GeneralTestResult res;
		int pos_counter = 0;
		int neg_counter = 0;

		manager->exec(code, e, ans_pos, ans_neg);

/*
		cerr<< ans_pos.map<string>( [&](const std::unique_ptr<Netqre::IntValue>& ptr)->string {
			return ptr->to_s();
		}).to_s() <<std::endl;
		*/


		/* make as much match as possible */
		ans_pos.each( [](auto& ans) {
			ans->lower = ans->upper;
		});
		ans_neg.each( [](auto& ans) {
			ans->lower = ans->upper;
		});


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

//		cerr<<"Threshold: "<<e->threshold<<endl;
//		cerr<<"accurate predictions:"<<pos_counter<<" "<<neg_counter<<endl;
		res.pos_accuracy = (double)pos_counter / (double)ans_pos.size();
		res.neg_accuracy = (double)neg_counter / (double)ans_neg.size();

		return res;
	}

	class HandleNAnswer {
		public:
		Netqre::IntValue answer;
		int handle;
	};


	GeneralMatchingResult accept(
					AbstractCode code, 
					bool complete,  
					shared_ptr<GeneralExample> input, 
					IEConfig cfg, 
					shared_ptr<GeneralSolutionGroupConstraint> constraint ) 
	{

		/* trivial request */
		if (cfg.required_accuracy <= 0)
			return true;
		if (cfg.required_accuracy > 1)
			return false;

		auto e = std::static_pointer_cast<NetqreExampleHandle>(input);

		std::pair< std::string, shared_ptr<NetqreExampleHandle> > key;
		key.first = code.pos;
		key.second = e;

		auto ast = parser.parse(code.pos);
		auto m = interpreter.interpret(ast);
		GeneralMatchingResult res;

		auto master = require_(std::thread::id, "master_id");
		auto e_top = require_from_(master, shared_ptr<NetqreExample>, "global_example");
		m->bind_context(e_top);
		if (!m->valid())
		{
			res = false;
			return res;
		}

//		StreamFieldType pos_min_upper = -1;
//		StreamFieldType pos_min_lower = -1;
//		StreamFieldType neg_max_upper = -1;
//		StreamFieldType neg_max_lower = -1;

		vector<std::unique_ptr<Netqre::IntValue> > ans_pos_buf;
		vector<std::unique_ptr<Netqre::IntValue> > ans_neg_buf;

		manager->exec(code.pos, e, ans_pos_buf, ans_neg_buf);

		/* remove unique_ptr for sorting */
		auto ans_pos = ans_pos_buf.map<HandleNAnswer>( [&](int index, const unique_ptr<Netqre::IntValue>& ptr)->HandleNAnswer {
			return {*ptr, e->positive_token[index]};
		});
		auto ans_neg = ans_neg_buf.map<HandleNAnswer>( [&](int index, const unique_ptr<Netqre::IntValue>& ptr)->HandleNAnswer {
			return {*ptr, e->negative_token[index]};
		});

		if (complete)
		{
			/* make as much match as possible */
			ans_pos.each( [](auto& ans) {
				ans.answer.lower = ans.answer.upper;
			});
			ans_neg.each( [](auto& ans) {
				ans.answer.lower = ans.answer.upper;
			});


			/* pack with group_by to ensure stable sort */
			ans_pos = ans_pos.sort_by<StreamFieldType>( [](auto ans)->StreamFieldType {
				return ans.answer.upper;
			}).group_by<StreamFieldType>( [](auto ans)->StreamFieldType {
				return ans.answer.lower;
			}).sort_by<StreamFieldType>( [](auto ans_v)->StreamFieldType {
				return ans_v[0].answer.lower;
			}).flatten();

			ans_neg = ans_neg.sort_by<StreamFieldType>( [](auto ans)->StreamFieldType {
				return -(ans.answer.lower);
			}).group_by<StreamFieldType>( [](auto ans)->StreamFieldType {
				return ans.answer.upper;
			}).sort_by<StreamFieldType>( [](auto ans_v)->StreamFieldType {
				return -(ans_v[0].answer.upper);
			}).flatten();

			int invalid_count = 0;
			vector<int> valid_pos_handle;
			vector<int> valid_neg_handle;
			auto ans_both = ans_pos.norm_zip(ans_neg);

			ans_both.each( [&](auto pair) {
				if (pair[0].answer.lower < 0 || pair[1].answer.lower < 0) // not matched
					invalid_count ++;
				else if (pair[0].answer.lower < pair[1].answer.upper) // not satisfying
					invalid_count ++;
				else if (	pair[0].answer.lower == pair[0].answer.upper && // ambiguous
							pair[0].answer.upper == pair[1].answer.lower && 
							pair[1].answer.lower == pair[1].answer.upper )
					invalid_count ++;
				else
				{
					valid_pos_handle.push_back(pair[0].handle);
					valid_neg_handle.push_back(pair[1].handle);
				}
			});

			double valid_value = 0;
			valid_pos_handle.zip(valid_neg_handle).each( [&](auto pair) {
				if (constraint == nullptr)
					valid_value += 1.0;
				else
					valid_value += (constraint->pos_weight[pair[0]] + constraint->neg_weight[pair[1]]) / 2.0;
			});

			/* too many invalid */
			if (valid_value / ((double)(ans_both.size())) < cfg.required_accuracy)
				return false;
			else
			{
				if (constraint != nullptr)
				{
					valid_pos_handle.each( [&](auto handle) {
						double& w = constraint->pos_weight[handle];
						w = constraint->updater(w);
					});
					valid_neg_handle.each( [&](auto handle) {
						double& w = constraint->neg_weight[handle];
						w = constraint->updater(w);
					});
				}

				auto pair = ans_both[invalid_count]; // the decisive pair

				e->threshold = (pair[0].answer.lower + pair[1].answer.upper)/2;
				if (pair[0].answer.lower != pair[0].answer.upper)
					e->indistinguishable_is_negative = true;
				else
					e->indistinguishable_is_negative = false;

//				cerr<<"[Accuracy:" S_(((double)invalid_count) / ((double)(ans_both.size()))) "]\n";
//				cerr<<"[Threshold:" S_(e->threshold) "]\n";
				return true;
			}
		}
		else 
		{
			ans_pos = ans_pos.sort_by<StreamFieldType>( [](auto ans)->StreamFieldType {
				return ans.answer.upper;
			});

			ans_neg = ans_neg.sort_by<StreamFieldType>( [](auto ans)->StreamFieldType {
				return -(ans.answer.lower);
			});

			int invalid_count = 0;
			auto ans_both = ans_pos.norm_zip(ans_neg);

			ans_both.each( [&](auto pair) {
				if (pair[0].answer.lower < 0 || pair[1].answer.lower < 0) // not matched
					invalid_count ++;
				else if (pair[0].answer.upper <= pair[1].answer.lower) // not satisfying
					invalid_count ++;
			});

			/* too many invalid */
			if (((double)invalid_count) / ((double)(ans_both.size())) > (1.0 - cfg.required_accuracy))
				return false;
			else
				return true;
		}
	
/*
		for (int i=0; i<e->positive_token.size(); i++)
		{
			unique_ptr<Netqre::IntValue>& ans = ans_pos[i];
			if (ans->lower < 0)
			{
				res = false;
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
		}

		for (int i=0; i<e->negative_token.size(); i++)
		{
			unique_ptr<Netqre::IntValue>& ans = ans_neg[i];
			if (ans->lower < 0)
			{
				res = false;
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
		}

		if (!complete)
		{
			res = neg_max_lower < pos_min_upper;
		}
		else
		{
			res =	(neg_max_upper < pos_min_lower) 
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
			if (res)
			{
				e->threshold = (pos_min_lower + neg_max_upper)/2;
				if (pos_min_lower != pos_min_upper)
					e->indistinguishable_is_negative = true;
				else
					e->indistinguishable_is_negative = false;
			}
		}
		return res;
		*/
	}

	class ComplexityContext
	{
		public:
		bool missing_predicate = false;
		bool missing_non_predicate = false;
	};

	constexpr static double UNIT = 100.0;
	constexpr static double INCOMPLETE_PENALTY = 1.5;
	constexpr static double INVALID_PANELTY = 100.0;
	constexpr static double PREFER_REWARD = 0.2;
	constexpr static double UNFAVORED_PANELTY = 10.0;

	double extra_complexity(shared_ptr<GeneralSyntaxTree> code) {
		ComplexityContext ctxt;
		double ret = real_extra_complexity(code, ctxt);
		if (ctxt.missing_predicate)
			ret -= UNIT * PREFER_REWARD;
		return ret;
	}

	double real_extra_complexity(shared_ptr<GeneralSyntaxTree> code, ComplexityContext& ctxt) {

		double complexity = 0;
		string name = code->root->get_type()->name;
		auto option = code->root->get_option();
		bool is_term = code->root->get_type()->is_term;
		auto prune_count = code->prune_count;
		if (is_term)
		{
			complexity = UNIT;
			if ((name == "_")
			 	||(name == "*(_)"))
				complexity += INCOMPLETE_PENALTY * UNIT;
		}
		else if (option == SyntaxLeftHandSide::NoOption)
		{
			if (!(name == "#index" ||
				name == "#prefix" ||
				name == "#cap_index" ||
				name == "#bottom_index"))
				ctxt.missing_non_predicate = true;
			else
				ctxt.missing_predicate = true;

			if (ctxt.missing_predicate && ctxt.missing_non_predicate)
				complexity = UNIT * INVALID_PANELTY;
			else
				complexity = UNIT * INCOMPLETE_PENALTY;
		}
		else
		{
			complexity = 0;
			for (int i=0; i<code->subtree.size(); i++)
				complexity += real_extra_complexity(static_pointer_cast<GeneralSyntaxTree>(code->subtree[i]), ctxt);

			complexity += (code->subtree.size()-1) * UNIT;

			if ((name == "#re")
				|| (name == "#predicate_set")
				|| (name == "#predicate_entry"))
				complexity -= UNIT * PREFER_REWARD;
			if (name == "#qre_vs")
				complexity = complexity * 2;
//			complexity -= prune_count * 100;
		}
		return complexity;
	}

	vector<string> get_range(int handle, shared_ptr<GeneralExample> input)
	{
		auto e = std::static_pointer_cast<NetqreExample>(input);
		cerr<<"Getting range for "<<handle<<endl;

		if  (handle == 0)
		{
			vector<string> ans;
			for(int i=0; i<e->config.field_number; i++)
				ans.push_back(_S_(i));
			return ans;
		}

		if (handle == 1)
		{
			vector<string> ans;
			for(int i=0; i<e->config.field_number; i++)
				if (e->config.field_property[i] == StreamFieldProperty::DISCRETE)
					ans.push_back(_S_(i));
			return ans;
		}

		if (handle == 2)
		{
			vector<string> ans;
			for(int i=0; i<e->config.field_number; i++)
				if (e->config.field_property[i] == StreamFieldProperty::SCALAR || e->config.field_property[i] == StreamFieldProperty::RANGED)
					ans.push_back(_S_(i));
			return ans;
		}

		if (handle == 3)
		{
			vector<string> ans;
			for(int i=0; i<e->config.field_number; i++)
				if (e->config.field_property[i] == StreamFieldProperty::SCALAR)
					ans.push_back(_S_(i));
			return ans;
		}

		if (handle == 4)
		{
			vector<string> ans;
			for(int i=0; i<e->config.field_number; i++)
				if (e->config.field_property[i] == StreamFieldProperty::SCALAR)
					ans.push_back(_S_(i));
			return ans;
		}
	
		return vector<string>();
	}

	

	Netqre::Interpreter interpreter;
	Netqre::NetqreParser parser;
	Netqre::NetqreClientManager* manager;
};


#endif
