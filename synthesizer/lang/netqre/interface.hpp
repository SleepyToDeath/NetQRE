#ifndef NETQRE_INTERFACE_HPP
#define NETQRE_INTERFACE_HPP

#include "../general/general.hpp"
#include "../../../netqre2dt/interpreter.h"
#include "../../../netqre2dt/parser.h"
#include <sstream>
#include <algorithm>

using std::min;
using std::max;

class NetqreExample: public GeneralExample {
	public:
	vector<TokenStream> positive_token;
	vector<TokenStream> negative_token;
};

class NetqreInterpreterInterface: public GeneralInterpreter {
	public:

	GeneralMatchingResult accept(AbstractCode code, bool complete,  shared_ptr<GeneralExample> input, IEConfig cfg) {
		auto e = std::static_pointer_cast<NetqreExample>(input);
		auto ast = parser.parse(code.pos);
		auto m = interpreter.interpret(ast);
		GeneralMatchingResult res;

		StreamFieldType pos_min_upper = -1;
		StreamFieldType pos_min_lower = -1;
		StreamFieldType neg_max_upper = -1;
		StreamFieldType neg_max_lower = -1;
		for (int i=0; i<e->positive_token.size(); i++)
		{
			auto s = e->positive_token[i];
			auto ans = m->process(s);
			if (ans->lower < 0)
			{
				res.accept = 0;
				res.utility_rate = 1.0;
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
			auto s = e->negative_token[i];
			auto ans = m->process(s);
			if (ans->lower < 0)
			{
				res.accept = 0;
				res.utility_rate = 1.0;
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

		res.accept = neg_max_lower < pos_min_upper;
		res.utility_rate = 1.0;
		
		if (neg_max_upper < pos_min_lower)
			cout<<"Answer found:"<<code.pos<<
			" Threshold range:"<<neg_max_upper<<" ~ "<<pos_min_lower<<endl;

		return res;
	}

	double extra_complexity(AbstractCode code) {
		return 0.0;
	}

	vector<string> get_range(int handle, shared_ptr<GeneralExample> input)
	{
		auto e = std::static_pointer_cast<NetqreExample>(input);
		vector<StreamFieldType> range_raw;
		vector<StreamFieldType> range;

		/* collect */
		for (int i=0; i<e->positive_token.size(); i++)
			for (int j=0; j<e->positive_token[i].size(); j++)
				range.push_back(e->positive_token[i][j][handle].value);
		for (int i=0; i<e->negative_token.size(); i++)
			for (int j=0; j<e->negative_token[i].size(); j++)
				range.push_back(e->negative_token[i][j][handle].value);

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
			ans.push_back(tmp);
		}

		return ans;
	}

	Netqre::Interpreter interpreter;
	Netqre::NetqreParser parser;
};

shared_ptr<NetqreExample> prepare_examples() {
	TokenStream pos;
	TokenStream neg;
	FeatureSlot zero(32, true, 1);
	FeatureSlot one(32, true, 0);
	FeatureVector v00;
	FeatureVector v01;
	FeatureVector v10;
	FeatureVector v11;
	v00.push_back(zero); v00.push_back(zero);
	v01.push_back(zero); v01.push_back(one);
	v10.push_back(one); v10.push_back(zero);
	v11.push_back(one); v11.push_back(one);
	pos.push_back(v01);
	pos.push_back(v11);
	pos.push_back(v01);
	pos.push_back(v11);
	pos.push_back(v00);
	pos.push_back(v10);
	pos.push_back(v01);
	pos.push_back(v11);
	pos.push_back(v01);
	pos.push_back(v11);
	pos.push_back(v00);
	pos.push_back(v10);
	pos.push_back(v01);
	pos.push_back(v01);
	pos.push_back(v00);

	neg.push_back(v01);
	neg.push_back(v01);
	neg.push_back(v00);

	auto ans = shared_ptr<NetqreExample>(new NetqreExample());
	ans->positive_token.push_back(pos);
	ans->negative_token.push_back(neg);
	return ans;
}

#endif
