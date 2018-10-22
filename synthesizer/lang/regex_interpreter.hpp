#ifndef REGEX_INTERPRETER_HPP
#define REGEX_INTERPRETER_HPP

#include "nfa_skip.hpp"
#include <vector>

using std::vector;
using std::string;

#include <iostream>
using std::cin;
using std::cout;
using std::endl;

enum RegexWordType {
	CONCAT, STAR, CHAR, RE, ESC, UNION
};

const char CHAR_SET[] = { '.' , ':', '/', '?', '_', '-' };

class RegexAST {
	public:
	RegexWordType type;
	char name;
	vector<shared_ptr<RegexAST> > subtree;

	string to_string() {
		string s = "0";
		s[0] = name;
		for (int i=0; i<subtree.size(); i++)
			s += subtree[i]->to_string();
		return s;
	}

	double get_complexity()
	{
		double sum = 0.0;
		for (int i=0; i<subtree.size(); i++)
			sum += subtree[i]->get_complexity();
		if (type == UNION)
			return (sum + 200) * 1.5;
//		else if (type == CONCAT)
//			return sum + 100.0;
		if (type == STAR)
			return (sum + 200) * 1.2;
//		else if (type == CHAR && name != '?')
//			return sum - 0.0;
		return sum;
	}

	shared_ptr<NFASkip> to_nfa()
	{
		switch (type)
		{
			case ESC:
			{
				auto nfa = shared_ptr<NFASkip>(new NFASkip());
				if (name == 'c')
				{
					auto op = shared_ptr<NFAState>(new NFAState());
					unordered_set<shared_ptr<NFAState> > op_trans;
					auto ed = shared_ptr<NFAState>(new NFAState());

					op_trans.insert(ed);
					for (char c = 'a'; c<='z'; c++)
						op->transitions[c] = op_trans;
					nfa->states.insert(op);
					nfa->states.insert(ed);
					nfa->start_states.insert(op);
					nfa->accept_states.insert(ed);
				}
				else if (name == 'C')
				{
					auto op = shared_ptr<NFAState>(new NFAState());
					unordered_set<shared_ptr<NFAState> > op_trans;
					auto ed = shared_ptr<NFAState>(new NFAState());

					op_trans.insert(ed);
					for (char c = 'A'; c<='Z'; c++)
						op->transitions[c] = op_trans;
					nfa->states.insert(op);
					nfa->states.insert(ed);
					nfa->start_states.insert(op);
					nfa->accept_states.insert(ed);
				}
				else if (name == 'd')
				{
					auto op = shared_ptr<NFAState>(new NFAState());
					unordered_set<shared_ptr<NFAState> > op_trans;
					auto ed = shared_ptr<NFAState>(new NFAState());

					op_trans.insert(ed);
					for (char c = '0'; c<='9'; c++)
						op->transitions[c] = op_trans;
					nfa->states.insert(op);
					nfa->states.insert(ed);
					nfa->start_states.insert(op);
					nfa->accept_states.insert(ed);
				}
				else if (name == '#')
				{
					/* do nothing, empty NFA accepting nothing */
				}
				return nfa;
			}
			case CHAR:
			{
				auto nfa = shared_ptr<NFASkip>(new NFASkip());
				if (name != '?')
				{
					auto op = shared_ptr<NFAState>(new NFAState());
					unordered_set<shared_ptr<NFAState> > op_trans;
					auto ed = shared_ptr<NFAState>(new NFAState());

					op_trans.insert(ed);
					op->transitions[name] = op_trans;
					nfa->states.insert(op);
					nfa->states.insert(ed);
					nfa->start_states.insert(op);
					nfa->accept_states.insert(ed);
				}
				else
				{
					auto op = shared_ptr<NFAState>(new NFAState());
					unordered_set<shared_ptr<NFAState> > op_trans;
					auto ed = shared_ptr<NFAState>(new NFAState());

					op_trans.insert(ed);
					for (char c = 'A'; c<='Z'; c++)
						op->transitions[c] = op_trans;
					for (char c = 'a'; c<='z'; c++)
						op->transitions[c] = op_trans;
					for (char c = '0'; c<='9'; c++)
						op->transitions[c] = op_trans;
					for (int i=0; i<sizeof(CHAR_SET); i++)
						op->transitions[CHAR_SET[i]] = op_trans;
					nfa->states.insert(op);
					nfa->states.insert(ed);
					nfa->start_states.insert(op);
					nfa->accept_states.insert(ed);
				}

				return nfa;
			}


			case STAR:
			{
				if (subtree.size() == 0)
					return shared_ptr<NFASkip>(new NFASkip());
				auto child = subtree[0]->to_nfa();
				auto root = shared_ptr<NFASkip>(new NFASkip(child));
				auto zero = shared_ptr<NFAState>(new NFAState());
				root->states.insert(zero);
				root->accept_states.insert(zero);
				for (NFAIt i = root->accept_states.begin(); i!=root->accept_states.end(); i++)
				{
					auto e_trans = (*i)->transitions[Epsilon];
					e_trans.insert(root->start_states.begin(), root->start_states.end());
					(*i)->transitions[Epsilon] = e_trans;
				}
//				root->accept_states.erase(zero); // disable zero matching
				root->start_states.clear();
				root->start_states.insert(zero);

				return root;
			}

			
			case CONCAT:
			{
				if (subtree.size() == 0)
					return shared_ptr<NFASkip>(new NFASkip());
				auto root = shared_ptr<NFASkip>(new NFASkip(subtree[0]->to_nfa()));
				for (int i=1; i<subtree.size(); i++)
				{
					auto head = shared_ptr<NFASkip>(new NFASkip(root));
					auto next = shared_ptr<NFASkip>(new NFASkip(subtree[i]->to_nfa()));
					root = shared_ptr<NFASkip>(new NFASkip());
					root->states.insert(head->states.begin(), head->states.end());
					root->states.insert(next->states.begin(), next->states.end());
					root->start_states = head->start_states;
					root->accept_states = next->accept_states;
					for (NFAIt i = head->accept_states.begin(); i!= head->accept_states.end(); i++)
					{
						auto e_trans = (*i)->transitions[Epsilon];
						e_trans.insert(next->start_states.begin(), next->start_states.end());
						(*i)->transitions[Epsilon] = e_trans;
					}
				}

				return root;
			}

			case UNION:
			{
				if (subtree.size() == 0)
					return shared_ptr<NFASkip>(new NFASkip());
				auto root = shared_ptr<NFASkip>(new NFASkip());
				auto left = shared_ptr<NFASkip>(new NFASkip(subtree[0]->to_nfa()));
				auto right = shared_ptr<NFASkip>(new NFASkip(subtree[1]->to_nfa()));
				auto zero = shared_ptr<NFAState>(new NFAState());
				root->states.insert(left->states.begin(), left->states.end());
				root->states.insert(right->states.begin(), right->states.end());
				root->states.insert(zero);

				unordered_set<shared_ptr<NFAState> > zero_trans;
				zero_trans.clear();
				zero_trans.insert(left->start_states.begin(), left->start_states.end());
				zero_trans.insert(right->start_states.begin(), right->start_states.end());
				zero->transitions[Epsilon] = zero_trans;

				root->start_states.clear();
				root->start_states.insert(zero);

				root->accept_states = right->accept_states;
				root->accept_states.insert(left->accept_states.begin(), left->accept_states.end());

				return root;
			}

	

			case RE:
			return subtree[0]->to_nfa();


			default:
			return nullptr;
		}
	}
};

class RegexInterpreter : public GeneralInterpreter
{
	public:

	virtual double extra_complexity(AbstractCode code) {
		auto cursor = shared_ptr<int>(new int);
		(*cursor) = 0;
		auto ast = parse(code.neg, cursor);
		return ast->get_complexity();
	}

	bool accept(AbstractCode code, bool complete, shared_ptr<GeneralExample> input, IEConfig cfg) {

		/* parse code & build NFA */
		auto cursor = shared_ptr<int>(new int);
		(*cursor) = 0;
		auto ast_pos = parse(code.pos, cursor);
//		cout<<"[Pos Ast]"<<ast_pos->to_string()<<endl;
		(*cursor) = 0;
		auto ast_neg = parse(code.neg, cursor);
//		cout<<"[Neg Ast]"<<ast_neg->to_string()<<endl;
		auto nfa_pos = ast_pos->to_nfa();
		auto nfa_neg = ast_neg->to_nfa();

		bool pos_flag = cfg.pos_all;

		if (cfg.pos_check)
		{
			/* match positive */
			for (int i=0; i<input->positive.size(); i++)
			{
				string example = input->positive[i];
				vector<bool> dummy(example.size(), false);
				bool acc = nfa_pos->accept(example, dummy) xor (!cfg.pos_accept);
				if (!acc && cfg.pos_all)
				{
//					cout<<"[Target Example] "<<example<<endl;
					pos_flag = false;
					break;
				}
				if (acc && !cfg.pos_all)
				{
//					cout<<"[Target Example] "<<example<<endl;
					pos_flag = true;
					break;
				}
			}
		}
		else
			pos_flag = true;

		if (!pos_flag)
		{
//			cout<<"Rej pos!\n";
			return false;
		}

		bool neg_flag = cfg.neg_all;

		if (cfg.pos_check)
		{
			/* don't match negative */
			for (int i=0; i<input->negative.size(); i++)
			{
				string example = input->negative[i];
				vector<bool> dummy(example.size(), false);
				bool acc = nfa_neg->accept(example, dummy) xor (!cfg.neg_accept);
				if (!acc && cfg.neg_all)
				{
//					cout<<"[Target Example] "<<example<<endl;
					neg_flag = false;
					break;
				}
				if (acc && !cfg.neg_all)
				{
//					cout<<"[Target Example] "<<example<<endl;
					neg_flag = true;
					break;
				}
			}
		}
		else
			neg_flag = true;
		
		if (!neg_flag)
		{
//			cout<<"Rej neg!\n";
			return false;
		}

//		cout<<"Accept!\n";
		return true;
	}

	/* parser */
	shared_ptr<RegexAST> parse(string code, shared_ptr<int> cursor)
	{
		auto root = shared_ptr<RegexAST>(new RegexAST());
		root->type = CONCAT;
		while ((*cursor)<code.length())
		{
			while ((code[(*cursor)] == ' ' ) || (code[(*cursor)] == '$' ))
				(*cursor)++;
			if (code[(*cursor)] == '*')
			{
				(*cursor) += 2; /* *( */
				auto clause = shared_ptr<RegexAST>(new RegexAST());
				clause->type = STAR;
				clause->subtree.push_back(parse(code, cursor));
				(*cursor)++; /* ) */
				root->subtree.push_back(clause);
			}
			else if (code[(*cursor)] == ')')
				return root;
			else if (code[(*cursor)] == '\\' )
			{
				(*cursor)++; /* \ */
				auto clause = shared_ptr<RegexAST>(new RegexAST());
				clause->type = ESC;
				clause->name = code[(*cursor)];
				(*cursor)++; /* char */
				root->subtree.push_back(clause);
			}
			else if (code[(*cursor)] == '|')
			{
				(*cursor) += 4; /* ||(( */
				auto clause = shared_ptr<RegexAST>(new RegexAST());
				clause->type = UNION;
				clause->subtree.push_back(parse(code, cursor));
				(*cursor) += 3; /* ),( */
				clause->subtree.push_back(parse(code, cursor));
				(*cursor) += 2; /* ) */
				root->subtree.push_back(clause);
			}
			else
			{
				auto clause = shared_ptr<RegexAST>(new RegexAST());
				clause->type = CHAR;
				clause->name = code[(*cursor)];
				(*cursor)++;
				root->subtree.push_back(clause);
//				cout<<root->subtree.size();
			}
		}
		return root;
	}
};


void test_interpretor()
{
	string pattern;
	string candidate;
	getline(cin, pattern);
	getline(cin, candidate);
	auto e = shared_ptr<GeneralExample>(new GeneralExample());
	e->positive.push_back(candidate);
	auto p = shared_ptr<GeneralProgram>(new GeneralProgram(AbstractCode(pattern,""), true));
	cout<<"Matching result: "<<(p->accept(e)?"Match":"Mismatch")<<endl;;
}

#endif
