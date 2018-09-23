#include "../search_graph.h"
#include "../search_tree.h"
#include "../syntax_tree.h"
#include "../incomplete_execution.h"
#include "nfa.hpp"
#include <iostream>
#include <vector>
#include <map>

enum RegexSyntaxLHSType {
	RE_ZERO = '0', 
	RE_ONE, 
	RE_a = 'a',
	RE_A = 'A',
	RE_STAR = 'z'+1, 
	RE_CLAUSE, 
	RE_REPEAT_CLAUSE,
	RE_CHAR, 
	RE_RE, 
	RE_DOT
};

enum RegexSyntaxRHSType {
	MU_ZERO = '0', 
	MU_ONE, 
	MU_a = 'a',
	MU_A = 'z',
	MU_CHAR = 'z'+1, 
	MU_CONCAT, 
	MU_DOT, 
	MU_STAR, 
	MU_STAR_UNFOLD, 
	MU_CLAUSE,
	MU_OR
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
	~RegexIEProgram() {
		delete nfa;
	}

	RegexIEProgram() {
		complete = false;
	}

	RegexIEProgram(bool complete0) {
		complete = complete0;
	}

	RegexIEProgram(NFA* nfa0, bool complete0) {
		complete = complete0;
		nfa = nfa0;
	}

	RegexIEProgram(RegexIEProgram* src) {
		complete = src->complete;
		nfa = new NFA(src->nfa);
//std::cout<<"RegexIEProgram source nfa:"<<nfa<<std::endl;
	}

	bool accept(IEExample* example) {
		RegexIEExample* re = (RegexIEExample*)example;
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
};


/* =================== Left Hand Side ================= */

class LHSABC : public IESyntaxLeftHandSide {
	public:
	LHSABC() {
		name = "\\C";
		is_term = false;
	}

	IEProgram* to_program() {
		NFA* nfa = new NFA();
		NFAState* op = new NFAState();
		std::set<NFAState*> op_trans;
		NFAState* ed = new NFAState();

		op_trans.insert(ed);
		for (char c = 'A'; c<='Z'; c++)
			op->transitions[c] = op_trans;
		nfa->states.insert(op);
		nfa->states.insert(ed);
		nfa->start_states.insert(op);
		nfa->accept_states.insert(ed);
		
		RegexIEProgram* p = new RegexIEProgram(nfa, false);
		return p;
	}
};

class LHSabc : public IESyntaxLeftHandSide {
	public:
	LHSabc() {
		name = "\\c";
		is_term = false;
	}

	IEProgram* to_program() {
		NFA* nfa = new NFA();
		NFAState* op = new NFAState();
		std::set<NFAState*> op_trans;
		NFAState* ed = new NFAState();

		op_trans.insert(ed);
		for (char c = 'a'; c<='z'; c++)
			op->transitions[c] = op_trans;
		nfa->states.insert(op);
		nfa->states.insert(ed);
		nfa->start_states.insert(op);
		nfa->accept_states.insert(ed);
		
		RegexIEProgram* p = new RegexIEProgram(nfa, false);
		return p;
	}
};

class LHS123 : public IESyntaxLeftHandSide {
	public:
	LHS123() {
		name = "\\d";
		is_term = false;
	}

	IEProgram* to_program() {
		NFA* nfa = new NFA();
		NFAState* op = new NFAState();
		std::set<NFAState*> op_trans;
		NFAState* ed = new NFAState();

		op_trans.insert(ed);
		for (char c = '0'; c<='9'; c++)
			op->transitions[c] = op_trans;
		nfa->states.insert(op);
		nfa->states.insert(ed);
		nfa->start_states.insert(op);
		nfa->accept_states.insert(ed);
		
		RegexIEProgram* p = new RegexIEProgram(nfa, false);
		return p;
	}
};

class LHSGeneralChar : public IESyntaxLeftHandSide {
	public:
	LHSGeneralChar(char ch0) {
		ch = ch0;
		name = "_";
		name[0] = ch;
		is_term = true;
	}

	IEProgram* to_program() {
		NFA* nfa = new NFA();
		NFAState* op = new NFAState();
		std::set<NFAState*> op_trans;
		NFAState* ed = new NFAState();

		op_trans.insert(ed);
		op->transitions[ch] = op_trans;
		nfa->states.insert(op);
		nfa->states.insert(ed);
		nfa->start_states.insert(op);
		nfa->accept_states.insert(ed);
		
		RegexIEProgram* p = new RegexIEProgram(nfa, true);
		return p;
	}

	private:
	char ch;
};

class LHSRepeatClause : public IESyntaxLeftHandSide {
	public:
	LHSRepeatClause()
	{
		name = "repeat_clause";
		is_term = false;
	}

	IEProgram* to_program() {
		NFA* nfa = new NFA();
		NFAState* zero = new NFAState();
		std::set<NFAState*> zero_trans;
		NFAState* op = new NFAState();
		std::set<NFAState*> op_trans;
		NFAState* ed = new NFAState();
		std::set<NFAState*> ed_trans;

		zero_trans.insert(op);
		zero->transitions[Epsilon] = zero_trans;
		op_trans.insert(ed);
		for (char c = '0'; c<='9'; c++)
			op->transitions[c] = op_trans;
		for (char c = 'A'; c<='Z'; c++)
			op->transitions[c] = op_trans;
		for (char c = 'a'; c<='z'; c++)
			op->transitions[c] = op_trans;
		ed_trans.insert(op);
		ed->transitions[Epsilon] = ed_trans;
		nfa->states.insert(zero);
		nfa->states.insert(op);
		nfa->states.insert(ed);
		nfa->start_states.insert(zero);
//		nfa->accept_states.insert(zero);
		nfa->accept_states.insert(ed);
		
		RegexIEProgram* p = new RegexIEProgram(nfa, false);
		return p;
	}
};


class LHSRe : public IESyntaxLeftHandSide {
	public:
	LHSRe() {
		name = "re";
		is_term = false;
	}

	IEProgram* to_program() {
		NFA* nfa = new NFA();
		NFAState* zero = new NFAState();
		std::set<NFAState*> zero_trans;
		NFAState* op = new NFAState();
		std::set<NFAState*> op_trans;
		NFAState* ed = new NFAState();
		std::set<NFAState*> ed_trans;

		zero_trans.insert(op);
		zero->transitions[Epsilon] = zero_trans;
		op_trans.insert(ed);
		for (char c = '0'; c<='9'; c++)
			op->transitions[c] = op_trans;
		for (char c = 'A'; c<='Z'; c++)
			op->transitions[c] = op_trans;
		for (char c = 'a'; c<='z'; c++)
			op->transitions[c] = op_trans;
		ed_trans.insert(op);
		ed->transitions[Epsilon] = ed_trans;
		nfa->states.insert(zero);
		nfa->states.insert(op);
		nfa->states.insert(ed);
		nfa->start_states.insert(zero);
//		nfa->accept_states.insert(zero);
		nfa->accept_states.insert(ed);
		
		RegexIEProgram* p = new RegexIEProgram(nfa, false);
		return p;
	}
};

class LHSCh : public IESyntaxLeftHandSide {
	public:
	LHSCh() {
		name = "ch";
		is_term = false;
	}

	IEProgram* to_program() {
		NFA* nfa = new NFA();
		NFAState* op = new NFAState();
		std::set<NFAState*> op_trans;
		NFAState* ed = new NFAState();

		op_trans.insert(ed);
		for (char c = '0'; c<='9'; c++)
			op->transitions[c] = op_trans;
		for (char c = 'A'; c<='Z'; c++)
			op->transitions[c] = op_trans;
		for (char c = 'a'; c<='z'; c++)
			op->transitions[c] = op_trans;
		nfa->states.insert(op);
		nfa->states.insert(ed);
		nfa->start_states.insert(op);
		nfa->accept_states.insert(ed);
		
		RegexIEProgram* p = new RegexIEProgram(nfa, false);
		return p;
	}
};

class LHSZero : public IESyntaxLeftHandSide {
	public:
	LHSZero() {
		name = "0";
		is_term = true;
	}

	IEProgram* to_program() {
		NFA* nfa = new NFA();
		NFAState* op = new NFAState();
		std::set<NFAState*> op_trans;
		NFAState* ed = new NFAState();

		op_trans.insert(ed);
		op->transitions['0'] = op_trans;
		nfa->states.insert(op);
		nfa->states.insert(ed);
		nfa->start_states.insert(op);
		nfa->accept_states.insert(ed);
		
		RegexIEProgram* p = new RegexIEProgram(nfa, true);
		return p;
	}
};

class LHSOne : public IESyntaxLeftHandSide {
	public:
	LHSOne() {
		name = "1";
		is_term = true;
	}

	IEProgram* to_program() {
		NFA* nfa = new NFA();
		NFAState* op = new NFAState();
		std::set<NFAState*> op_trans;
		NFAState* ed = new NFAState();

		op_trans.insert(ed);
		op->transitions['1'] = op_trans;
		nfa->states.insert(op);
		nfa->states.insert(ed);
		nfa->start_states.insert(op);
		nfa->accept_states.insert(ed);
		
		RegexIEProgram* p = new RegexIEProgram(nfa, true);
		return p;
	}
};

class LHSDot : public IESyntaxLeftHandSide {
	public:
	LHSDot() {
		name = "?";
		is_term = true;
	}

	IEProgram* to_program() {
		NFA* nfa = new NFA();
		NFAState* op = new NFAState();
		std::set<NFAState*> op_trans;
		NFAState* ed = new NFAState();

		op_trans.insert(ed);
		for (char c = '0'; c<='9'; c++)
			op->transitions[c] = op_trans;
		for (char c = 'A'; c<='Z'; c++)
			op->transitions[c] = op_trans;
		for (char c = 'a'; c<='z'; c++)
			op->transitions[c] = op_trans;
		nfa->states.insert(op);
		nfa->states.insert(ed);
		nfa->start_states.insert(op);
		nfa->accept_states.insert(ed);
		
		RegexIEProgram* p = new RegexIEProgram(nfa, true);
		return p;
	}
};

class LHSClause : public IESyntaxLeftHandSide {
	public:
	LHSClause() {
		name = "clause";
		is_term = false;
	}

	IEProgram* to_program() {
		NFA* nfa = new NFA();
		NFAState* zero = new NFAState();
		std::set<NFAState*> zero_trans;
		NFAState* op = new NFAState();
		std::set<NFAState*> op_trans;
		NFAState* ed = new NFAState();
		std::set<NFAState*> ed_trans;

		zero_trans.insert(op);
		zero->transitions[Epsilon] = zero_trans;
		op_trans.insert(ed);
		for (char c = '0'; c<='9'; c++)
			op->transitions[c] = op_trans;
		for (char c = 'A'; c<='Z'; c++)
			op->transitions[c] = op_trans;
		for (char c = 'a'; c<='z'; c++)
			op->transitions[c] = op_trans;
		ed_trans.insert(op);
		ed->transitions[Epsilon] = ed_trans;
		nfa->states.insert(zero);
		nfa->states.insert(op);
		nfa->states.insert(ed);
		nfa->start_states.insert(zero);
//		nfa->accept_states.insert(zero);
		nfa->accept_states.insert(ed);
		
		RegexIEProgram* p = new RegexIEProgram(nfa, false);
		return p;
	}
};

class LHSStar : public IESyntaxLeftHandSide {
	public:
	LHSStar() {
		name = "*";
		is_term = false;
	}

	IEProgram* to_program() {
		NFA* nfa = new NFA();
		NFAState* zero = new NFAState();
		std::set<NFAState*> zero_trans;
		NFAState* op = new NFAState();
		std::set<NFAState*> op_trans;
		NFAState* ed = new NFAState();
		std::set<NFAState*> ed_trans;

		zero_trans.insert(op);
		zero->transitions[Epsilon] = zero_trans;
		op_trans.insert(ed);
		for (char c = '0'; c<='9'; c++)
			op->transitions[c] = op_trans;
		for (char c = 'A'; c<='Z'; c++)
			op->transitions[c] = op_trans;
		for (char c = 'a'; c<='z'; c++)
			op->transitions[c] = op_trans;
		ed_trans.insert(op);
		ed->transitions[Epsilon] = ed_trans;
		nfa->states.insert(zero);
		nfa->states.insert(op);
		nfa->states.insert(ed);
		nfa->start_states.insert(zero);
//		nfa->accept_states.insert(zero);
		nfa->accept_states.insert(ed);
		
		RegexIEProgram* p = new RegexIEProgram(nfa, false);
		return p;
	}
};




/* =================== Right Hand Side ================= */

class RHSOr : public IESyntaxRightHandSide {
	public:
	RHSOr() {
		name = "||";
		independent = true;
	}

	IEProgram* combine_subprograms(std::vector<IEProgram*> subprograms) {
		NFA* root = new NFA();
		NFA* left = new NFA(((RegexIEProgram*)subprograms[0])->nfa);
		NFA* right = new NFA(((RegexIEProgram*)subprograms[1])->nfa);
		NFAState* zero = new NFAState();
		root->states.insert(left->states.begin(), left->states.end());
		root->states.insert(right->states.begin(), right->states.end());
		root->states.insert(zero);

		std::set<NFAState*> zero_trans;
		zero_trans.clear();
		zero_trans.insert(left->start_states.begin(), left->start_states.end());
		zero_trans.insert(right->start_states.begin(), right->start_states.end());
		zero->transitions[Epsilon] = zero_trans;
		root->start_states.clear();
		root->start_states.insert(zero);

		root->accept_states = right->accept_states;
		root->accept_states.insert(left->accept_states.begin(), left->accept_states.end());

		left->states.clear();
		right->states.clear();
		delete left;
		delete right;

		return new RegexIEProgram(root, ((RegexIEProgram*)subprograms[0])->complete && ((RegexIEProgram*)subprograms[1])->complete);
	}

	std::string to_string(std::vector<std::string> subprograms) {
		return "( " + subprograms[0] + " || " + subprograms[1] + " )";
	}
};

class RHSConcat : public IESyntaxRightHandSide {
	public:
	RHSConcat() {
		name = "concat";
		independent = true;
	}

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
		left->states.clear();
		right->states.clear();
		delete left;
		delete right;
		return new RegexIEProgram(root, ((RegexIEProgram*)subprograms[0])->complete && ((RegexIEProgram*)subprograms[1])->complete);
	}

	std::string to_string(std::vector<std::string> subprograms) {
		return subprograms[0] + " " + subprograms[1];
	}
};

class RHSCh : public IESyntaxRightHandSide {
	public:
	RHSCh() {
		name = "ch";
		independent = true;
	}

	IEProgram* combine_subprograms(std::vector<IEProgram*> subprograms) {
		return new RegexIEProgram((RegexIEProgram*)subprograms[0]);
	}

	std::string to_string(std::vector<std::string> subprograms) {
		return subprograms[0];
	}
};

class RHS123 : public IESyntaxRightHandSide {
	public:
	RHS123() {
		name = "[123]";
		independent = true;
	}

	IEProgram* combine_subprograms(std::vector<IEProgram*> subprograms) {
		return new RegexIEProgram((RegexIEProgram*)subprograms[0]);
	}

	std::string to_string(std::vector<std::string> subprograms) {
		return subprograms[0];
	}
};

class RHSABC : public IESyntaxRightHandSide {
	public:
	RHSABC() {
		name = "[ABC]";
		independent = true;
	}

	IEProgram* combine_subprograms(std::vector<IEProgram*> subprograms) {
		return new RegexIEProgram((RegexIEProgram*)subprograms[0]);
	}

	std::string to_string(std::vector<std::string> subprograms) {
		return subprograms[0];
	}
};

class RHSabc : public IESyntaxRightHandSide {
	public:
	RHSabc() {
		name = "[abc]";
		independent = true;
	}

	IEProgram* combine_subprograms(std::vector<IEProgram*> subprograms) {
		return new RegexIEProgram((RegexIEProgram*)subprograms[0]);
	}

	std::string to_string(std::vector<std::string> subprograms) {
		return subprograms[0];
	}
};

class RHSZero : public IESyntaxRightHandSide {
	public:
	RHSZero() {
		name = "0";
		independent = true;
	}

	IEProgram* combine_subprograms(std::vector<IEProgram*> subprograms) {
		return new RegexIEProgram((RegexIEProgram*)subprograms[0]);
	}

	std::string to_string(std::vector<std::string> subprograms) {
		return subprograms[0];
	}
};

class RHSOne : public IESyntaxRightHandSide {
	public:
	IEProgram* combine_subprograms(std::vector<IEProgram*> subprograms) {
		return new RegexIEProgram((RegexIEProgram*)subprograms[0]);
	}

	std::string to_string(std::vector<std::string> subprograms) {
		return subprograms[0];
	}
};

class RHSDot : public IESyntaxRightHandSide {
	public:
	RHSDot() {
		name = ".";
		independent = true;
	}

	IEProgram* combine_subprograms(std::vector<IEProgram*> subprograms) {
		return new RegexIEProgram((RegexIEProgram*)subprograms[0]);
	}

	std::string to_string(std::vector<std::string> subprograms) {
		return subprograms[0];
	}
};

class RHSStar : public IESyntaxRightHandSide {
	public:
	RHSStar() {
		name = "*";
		independent = true;
	}

	IEProgram* combine_subprograms(std::vector<IEProgram*> subprograms) {
		return new RegexIEProgram((RegexIEProgram*)subprograms[0]);
	}

	std::string to_string(std::vector<std::string> subprograms) {
		return subprograms[0];
	}
};

class RHSStarUnfold : public IESyntaxRightHandSide {
	public:
	RHSStarUnfold() {
		name = "*";
		independent = false;
	}

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
		root->accept_states.erase(zero); // disable zero matching
		root->start_states.clear();
		root->start_states.insert(zero);
		return new RegexIEProgram(root, ((RegexIEProgram*)subprograms[0])->complete);
	}

	std::string to_string(std::vector<std::string> subprograms) {
		return "( " + subprograms[0] + " )*";
	}
};

class RHSClause : public IESyntaxRightHandSide {
	public:
	RHSClause() {
		name = "clause";
		independent = true;
	}

	IEProgram* combine_subprograms(std::vector<IEProgram*> subprograms) {
		return new RegexIEProgram((RegexIEProgram*)subprograms[0]);
	}

	std::string to_string(std::vector<std::string> subprograms) {
		return subprograms[0];
	}
};


LHSRe* l_re;
LHSCh* l_ch;
LHSDot* l_dot;
LHSClause* l_clause;
LHSStar* l_star;
LHSRepeatClause* l_repeat_clause;
LHS123* l_123;
LHSABC* l_ABC;
LHSabc* l_abc;
std::vector<LHSGeneralChar*> ll_123;
std::vector<LHSGeneralChar*> ll_abc;
std::vector<LHSGeneralChar*> ll_ABC;

RHSConcat* r_concat;
RHSCh* r_ch;
RHSDot* r_dot;
RHSStar* r_star;
RHSStarUnfold* r_star_unfold;
RHSClause* r_clause;
RHSOr* r_or;
RHS123* r_123;
RHSABC* r_ABC;
RHSabc* r_abc;
std::vector<RHSCh*> rr_123;
std::vector<RHSCh*> rr_ABC;
std::vector<RHSCh*> rr_abc;

void init()
{
	l_re = new LHSRe();
	l_ch = new LHSCh();
	l_dot = new LHSDot();
	l_star = new LHSStar();
	l_clause = new LHSClause();
	l_repeat_clause = new LHSRepeatClause();
	l_123 = new LHS123();
	l_ABC = new LHSABC();
	l_abc = new LHSabc();

	for (char c = '0'; c<='9'; c++)
		ll_123.push_back(new LHSGeneralChar(c));
	ll_123.push_back(new LHSGeneralChar('.'));
	for (char c = 'A'; c<='Z'; c++)
		ll_ABC.push_back(new LHSGeneralChar(c));
	for (char c = 'a'; c<='z'; c++)
		ll_abc.push_back(new LHSGeneralChar(c));

	r_concat = new RHSConcat();
	r_ch = new RHSCh();
	r_dot = new RHSDot();
	r_star = new RHSStar();
	r_star_unfold = new RHSStarUnfold();
	r_clause = new RHSClause();
	r_or = new RHSOr();
	r_123 = new RHS123();
	r_ABC = new RHSABC();
	r_abc = new RHSabc();

	for (char c = '0'; c<='9'; c++)
		rr_123.push_back(new RHSCh());
	rr_123.push_back(new RHSCh());
	for (char c = 'A'; c<='Z'; c++)
		rr_ABC.push_back(new RHSCh());
	for (char c = 'a'; c<='z'; c++)
		rr_abc.push_back(new RHSCh());

	l_re->option.push_back(r_concat);
	l_re->option.push_back(r_clause);
//	l_re->option.push_back(r_or);
	l_clause->option.push_back(r_ch);
	l_clause->option.push_back(r_star);
	l_repeat_clause->option.push_back(r_ch);
	l_repeat_clause->option.push_back(r_concat);
	l_star->option.push_back(r_star_unfold);
	l_ch->option.push_back(r_dot);
	l_ch->option.push_back(r_123);
	for (int i=0; i<rr_123.size(); i++)
		l_123->option.push_back(rr_123[i]);
	l_ch->option.push_back(r_abc);
	for (int i=0; i<rr_ABC.size(); i++)
		l_ABC->option.push_back(rr_ABC[i]);
	l_ch->option.push_back(r_ABC);
	for (int i=0; i<rr_abc.size(); i++)
		l_abc->option.push_back(rr_abc[i]);

	r_concat->subexp.push_back(l_clause);
	r_concat->subexp.push_back(l_re);
//	r_or->subexp.push_back(l_clause);
//	r_or->subexp.push_back(l_re);
	r_or->subexp.push_back(l_ch);
	r_or->subexp.push_back(l_ch);
	r_ch->subexp.push_back(l_ch);
	r_dot->subexp.push_back(l_dot);
	r_clause->subexp.push_back(l_clause);
	r_star->subexp.push_back(l_star);
	r_star_unfold->subexp.push_back(l_repeat_clause);
	r_123->subexp.push_back(l_123);
	r_ABC->subexp.push_back(l_ABC);
	r_abc->subexp.push_back(l_abc);
	for (int i=0; i<rr_123.size(); i++)
		rr_123[i]->subexp.push_back(ll_123[i]);
	for (int i=0; i<rr_ABC.size(); i++)
		rr_ABC[i]->subexp.push_back(ll_ABC[i]);
	for (int i=0; i<rr_abc.size(); i++)
		rr_abc[i]->subexp.push_back(ll_abc[i]);
}

int main() {
	init();
	int d, b, a;
	std::cin>>d>>b>>a;
	SearchGraph g(d, b, a, l_re);
	std::vector<IEExample*> example;
	int positive_num, negative_num;
	std::cin>>positive_num;
	std::cin>>negative_num;
	for (int i=0; i<positive_num; i++)
	{
		std::string example_string;
		std::cin>>example_string;
		example.push_back(new RegexIEExample(O_YES, example_string));
	}
	for (int i=0; i<negative_num; i++)
	{
		std::string example_string;
		std::cin>>example_string;
		example.push_back(new RegexIEExample(O_NO, example_string));
	}
	std::vector<IESyntaxTree*> program = g.search_top_level_v2(example);
	std::cout<<"====================================================\n";
	std::cout<<"Positive Examples:\n";
	for (int i=0; i<positive_num; i++)
		std::cout<<((RegexIEExample*)(example[i]))->s<<std::endl;
	std::cout<<std::endl;
	std::cout<<"Negative Examples:\n";
	for (int i=0; i<negative_num; i++)
		std::cout<<((RegexIEExample*)(example[i+positive_num]))->s<<std::endl;
	std::cout<<std::endl;
	std::cout<<"Synthesized Programs:\n";
	if (program.size() == 0)
		std::cout<<"Not Found!\n";
	else
	{
		for (int i=0; i<program.size(); i++)
			std::cout<<program[i]->to_string()<<std::endl;
	}
}
