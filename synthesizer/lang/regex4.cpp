#include "../search_graph.h"
#include "../search_tree.h"
#include "../syntax_tree.h"
#include "nfa.hpp"
#include <iostream>
#include <vector>
#include <map>

enum RegexSyntaxLHSType {
	RE_ZERO = 0, RE_ONE, RE_STAR, RE_CLAUSE, RE_CHAR, RE_RE, RE_DOT
};

enum RegexSyntaxRHSType {
	MU_ZERO = 0, MU_ONE, MU_CHAR, MU_CONCAT, MU_DOT, MU_STAR, MU_STAR_UNFOLD, MU_CLAUSE
};

enum RegexOutputType {
	O_YES = 0, O_NO, O_WHATEVER
};

class RegexIEExample: public IEExample {
	public:
	RegexIEExample(RegexOutputType out0, std::string s0) {
		s = s0;
		out = out0;
	}

	bool is_positive() {
		return (out == O_YES);
	}

	RegexOutputType out;
	std::string s;
};



class RegexIEProgram: public IEProgram {
	public:
	RegexIEProgram() {
		complete = false;
	}

	RegexIEProgram(bool complete0) {
		complete = complete0;
	}

	RegexIEProgram(NFA* nfa, bool complete0) {
		complete = complete0;
	}

	RegexIEProgram(RegexIEProgram* src) {
		complete = src->complete;
		nfa = new NFA(src->nfa);
	}

	bool accept(IEExample* example) {
		RegexIEExample* re = (RegexIEExampl*)example;
		if (re->is_positive())
		{
			return nfa->accept(re->s);
		}
		else
		{
			if (complete)
				return !nfa->accept(re->s);
			else
				return true;
		}
	}

	bool complete;
	NFA* nfa;
}




class LHSRe : public IESyntaxLeftHandSide {
	public:
	IEProgram* to_program() {
		NFA* nfa = new NFA();
		NFAState* zero = new NFAState();
		std::set<NFAState*> zero_trans;
		NFAState* op = new NFAState();
		std::set<NFAState*> op_trans;
		NFAState* ed = new NFAState();
		std::set<NFAState*> ed_trans;

		zero_trans.insert(op);
		zero[Epsilon] = zero_trans;
		op_trans.insert(ed);
		op->transitions["0"] = op_trans;
		op->transitions["1"] = op_trans;
		ed_trans.insert(op);
		ed[Epsilon] = ed_trans;
		NFA->states.insert(zero);
		NFA->states.insert(op);
		NFA->states.insert(ed);
		NFA->start_states.insert(zero);
		NFA->accept_states.insert(zero);
		NFA->accept_states.insert(ed);
		
		RegexIEProgram* p = new RegexIEProgram(nfa, false);
		return p;
	}
};

class LHSCh : public IESyntaxLeftHandSide {
	public:
	IEProgram* to_program() {
		NFA* nfa = new NFA();
		NFAState* op = new NFAState();
		std::set<NFAState*> op_trans;
		NFAState* ed = new NFAState();

		op_trans.insert(ed);
		op->transitions["0"] = op_trans;
		op->transitions["1"] = op_trans;
		NFA->states.insert(op);
		NFA->states.insert(ed);
		NFA->start_states.insert(op);
		NFA->accept_states.insert(ed);
		
		IEProgram* p = new IEProgram(nfa, false);
		return p;
	}
};

class LHSZero : public IESyntaxLeftHandSide {
	public:
	IEProgram* to_program() {
		NFA* nfa = new NFA();
		NFAState* op = new NFAState();
		std::set<NFAState*> op_trans;
		NFAState* ed = new NFAState();

		op_trans.insert(ed);
		op->transitions["0"] = op_trans;
		NFA->states.insert(op);
		NFA->states.insert(ed);
		NFA->start_states.insert(op);
		NFA->accept_states.insert(ed);
		
		RegexIEProgram* p = new RegexIEProgram(nfa, true);
		return p;
	}
};

class LHSOne : public IESyntaxLeftHandSide {
	public:
	IEProgram* to_program() {
		NFA* nfa = new NFA();
		NFAState* op = new NFAState();
		std::set<NFAState*> op_trans;
		NFAState* ed = new NFAState();

		op_trans.insert(ed);
		op->transitions["1"] = op_trans;
		NFA->states.insert(op);
		NFA->states.insert(ed);
		NFA->start_states.insert(op);
		NFA->accept_states.insert(ed);
		
		RegexIEProgram* p = new RegexIEProgram(nfa, true);
		return p;
	}
};

class LHSDot : public IESyntaxLeftHandSide {
	public:
	IEProgram* to_program() {
		NFA* nfa = new NFA();
		NFAState* op = new NFAState();
		std::set<NFAState*> op_trans;
		NFAState* ed = new NFAState();

		op_trans.insert(ed);
		op->transitions["0"] = op_trans;
		op->transitions["1"] = op_trans;
		NFA->states.insert(op);
		NFA->states.insert(ed);
		NFA->start_states.insert(op);
		NFA->accept_states.insert(ed);
		
		RegexIEProgram* p = new RegexIEProgram(nfa, true);
		return p;
	}
};

class LHSClause : public IESyntaxLeftHandSide {
	public:
	IEProgram* to_program() {
		NFA* nfa = new NFA();
		NFAState* zero = new NFAState();
		std::set<NFAState*> zero_trans;
		NFAState* op = new NFAState();
		std::set<NFAState*> op_trans;
		NFAState* ed = new NFAState();
		std::set<NFAState*> ed_trans;

		zero_trans.insert(op);
		zero[Epsilon] = zero_trans;
		op_trans.insert(ed);
		op->transitions["0"] = op_trans;
		op->transitions["1"] = op_trans;
		ed_trans.insert(op);
		ed[Epsilon] = ed_trans;
		NFA->states.insert(zero);
		NFA->states.insert(op);
		NFA->states.insert(ed);
		NFA->start_states.insert(zero);
		NFA->accept_states.insert(zero);
		NFA->accept_states.insert(ed);
		
		RegexIEProgram* p = new RegexIEProgram(nfa, false);
		return p;
	}
};

class LHSStar : public IESyntaxLeftHandSide {
	public:
	IEProgram* to_program() {
		NFA* nfa = new NFA();
		NFAState* zero = new NFAState();
		std::set<NFAState*> zero_trans;
		NFAState* op = new NFAState();
		std::set<NFAState*> op_trans;
		NFAState* ed = new NFAState();
		std::set<NFAState*> ed_trans;

		zero_trans.insert(op);
		zero[Epsilon] = zero_trans;
		op_trans.insert(ed);
		op->transitions["0"] = op_trans;
		op->transitions["1"] = op_trans;
		ed_trans.insert(op);
		ed[Epsilon] = ed_trans;
		NFA->states.insert(zero);
		NFA->states.insert(op);
		NFA->states.insert(ed);
		NFA->start_states.insert(zero);
		NFA->accept_states.insert(zero);
		NFA->accept_states.insert(ed);
		
		RegexIEProgram* p = new RegexIEProgram(nfa, false);
		return p;
	}
};



/* [TODO] no RegexIEProgram or NFA or NFAState released; memory leak */
class RHSConcat : public IESyntaxRightHandSide {
	public:
	IEProgram* combine_subprograms(std::vector<IEProgram*> subprograms) {
		NFA* root = new NFA();
		NFA* left = new NFA(((RegexIEProgram*)subprograms[0])->nfa);
		NFA* right = new NFA(((RegexIEProgram*)subprograms[1])->nfa);
		root->states.insert(left->states.begin(), left->states.end());
		root->states.insert(right->states.begin(), right->states.end());
		root->start_states = left->start_states;
		root->accept_states = right->accept_states;
		for (NFAIt i = left->accept_states.begin(); i!= left->accept_states.end(); i++)
		{
			std::set<NFAState*> e_trans = (*i)->transitions[Epsilon];
			e_trans.insert(right->start_states.begin(), right->start_states.end());
			(*i)->transitions[Epsilon] = e_trans;
		}
		left->states = std::set<NFAState*>(); /* to avoid double free */
		right->states = std::set<NFAState*>(); /* to avoid double free */
		delete left;
		delete right;
		return new RegexIEProgram(root, ((RegexIEProgram*)subprograms[0])->complete && ((RegexIEProgram*)subprograms[1])->complete);
	}
};

class RHSCh : public IESyntaxRightHandSide {
	public:
	IEProgram* combine_subprograms(std::vector<IEProgram*> subprograms) {
		return new RegexIEProgram(subprograms[0]);
	}
};

class RHSZero : public IESyntaxRightHandSide {
	public:
	IEProgram* combine_subprograms(std::vector<IEProgram*> subprograms) {
		return new RegexIEProgram(subprograms[0]);
	}
};

class RHSOne : public IESyntaxRightHandSide {
	public:
	IEProgram* combine_subprograms(std::vector<IEProgram*> subprograms) {
		return new RegexIEProgram(subprograms[0]);
	}
};

class RHSDot : public IESyntaxRightHandSide {
	public:
	IEProgram* combine_subprograms(std::vector<IEProgram*> subprograms) {
		return new RegexIEProgram(subprograms[0]);
	}
};

class RHSStar : public IESyntaxRightHandSide {
	public:
	IEProgram* combine_subprograms(std::vector<IEProgram*> subprograms) {
		return new RegexIEProgram(subprograms[0]);
	}
};

class RHSStarUnfold : public IESyntaxRightHandSide {
	public:
	IEProgram* combine_subprograms(std::vector<IEProgram*> subprograms) {
		NFA* child = ((RegexIEProgram*)subprograms[0])->nfa;
		NFA* root = new NFA(child);
		NFAState* zero = new NFAState();
		root->states.insert(zero);
		root->accept_states.insert(zero);
		for (NFAIt i = root->accept_states.begin(); i!=root->accept_states.end(); i++)
		{
			std::set<NFAState*> e_trans = (*i)->transitions[Epsilon];
			e_trans.insert(root->start_states.begin(), root->start_states.end());
			(*i)->transitions[Epsilon] = e_trans;
		}
		return new RegexIEProgram(root, ((RegexIEProgram*)subprograms[0])->complete);
	}
};

class RHSClause : public IESyntaxRightHandSide {
	public:
	IEProgram* combine_subprograms(std::vector<IEProgram*> subprograms) {
		return RegexIEProgram(subprograms[0]);
	}
};


LHSRe* l_re;
LHSCh* l_ch;
LHSZero* l_zero;
LHSOne* l_one;
LHSDot* l_dot;
LHSClause* l_clause;
LHSStar* l_star;

RHSConcat* r_concat;
RHSCh* r_ch;
RHSZero* r_zero;
RHSOne* r_one;
RHSDot* r_dot;
RHSStar* r_star;
RHSStarUnfold* r_star_unfold;
RHSClause* r_clause;

void init()
{
	l_re = new SyntaxLeftHandSide;
	l_ch = new SyntaxLeftHandSide();
	l_zero = new SyntaxLeftHandSide();
	l_one = new SyntaxLeftHandSide();
	l_dot = new SyntaxLeftHandSide();
	l_star = new SyntaxLeftHandSide();
	l_clause = new SyntaxLeftHandSide();

	r_concat = new SyntaxRightHandSide();
	r_ch = new SyntaxRightHandSide();
	r_zero = new SyntaxRightHandSide();
	r_one = new SyntaxRightHandSide();
	r_dot = new SyntaxRightHandSide();
	r_star = new SyntaxRightHandSide();
	r_star_unfold = new SyntaxRightHandSide();
	r_clause = new SyntaxRightHandSide();

	l_re->id = RE_RE;
	l_ch->id = RE_CHAR;
	l_zero->id = RE_ZERO;
	l_one->id = RE_ONE;
	l_dot->id = RE_DOT;
	l_clause->id = RE_CLAUSE;
	l_star->id = RE_STAR;

	l_re->name = "re";
	l_ch->name = "char";
	l_zero->name = "0";
	l_one->name = "1";
	l_dot->name = ".";
	l_clause->name = "clause";
	l_star->name = "*";

	l_re->is_term = false;
	l_ch->is_term = false;
	l_zero->is_term = true;
	l_one->is_term = true;
	l_dot->is_term = true;
	l_clause->is_term = false;
	l_star->is_term = false;

	r_concat->independent = true;
	r_ch->independent = true;
	r_zero->independent = true;
	r_one->independent = true;
	r_dot->independent = true;
	r_star->independent = true;
	r_star_unfold->independent = false;
	r_clause->independent = true;

	r_concat->id = MU_CONCAT;
	r_ch->id = MU_CHAR;
	r_zero->id = MU_ZERO;
	r_one->id = MU_ONE;
	r_dot->id = MU_DOT;
	r_star->id = MU_STAR;
	r_star_unfold->id = MU_STAR_UNFOLD;
	r_clause->id = MU_CLAUSE;

	r_concat->name = "concat";
	r_ch->name = "char";
	r_zero->name = "0";
	r_one->name = "1";
	r_dot->name = ".";
	r_star->name = "*";
	r_star_unfold->name = "*";
	r_clause->name = "clause";

	l_re->option.push_back(r_concat);
	l_re->option.push_back(r_clause);
//	l_ch->option.push_back(r_dot);
	l_ch->option.push_back(r_zero);
	l_ch->option.push_back(r_one);
	l_clause->option.push_back(r_ch);
	l_clause->option.push_back(r_star);
	l_star->option.push_back(r_star_unfold);

	r_concat->subexp.push_back(l_clause);
	r_concat->subexp.push_back(l_re);
	r_ch->subexp.push_back(l_ch);
	r_zero->subexp.push_back(l_zero);
	r_one->subexp.push_back(l_one);
	r_dot->subexp.push_back(l_dot);
	r_clause->subexp.push_back(l_clause);
	r_star->subexp.push_back(l_star);
	r_star_unfold->subexp.push_back(l_re);

}

int main() {
	init();
	int d;
	std::cin>>d;
	SearchGraph g(d, l_re);
	std::vector<IEExample*> example;
	int positive_num, negative_num;
	std::cin>>positive_num;
	for (int i=0; i<positive_num; i++)
	{
		std::string example_string;
		std::cin>>example_string;
		example.push_back(new RegexIEExample(O_YES, example_string));
	}
	std::cin>>negative_num;
	for (int i=0; i<negative_num; i++)
	{
		std::string example_string;
		std::cin>>example_string;
		example.push_back(new RegexIEExample(O_NO, example_string));
	}
	SyntaxTree* program = g.search_top_level_v2(example);
	if (program == nullptr)
		std::cout<<"Not Found!\n";
	else
		std::cout<<program->to_string()<<std::endl;
}
