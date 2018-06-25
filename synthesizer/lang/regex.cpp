#include "../search_graph.h"
#include "../search_tree.h"
#include "../syntax_tree.h"
#include <iostream>
#include <vector>
#include <map>

enum RegexSyntaxLHSType {
	RE_ZERO = 0, RE_ONE, RE_CHAR, RE_RE
};

enum RegexSyntaxRHSType {
	MU_ZERO = 0, MU_ONE, MU_CHAR, MU_CONCAT
};

class RegexSearchState: public SearchState {
	public:
	int l,r;
	RegexSyntaxLHSType type;

	std::vector<int> to_vector() {
		std::vector<int> ans;
		ans.push_back(l);
		ans.push_back(r);
		ans.push_back(type);
		return ans;
	}

	void print_state() {
		std::cout<<"State: "<<l<<" "<<r<<" "<<type<<std::endl;
	}
};

/*
bool operator < (const RegexSearchState& s1, const RegexSearchState& s2) {
	bool flag;
	if (s2.l > s1.l)
		flag =  true;
	else if (s2.l > s1.l)
		flag =  false;
	else if (s2.r > s1.r)
		flag =  true;
	else if (s2.r < s1.r)
		flag =  false;
	else if (s2.type > s1.type)
		flag =  true;
	else if (s2.type < s1.type)
		flag =  false;
	else
		flag =  false;
	std::cout<<"============cmp=================\n";
	std::cout<<"Player1: "<<s1.l<<" "<<s1.r<<" "<<s1.type<<"\n";
	std::cout<<"Player2: "<<s2.l<<" "<<s2.r<<" "<<s2.type<<"\n";
	std::cout<<"result: "<<flag<<std::endl;
	std::cout<<"================================\n";
	return flag;
}
*/


class RegexExampleType: public ExampleType {
	public:
	RegexExampleType(std::string s0) {
		s = s0;
	}

	SearchState* to_init_state() {
		RegexSearchState* rstate = new RegexSearchState();
		rstate->l = 0;
		rstate->r = s.length();
		rstate->type = RE_RE;
	}


	bool match(SearchState* state, SyntaxLeftHandSide* terminal) {
		RegexSearchState* rstate = (RegexSearchState*) state;
		int r = rstate->r;
		int l = rstate->l;
		if (r-l>1)
			return false;
		if ((terminal->id == RE_ZERO) && (s[l] =='0'))
			return true;
		if ((terminal->id == RE_ONE) && (s[l] =='1'))
			return true;
		return false;
	}

	std::string s;
};

template<class T>
class RegexSearchTreeCache: public SearchTreeCache<T> {
	public:
	T& operator [](SearchState* state) {
		RegexSearchState* rstate = (RegexSearchState*) state;
		return m[rstate->to_vector()];
	}

	int count(SearchState* state) {
		RegexSearchState* rstate = (RegexSearchState*) state;
		return m.count(rstate->to_vector());
	}

	std::map<std::vector<int>,T> m;
};

template<class T>
class RegexSearchTreeCacheFactory: public SearchTreeCacheFactory<T> {
	public:
	SearchTreeCache<T>* get_cache() {
		return new RegexSearchTreeCache<T>();
	}
};

/* one for each right hand side option */
class RegexConcatDivideStrategy: public DivideStrategy {
	public:

	bool valid_state(SearchState* state) {
		RegexSearchState* rstate = (RegexSearchState*) state;
		if (rstate->l < 0)
			return false;
		if (rstate->r - rstate->l <2)
			return false;
		return true;
	}

	std::vector< std::vector<SearchState*> > get_indep_substates(SearchState* state) {
		RegexSearchState* rstate = (RegexSearchState*) state;
		std::vector< std::vector<SearchState*> > ans;
		for (int i=rstate->l+1; i<rstate->r; i++)
		{
			std::vector<SearchState*> div;
			RegexSearchState* left = new RegexSearchState;
			RegexSearchState* right = new RegexSearchState;
			left->l = rstate->l;
			left->r = i;
			left->type = RE_RE;
			right->l = i;
			right->r = rstate->r;
			right->type = RE_RE;
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
		return nullptr;
	}

	bool valid_combination(std::vector<bool> valid_subexp) {
		for (int i=0; i<valid_subexp.size(); i++)
			if (!valid_subexp[i])
				return false;
		return true;
	}
};

class RegexCharDivideStrategy : public DivideStrategy {
	public:

	bool valid_state(SearchState* state) {
		RegexSearchState* rstate = (RegexSearchState*) state;
		if (rstate->l < 0)
			return false;
		if (rstate->r - rstate->l != 1)
			return false;
		return true;
	}

	std::vector< std::vector<SearchState*> > get_indep_substates(SearchState* state) {
		std::vector< std::vector<SearchState*> > ans;
		std::vector<SearchState*> ans_inner;
		RegexSearchState* rstate = (RegexSearchState*) state;
		RegexSearchState* ans_final = new RegexSearchState();
		ans_final->l = rstate->l;
		ans_final->r = rstate->r;
		ans_final->type = RE_CHAR;
		ans_inner.push_back(ans_final);
		ans.push_back(ans_inner);
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
		return nullptr;
	}

	bool valid_combination(std::vector<bool> valid_subexp) {
		for (int i=0; i<valid_subexp.size(); i++)
			if (!valid_subexp[i])
				return false;
		return true;
	}
};

class RegexZeroDivideStrategy : public DivideStrategy {
	public:

	bool valid_state(SearchState* state) {
		RegexSearchState* rstate = (RegexSearchState*) state;
		if (rstate->l < 0)
			return false;
		if (rstate->r - rstate->l != 1)
			return false;
		return true;
	}

	std::vector< std::vector<SearchState*> > get_indep_substates(SearchState* state) {
		std::vector< std::vector<SearchState*> > ans;
		std::vector<SearchState*> ans_inner;
		RegexSearchState* rstate = (RegexSearchState*) state;
		RegexSearchState* ans_final = new RegexSearchState();
		ans_final->l = rstate->l;
		ans_final->r = rstate->r;
		ans_final->type = RE_ZERO;
		ans_inner.push_back(ans_final);
		ans.push_back(ans_inner);
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
		return nullptr;
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

	bool valid_state(SearchState* state) {
		RegexSearchState* rstate = (RegexSearchState*) state;
		if (rstate->l < 0)
			return false;
		if (rstate->r - rstate->l != 1)
			return false;
		return true;
	}

	std::vector< std::vector<SearchState*> > get_indep_substates(SearchState* state) {
		std::vector< std::vector<SearchState*> > ans;
		std::vector<SearchState*> ans_inner;
		RegexSearchState* rstate = (RegexSearchState*) state;
		RegexSearchState* ans_final = new RegexSearchState();
		ans_final->l = rstate->l;
		ans_final->r = rstate->r;
		ans_final->type = RE_ONE;
		ans_inner.push_back(ans_final);
		ans.push_back(ans_inner);
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
		return nullptr;
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

RegexSearchTreeCacheFactory<LNode*>* cache_pool;

void init()
{
	l_re = new SyntaxLeftHandSide;
	l_ch = new SyntaxLeftHandSide();
	l_zero = new SyntaxLeftHandSide();
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

	l_re->id = RE_RE;
	l_ch->id = RE_CHAR;
	l_zero->id = RE_ZERO;
	l_one->id = RE_ONE;

	l_re->name = "re";
	l_ch->name = "char";
	l_zero->name = "0";
	l_one->name = "1";

	l_re->is_term = false;
	l_ch->is_term = false;
	l_zero->is_term = true;
	l_one->is_term = true;

	r_concat->independent = true;
	r_ch->independent = true;
	r_zero->independent = true;
	r_one->independent = true;

	r_concat->id = MU_CONCAT;
	r_ch->id = MU_CHAR;
	r_zero->id = MU_ZERO;
	r_one->id = MU_ONE;

	r_concat->name = "concat";
	r_ch->name = "char";
	r_zero->name = "0";
	r_one->name = "1";

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

	cache_pool = new RegexSearchTreeCacheFactory<LNode*>();
}

int main()
{
	std::string example_string;
	init();
	SearchGraph g(10, l_re, r2d,  cache_pool );
	std::cin>>example_string;
	std::vector<ExampleType*> example;
	example.push_back(new RegexExampleType(example_string));
	SyntaxTree* program = g.search_top_level(example);
	/* [TODO] output syntax tree */
	std::cout<<program<<std::endl;
}
