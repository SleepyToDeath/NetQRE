#include "../search_graph.h"
#include "../search_tree.h"
#include "../syntax_tree.h"
#include <iostream>
#include <vector>
#include <map>

class RegexExampleType: public ExampleType {
	std::string s;
};

class RegexSyntaxType {
	ZERO, ONE, CHAR, RE
};

class RegexSearchState: public SearchState {
	public:
	int l,r;
	RegexSyntaxType type;
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


class RegexOneDivideStrategy : public DivideStrategy {
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

void init()
{
	


}

