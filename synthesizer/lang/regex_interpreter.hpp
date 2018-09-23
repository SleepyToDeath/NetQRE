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
	CONCAT, STAR, CHAR, RE
};


class RegexAST {
	public:
	RegexWordType type;
	char name;
	vector<shared_ptr<RegexAST> > subtree;

	shared_ptr<NFASkip> to_nfa()
	{
		switch (type)
		{
			case CHAR:
			{
				auto nfa = shared_ptr<NFASkip>(new NFASkip());
				if (name != '?')
				{
					auto op = shared_ptr<NFAState>(new NFAState());
					std::set<shared_ptr<NFAState> > op_trans;
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
					std::set<shared_ptr<NFAState> > op_trans;
					auto ed = shared_ptr<NFAState>(new NFAState());

					op_trans.insert(ed);
					for (char c = '0'; c<='1'; c++)
						op->transitions[c] = op_trans;
					nfa->states.insert(op);
					nfa->states.insert(ed);
					nfa->start_states.insert(op);
					nfa->accept_states.insert(ed);
				}

				return nfa;
			}


			case STAR:
			{
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
				root->accept_states.erase(zero); // disable zero matching
				root->start_states.clear();
				root->start_states.insert(zero);

				return root;
			}

			
			case CONCAT:
			{
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
	bool accept(string code, bool complete, shared_ptr<GeneralExample> input) {

//		cout<<"interpreting code: "<<code<<" is complete? "<<complete<<endl;
		
		/* parse code & build NFA */
		auto cursor = shared_ptr<int>(new int);
		(*cursor) = 0;
		auto ast = parse(code, cursor);
		auto nfa = ast->to_nfa();

		/* match positive */
		for (int i=0; i<input->positive.size(); i++)
		{
			string example = input->positive[i];
			vector<bool> dummy(example.size(), false);
			if (!nfa->accept(example, dummy))
			{
//				cout<<"Reject!\n";
				return false;
			}
		}

		/* don't match negative */
		if (complete)
		{
			for (int i=0; i<input->negative.size(); i++)
			{
				string example = input->negative[i];
				vector<bool> dummy(example.size(), false);
				if (nfa->accept(example, dummy))
				{
//					cout<<"Reject!\n";
					return false;
				}
			}
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
				(*cursor) += 2;
				auto clause = shared_ptr<RegexAST>(new RegexAST());
				clause->type = STAR;
				clause->subtree.push_back(parse(code, cursor));
				(*cursor)++;
				root->subtree.push_back(clause);
			}
			else if (code[(*cursor)] == ')')
				return root;
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
	auto p = shared_ptr<GeneralProgram>(new GeneralProgram(pattern, true));
	cout<<"Matching result: "<<(p->accept(e)?"Match":"Mismatch")<<endl;;
}

#endif
