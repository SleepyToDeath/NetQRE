#include "../search_graph.h"
#include "../search_tree.h"
#include "../syntax_tree.h"
#include "../incomplete_execution.h"
#include "nfa_skip.hpp"
#include <iostream>
#include <vector>
#include <map>

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

	std::vector<bool> can_skip;
	RegexOutputType out;
	std::string s;
};

enum PredicateType {
	P_123, P_ABC, P_abc, P_CH, P_CONJUNCTION, P_DISJUNCTION, P_PREDICATE
};

enum PredicateMatchType {
	M_TRUE, M_FALSE, M_UNKNOWN
};

class RegexPredicateIEProgram: public IEProgram {
	public:

	RegexPredicateIEProgram(RegexPredicateIEProgram* src) {
		ch = src->ch;
		type = src->type;
		l = src->l;
		r = src->r;
	}

	RegexPredicateIEProgram(PredicateType type0) {
		type = type0;
	}

	RegexPredicateIEProgram(PredicateType type0, char ch0) {
		type = type0;
		ch = ch0;
	}

	RegexPredicateIEProgram(PredicateType type0, RegexPredicateIEProgram* l0, RegexPredicateIEProgram* r0) {
		type = type0;
		l = l0;
		r = r0;
	}

	std::vector<PredicateMatchType> match(std::string s) {
		size_t len = s.length();
		switch (type) {
			case P_PREDICATE:
				return std::vector<PredicateMatchType>(len, M_UNKNOWN);
			case P_123:
			{
				std::vector<PredicateMatchType> m(len, M_UNKNOWN);
				for (int i=0; i<len; i++)
					if (s[i]>='0' && s[i]<='9')
						m[i] = M_TRUE;
					else
						m[i] = M_FALSE;
				return m;
			}
			case P_ABC:
			{
				std::vector<PredicateMatchType> m(len, M_UNKNOWN);
				for (int i=0; i<len; i++)
					if (s[i]>='A' && s[i]<='Z')
						m[i] = M_TRUE;
					else
						m[i] = M_FALSE;
				return m;
			}
			case P_abc:
			{
				std::vector<PredicateMatchType> m(len, M_UNKNOWN);
				for (int i=0; i<len; i++)
					if (s[i]>='a' && s[i]<='z')
						m[i] = M_TRUE;
					else
						m[i] = M_FALSE;
				return m;
			}
			case P_CH:
			{
				std::vector<PredicateMatchType> m(len, M_UNKNOWN);
				for (int i=0; i<len; i++)
					if (s[i] == ch)
						m[i] = M_TRUE;
					else
						m[i] = M_FALSE;
				return m;
			}
			case P_CONJUNCTION:
			{
				std::vector<PredicateMatchType> m(len, M_UNKNOWN);
				std::vector<PredicateMatchType> ml = l->match(s);
				std::vector<PredicateMatchType> mr = r->match(s);
				for (int i=0; i<len; i++)
					if (ml[i] == M_UNKNOWN || mr[i] == M_UNKNOWN)
					{
						if (ml[i] == M_FALSE || mr[i] == M_FALSE)
							m[i] = M_FALSE;
						else
							m[i] = M_UNKNOWN;
					}
					else
					{
						if ((ml[i] == M_TRUE) && (mr[i] == M_TRUE))
							m[i] = M_TRUE;
						else
							m[i] = M_FALSE;
					}
				return m;
			}
			case P_DISJUNCTION:
			{
				std::vector<PredicateMatchType> m(len, M_UNKNOWN);
				std::vector<PredicateMatchType> ml = l->match(s);
				std::vector<PredicateMatchType> mr = r->match(s);
				for (int i=0; i<len; i++)
					if (ml[i] == M_UNKNOWN || mr[i] == M_UNKNOWN)
					{
						if (ml[i] == M_TRUE || mr[i] == M_TRUE)
							m[i] = M_TRUE;
						else
							m[i] = M_UNKNOWN;
					}
					else
					{
						if ((ml[i] == M_TRUE) || (mr[i] == M_TRUE))
							m[i] = M_TRUE;
						else
							m[i] = M_FALSE;
					}
				return m;
			}
			break;
		}
		std::cout<<"[ERROR] unknown predicate type "<<type<<" !\n";
		return std::vector<PredicateMatchType>(len, M_UNKNOWN);
	}

	bool accept(IEExample* example) {
		return true;
	}

	char ch;
	PredicateType type;
	RegexPredicateIEProgram* l;
	RegexPredicateIEProgram* r;
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

	RegexIEProgram(NFASkip* nfa0, bool complete0) {
		complete = complete0;
		nfa = nfa0;
	}

	RegexIEProgram(RegexIEProgram* src) {
		complete = src->complete;
		nfa = new NFASkip(src->nfa);
//std::cout<<"RegexIEProgram source nfa:"<<nfa<<std::endl;
	}

	bool accept(IEExample* example) {
		RegexIEExample* ree = (RegexIEExample*)example;
		return nfa->accept(ree->s, ree->can_skip);
	}

	bool complete;
	NFASkip* nfa;
};

class RegexTopIEProgram: public IEProgram {
	public:
	RegexTopIEProgram(IEProgram* re0, IEProgram* pred0) {
		re = (RegexIEProgram*) re0;
		pred = (RegexPredicateIEProgram*) pred0;
	}

	bool accept(IEExample* e) {
		RegexIEExample* ree = (RegexIEExample*) e;
		if ((!ree->is_positive()) && (!re->complete))
			return true;
		else
		{
			std::string s0 = ree->s;
			std::vector<PredicateMatchType> m0 = pred->match(s0);
			std::string s;
			std::vector<bool> m;
			for (int i=0; i<m0.size(); i++)
				if (m0[i] != M_FALSE)
				{
					s += s0[i];
					m.push_back(m0[i] == M_UNKNOWN);
				}
			RegexIEExample etmp(ree->out, s);
			etmp.can_skip = m;
			if (ree->is_positive()) {
				return re->accept(&etmp);
			}
			else
			{
				return !re->accept(&etmp);
			}
		}
	}

	RegexIEProgram* re;
	RegexPredicateIEProgram* pred;
};



/* =================== Left Hand Side ================= */

/*
	<ReWithFilter> :: <Filter> <Re> (ReWithFilter)
	<Filter> :: <Predicate> (FilterToPredicate)
				|| <Filter> & <Filter> (Conjunction)
				|| <Filter> | <Filter> (Disjunction)
	<Predicate> :: <Predicate123> (PredicateTo123)
				|| <Predicateabc> (PredicateToabc)
				|| <PredicateABC> (PredicateToABC)
				|| PredicateGeneralChar (PredicateToChar)
	<PredicateGeneralChar> :: char.....

	<Re> :: .......
*/

class LHSPredicateCh : public IESyntaxLeftHandSide {
	public:
	LHSPredicateCh(char c0) {
//		name = "\\?";
		name = ".";
		name[0] = c0;
		if (c0 == P_123)
			name = "\\n";
		if (c0 == P_ABC)
			name = "\\C";
		if (c0 == P_abc)
			name = "\\c";
		is_term = true;
		c = c0;
	}

	IEProgram* to_program() {
		RegexPredicateIEProgram* p;
		if (c > P_abc)
			p = new RegexPredicateIEProgram(P_CH, c);
		else
			p = new RegexPredicateIEProgram((PredicateType)c);
					
		return p;
	}

	char c;
};

class LHSPredicateABC : public IESyntaxLeftHandSide {
	public:
	LHSPredicateABC() {
		name = "\\C";
		is_term = false;
	}

	IEProgram* to_program() {
		RegexPredicateIEProgram* p = new RegexPredicateIEProgram(P_ABC);
		return p;
	}
};

class LHSPredicateabc : public IESyntaxLeftHandSide {
	public:
	LHSPredicateabc() {
		name = "\\c";
		is_term = false;
	}

	IEProgram* to_program() {
		RegexPredicateIEProgram* p = new RegexPredicateIEProgram(P_abc);
		return p;
	}
};

class LHSPredicate123 : public IESyntaxLeftHandSide {
	public:
	LHSPredicate123() {
		name = "\\n";
		is_term = false;
	}

	IEProgram* to_program() {
		RegexPredicateIEProgram* p = new RegexPredicateIEProgram(P_123);
		return p;
	}
};

class LHSFilter : public IESyntaxLeftHandSide {
	public:
	LHSFilter() {
		name = "filter";
		is_term = false;
	}

	IEProgram* to_program() {
		RegexPredicateIEProgram* p = new RegexPredicateIEProgram(P_PREDICATE);
		return p;
	}

};

class LHSPredicate : public IESyntaxLeftHandSide {
	public:
	LHSPredicate() {
		name = "Predicate";
		is_term = false;
	}

	IEProgram* to_program() {
		RegexPredicateIEProgram* p = new RegexPredicateIEProgram(P_PREDICATE);
		return p;
	}

};

class LHSABC : public IESyntaxLeftHandSide {
	public:
	LHSABC() {
		name = "\\C";
		is_term = false;
	}

	IEProgram* to_program() {
		NFASkip* nfa = new NFASkip();
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
		NFASkip* nfa = new NFASkip();
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
		NFASkip* nfa = new NFASkip();
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
		NFASkip* nfa = new NFASkip();
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
		NFASkip* nfa = new NFASkip();
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
		NFASkip* nfa = new NFASkip();
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
		NFASkip* nfa = new NFASkip();
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
		NFASkip* nfa = new NFASkip();
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
		NFASkip* nfa = new NFASkip();
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
		NFASkip* nfa = new NFASkip();
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
		NFASkip* nfa = new NFASkip();
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
		NFASkip* nfa = new NFASkip();
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

class LHSReWithFilter : public IESyntaxLeftHandSide {
	public:
	LHSReWithFilter() {
		name = "re_with_filter";
		is_term = false;
	}

	IEProgram* to_program() {
		LHSRe re;
		LHSFilter filter;
		RegexTopIEProgram* p = new RegexTopIEProgram(re.to_program(), filter.to_program());
		return p;
	}
};


/* =================== Right Hand Side ================= */

/*
	<ReWithFilter> :: <Filter> <Re> (ReWithFilter)
	<Filter> :: <Predicate> (FilterToPredicate)
				|| <Filter> & <Filter> (Conjunction)
				|| <Filter> | <Filter> (Disjunction)
	<Predicate> :: <Predicate123> (PredicateTo123)
				|| <Predicateabc> (PredicateToabc)
				|| <PredicateABC> (PredicateToABC)
	<Predicate123> :: PredicateCh (Predicate123ToCh)
	<Predicateabc> :: PredicateCh (PredicateabcToCh)
	<PredicateABC> :: PredicateCh (PredicateABCToCh)

	<Re> :: .......
*/

class RHSReWithFilter : public IESyntaxRightHandSide {
	public:
	RHSReWithFilter() {
		name = "re_with_filter";
		independent = true;
	}

	IEProgram* combine_subprograms(std::vector<IEProgram*> subprograms) {
		return new RegexTopIEProgram(subprograms[1], subprograms[0]);
	}

	std::string to_string(std::vector<std::string> subprograms) {
		return "filter( " + subprograms[0] + " ) >> " + subprograms[1];
	}

};

class RHSPredicateTo123 : public IESyntaxRightHandSide {
	public:
	RHSPredicateTo123() {
		name = "p2123";
		independent = true;
	}

	IEProgram* combine_subprograms(std::vector<IEProgram*> subprograms) {
		return subprograms[0];
	}

	std::string to_string(std::vector<std::string> subprograms) {
		return subprograms[0];
	}

};


class RHSPredicateToabc : public IESyntaxRightHandSide {
	public:
	RHSPredicateToabc() {
		name = "p2abc";
		independent = true;
	}

	IEProgram* combine_subprograms(std::vector<IEProgram*> subprograms) {
		return subprograms[0];
	}

	std::string to_string(std::vector<std::string> subprograms) {
		return subprograms[0];
	}

};


class RHSPredicateToABC : public IESyntaxRightHandSide {
	public:
	RHSPredicateToABC() {
		name = "p2ABC";
		independent = true;
	}

	IEProgram* combine_subprograms(std::vector<IEProgram*> subprograms) {
		return subprograms[0];
	}

	std::string to_string(std::vector<std::string> subprograms) {
		return subprograms[0];
	}

};


class RHSFilterToPredicate : public IESyntaxRightHandSide {
	public:
	RHSFilterToPredicate() {
		name = "f2p";
		independent = true;
	}

	IEProgram* combine_subprograms(std::vector<IEProgram*> subprograms) {
		return subprograms[0];
	}

	std::string to_string(std::vector<std::string> subprograms) {
		return subprograms[0];
	}

};

class RHSConjunction : public IESyntaxRightHandSide {
	public:
	RHSConjunction() {
		name = "conjunction";
		independent = false;
	}

	IEProgram* combine_subprograms(std::vector<IEProgram*> subprograms) {
		return new RegexPredicateIEProgram(P_CONJUNCTION, 
								(RegexPredicateIEProgram*) subprograms[0],
								(RegexPredicateIEProgram*) subprograms[1] );
	}

	std::string to_string(std::vector<std::string> subprograms) {
		return subprograms[0] + " && " + subprograms[1];
	}

};

class RHSDisjunction : public IESyntaxRightHandSide {
	public:
	RHSDisjunction() {
		name = "disjunction";
		independent = false;
	}

	IEProgram* combine_subprograms(std::vector<IEProgram*> subprograms) {
		return new RegexPredicateIEProgram(P_DISJUNCTION, 
								(RegexPredicateIEProgram*) subprograms[0],
								(RegexPredicateIEProgram*) subprograms[1] );
	}

	std::string to_string(std::vector<std::string> subprograms) {
		return subprograms[0] + " || " + subprograms[1];
	}

};

class RHSPredicateToCh : public IESyntaxRightHandSide {
	public:
	RHSPredicateToCh() {
		name = "p2ch";
		independent = true;
	}

	IEProgram* combine_subprograms(std::vector<IEProgram*> subprograms) {
		return subprograms[0];
	}

	std::string to_string(std::vector<std::string> subprograms) {
		return subprograms[0];
	}

};

class RHSOr : public IESyntaxRightHandSide {
	public:
	RHSOr() {
		name = "||";
		independent = true;
	}

	IEProgram* combine_subprograms(std::vector<IEProgram*> subprograms) {
		NFASkip* root = new NFASkip();
		NFASkip* left = new NFASkip(((RegexIEProgram*)subprograms[0])->nfa);
		NFASkip* right = new NFASkip(((RegexIEProgram*)subprograms[1])->nfa);
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
		NFASkip* root = new NFASkip();
		NFASkip* left = new NFASkip(((RegexIEProgram*)subprograms[0])->nfa);
		NFASkip* right = new NFASkip(((RegexIEProgram*)subprograms[1])->nfa);
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
		NFASkip* child = ((RegexIEProgram*)subprograms[0])->nfa;
		NFASkip* root = new NFASkip(child);
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


LHSReWithFilter* l_re_with_filter;
LHSFilter* l_filter;
LHSPredicate* l_p;
LHSPredicate123* l_p123;
LHSPredicateabc* l_pabc;
LHSPredicateABC* l_pABC;
std::vector<LHSPredicateCh*> ll_p123; /* 123 should include special characters and wild cards */
std::vector<LHSPredicateCh*> ll_pabc;
std::vector<LHSPredicateCh*> ll_pABC;
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

RHSReWithFilter* r_re_with_filter;
RHSFilterToPredicate* r_f2p;
RHSPredicateTo123* r_p2123;
RHSPredicateToabc* r_p2abc;
RHSPredicateToABC* r_p2ABC;
std::vector<RHSPredicateToCh*> rr_p2123;
std::vector<RHSPredicateToCh*> rr_p2abc;
std::vector<RHSPredicateToCh*> rr_p2ABC;
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
	/* ============== allocate left hand side ================ */
	l_re_with_filter = new LHSReWithFilter();

	l_filter = new LHSFilter();
	l_p = new LHSPredicate();
	l_p123 = new LHSPredicate123();
	l_pabc = new LHSPredicateabc();
	l_pABC = new LHSPredicateABC();

	for (char c = '0'; c<='9'; c++)
		ll_p123.push_back(new LHSPredicateCh(c));
	ll_p123.push_back(new LHSPredicateCh((char)P_123));
	for (char c = 'A'; c<='Z'; c++)
		ll_pABC.push_back(new LHSPredicateCh(c));
	ll_pABC.push_back(new LHSPredicateCh((char)P_ABC));
	for (char c = 'a'; c<='z'; c++)
		ll_pabc.push_back(new LHSPredicateCh(c));
	ll_pabc.push_back(new LHSPredicateCh((char)P_abc));

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
	/* ======================================================== */



	/* ============== allocate right hand side ================ */
	r_re_with_filter = new RHSReWithFilter();

	r_f2p = new RHSFilterToPredicate();
	r_p2123 = new RHSPredicateTo123();
	r_p2abc = new RHSPredicateToabc();
	r_p2ABC = new RHSPredicateToABC();

	for (char c = '0'; c<='9'; c++)
		rr_p2123.push_back(new RHSPredicateToCh());
	rr_p2123.push_back(new RHSPredicateToCh());
	for (char c = 'A'; c<='Z'; c++)
		rr_p2ABC.push_back(new RHSPredicateToCh());
	rr_p2ABC.push_back(new RHSPredicateToCh());
	for (char c = 'a'; c<='z'; c++)
		rr_p2abc.push_back(new RHSPredicateToCh());
	rr_p2abc.push_back(new RHSPredicateToCh());

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
	/* ======================================================== */



	/* ============ configure mutation option ================= */
	l_re_with_filter->option.push_back(r_re_with_filter);

	l_filter->option.push_back(r_f2p);
	l_p->option.push_back(r_p2123);
	l_p->option.push_back(r_p2abc);
	l_p->option.push_back(r_p2ABC);
	for (int i=0; i<rr_p2123.size(); i++)
		l_p123->option.push_back(rr_p2123[i]);
	for (int i=0; i<rr_p2abc.size(); i++)
		l_pabc->option.push_back(rr_p2abc[i]);
	for (int i=0; i<rr_p2ABC.size(); i++)
		l_pABC->option.push_back(rr_p2ABC[i]);

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
	/* ======================================================== */



	/* ================ configure subprogram =================== */
	r_re_with_filter->subexp.push_back(l_filter);
	r_re_with_filter->subexp.push_back(l_re);
	r_f2p->subexp.push_back(l_p);
	r_p2123->subexp.push_back(l_p123);
	r_p2abc->subexp.push_back(l_pabc);
	r_p2ABC->subexp.push_back(l_pABC);
	for (int i=0; i<rr_p2123.size(); i++)
		rr_p2123[i]->subexp.push_back(ll_p123[i]);
	for (int i=0; i<rr_p2abc.size(); i++)
		rr_p2abc[i]->subexp.push_back(ll_pabc[i]);
	for (int i=0; i<rr_p2ABC.size(); i++)
		rr_p2ABC[i]->subexp.push_back(ll_pABC[i]);

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
	/* ======================================================== */
}

int main() {
	init();
	int d, b, a;
	std::cin>>d>>b>>a;
	SearchGraph g(d, b, a, l_re_with_filter);
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
