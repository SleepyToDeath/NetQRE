#include "../search_graph.h"
#include "../search_tree.h"
#include "../syntax_tree.h"
#include <iostream>
#include <vector>
#include <map>

class RegexSearchState;

class RegexSyntaxType {
	ZERO, ONE, CHAR, RE
};

class RegexExampleType: public ExampleType {
	public:
	RegexExampleType(std::string s0) {
		s = s0;
	}

	bool match(SearchState* state, SyntaxLeftHandSide* terminal) {
		RegexSearchState* rstate = (RegexSearchState*) state;
		if (r-l>1)
			return false;
		if ((terminal->id == ZERO) && (s[l] =='0'))
			return true;
		if ((terminal->id == one) && (s[l] =='1'))
			return true;
		return false;
	}

	std::string s;
};

class RegexSearchState: public SearchState {
	public:
	int l,r;
	RegexSyntaxType type;
};

template<class T>
class RegexSearchTreeCacheFactory: public SearchTreeCacheFactory {
	public:
	SearchTreeCache* get_cache() {
		return new RegexSearchTreeCache();
	}
};

template<class T>
class RegexSearchTreeCache: public SearchTreeCache {
	public:
	T& operator [](SearchState* state) {
		RegexSearchState* rstate = (RegexSearchState*) state;
		return m[rstate*];
	}

	int count(SearchState* state) {
		RegexSearchState* rstate = (RegexSearchState*) state;
		return m.count(rstate*);
	}

	std::map<RegexSearchState,T> m;
};

/* one for each right hand side option */
class RegexConcatDivideStrategy: public DivideStrategy {
	public:
	std::vector< std::vector<SearchState*> > get_indep_substates(SearchState* state) {
		RegexSearchState* rstate = (RegexSearchState*) state;
		std::vector< std::vector<SearchState*> > ans;
		for (int i=rstate->l+1; i<rstate->r; i++)
		{
			std::vetor<SearchState*> div;
			RegexSearchState* left = new RegexSearchState;
			RegexSearchState* right = new RegexSearchState;
			left->l = rstate->l;
			left->r = i;
			left->type = RE;
			right->l = i;
			right->r = rstate->r;
			right->type = RE;
			div.push_back(left);
			div.push_back(right);
			ans.push_back(div);
		}
		return ans;
	}

	int get_min(SearchState* state) {
		RegexSearchState* rstate = (RegexSearchState*) state;
		return rstate->l;
	}
	
	int get_max(SearchState* state) {
		RegexSearchState* rstate = (RegexSearchState*) state;
		return rstate->r;
	}

	SearchState* get_dep_substates(SearchState* s, int min, int max)
	{
		return nullptr
	}

	bool valid_combination(std::vector<bool> valid_subexp) {
		for (int i=0; i<valid_subexp.size(); i++)
			if (!valid_subexp[i])
				return false;
		return true;
	}
};

class RegexCharDivideStrategy : public DivideStrategy{
	public:
	std::vector< std::vector<SearchState*> > get_indep_substates(SearchState* state) {
		std::vector< std::vector<SearchState*> > ans;
		std::vector<SearchState*> ans_inner;
		RegexSearchState* rstate = (RegexSearchState*) state;
		RegexSearchState* ans_final = new RegexSearchState();
		ans_final->l = rstate->l;
		ans_final->r = rstate->r;
		ans_final->type = CHAR;
		ans_inner.push_back(ans_final);
		ans.push_back(ans_inner);
	}

	int get_min(SearchState* state) {
		RegexSearchState* rstate = (RegexSearchState*) state;
		return rstate->l;
	}
	
	int get_max(SearchState* state) {
		RegexSearchState* rstate = (RegexSearchState*) state;
		return rstate->r;
	}

	SearchState* get_dep_substates(SearchState* s, int min, int max)
	{
		return nullptr
	}

	bool valid_combination(std::vector<bool> valid_subexp) {
		for (int i=0; i<valid_subexp.size(); i++)
			if (!valid_subexp[i])
				return false;
		return true;
	}
};

class RegexZeroDivideStrategy : public DivideStrategy{
	public:
	std::vector< std::vector<SearchState*> > get_indep_substates(SearchState* state) {
		std::vector< std::vector<SearchState*> > ans;
		std::vector<SearchState*> ans_inner;
		RegexSearchState* rstate = (RegexSearchState*) state;
		RegexSearchState* ans_final = new RegexSearchState();
		ans_final->l = rstate->l;
		ans_final->r = rstate->r;
		ans_final->type = ZERO;
		ans_inner.push_back(ans_final);
		ans.push_back(ans_inner);
	}

	int get_min(SearchState* state) {
		RegexSearchState* rstate = (RegexSearchState*) state;
		return rstate->l;
	}
	
	int get_max(SearchState* state) {
		RegexSearchState* rstate = (RegexSearchState*) state;
		return rstate->r;
	}

	SearchState* get_dep_substates(SearchState* s, int min, int max)
	{
		return nullptr
	}

	bool valid_combination(std::vector<bool> valid_subexp) {
		for (int i=0; i<valid_subexp.size(); i++)
			if (!valid_subexp[i])
				return false;
		return true;
	}
};


class RegexOneDivideStrategy : public DivideStrategy {
	public:
	std::vector< std::vector<SearchState*> > get_indep_substates(SearchState* state) {
		std::vector< std::vector<SearchState*> > ans;
		std::vector<SearchState*> ans_inner;
		RegexSearchState* rstate = (RegexSearchState*) state;
		RegexSearchState* ans_final = new RegexSearchState();
		ans_final->l = rstate->l;
		ans_final->r = rstate->r;
		ans_final->type = ONE;
		ans_inner.push_back(ans_final);
		ans.push_back(ans_inner);
	}

	int get_min(SearchState* state) {
		RegexSearchState* rstate = (RegexSearchState*) state;
		return rstate->l;
	}
	
	int get_max(SearchState* state) {
		RegexSearchState* rstate = (RegexSearchState*) state;
		return rstate->r;
	}

	SearchState* get_dep_substates(SearchState* s, int min, int max)
	{
		return nullptr
	}

	bool valid_combination(std::vector<bool> valid_subexp) {
		for (int i=0; i<valid_subexp.size(); i++)
			if (!valid_subexp[i])
				return false;
		return true;
	}
};

SyntaxLeftHandSide* l_re;
SyntaxLeftHandSide* l_ch;
SyntaxLeftHandSide* l_zero;
SyntaxLeftHandSide* l_one;

SyntaxRightHandSide* r_concat;
SyntaxRightHandSide* r_ch;
SyntaxRightHandSide* r_zero;
SyntaxRightHandSide* r_one;

RegexConcatDivideStrategy* d_concat;
RegexCharDivideStrategy* d_ch;
RegexZeroDivideStrategy* d_zero;
RegexOneDivideStrategy* d_one;

RHSToDivider* r2d;

RegexSearchTreeCacheFactory* cache_pool;

void init()
{
	l_re = new SyntaxLeftHandSide;
	l_ch = new SyntaxLeftHandSide();
	l_zero = new SyntaxLeftHandSide()();
	l_one = new SyntaxLeftHandSide();

	r_concat = new SyntaxRightHandSide();
	r_ch = new SyntaxRightHandSide();
	r_zero = new SyntaxRightHandSide();
	r_one = new SyntaxRightHandSide();

	d_concat = new RegexConcatDivideStrategy();
	d_ch = new RegexCharDivideStrategy();
	d_zero = new RegexZeroDivideStrategy();
	d_one = new RegexOneDivideStrategy();

	r2d = new RHSToDivider();

	l_re->id = RE;
	l_ch->id = CHAR;
	l_zero->id = ZERO;
	l_one->id = ONE;

	l_re->is_term = false;
	l_ch->is_term = false;
	l_zero->is_term = false;
	l_one->is_term = false;

	r_concat->independent = true;
	r_ch->independent = true;
	r_zero->independent = true;
	r_one->independent = true;

	l_re->option.push_back(r_concat);
	l_re->option.push_back(r_ch);
	l_ch->option.push_back(r_zero);
	l_ch->option.push_back(r_one);

	r_concat->subexp.push_back(l_re);
	r_concat->subexp.push_back(l_re);
	r_ch->subexp.push_back(l_ch);
	r_zero->subexp.push_back(l_zero);
	r_one->subexp.push_back(l_one);

	r2d->register_divider(r_concat, d_concat);
	r2d->register_divider(r_ch, d_ch);
	r2d->register_divider(r_zero, d_zero);
	r2d->register_divider(r_one, d_one);

	cache_pool = new RegexSearchTreeCacheFactory();
}

int main()
{
	std::string example_string;
	init();
	SearchGraph g(10, l_re, r2d, SearchTreeCacheFactory* cache_pool, );
	std::cin>> example_string;
	std::vector<RegexExampleType*> example;
	example.push_back(new RegexExampleType(example_string);
	SyntaxTree* search_top_level(example);
	/* [TODO] output syntax tree */
}
