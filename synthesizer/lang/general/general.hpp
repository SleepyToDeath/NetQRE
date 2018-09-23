#ifndef LANG_GENERAL_HPP
#define LANG_GENERAL_HPP

#include "../../core/incomplete_execution.h"
#include "json_wrapper.h"
#include <memory>
#include <iostream>

using std::cout;
using std::endl;
using std::string;
using std::vector;
using std::map;

const std::string SYNTAX_ROOT_NAME = "program";

class GeneralExample;
class GeneralSyntaxRightHandSide;

class GeneralInterpreter {
	public:
	virtual bool accept(std::string code, bool complete,  shared_ptr<GeneralExample> input) = 0;
};

class GeneralExample: public IEExample {
	public:
	vector<string> positive;
	vector<string> negative;
};

/* [TODO] change to smart pointer */
/* somehow build source code when initializing
 * accept() will feed the source code and input to external interpreter */
class GeneralProgram: public IEProgram {
	public:
	GeneralProgram(std::string src, bool complete) {
		source_code = src;
		this->complete = complete;
	}

	bool accept(shared_ptr<IEExample> input) {
		return interpreter->accept(source_code, complete, std::static_pointer_cast<GeneralExample>(input));
	}

	/* set this to the interpreter of your specific language */
	static std::unique_ptr<GeneralInterpreter> interpreter;

	bool complete;
	std::string source_code;
};

class GeneralSyntaxLeftHandSide;

class GeneralSyntaxRightHandSide : public IESyntaxRightHandSide {
	public:
	GeneralSyntaxRightHandSide() { name = ""; }
	std::string to_string(std::vector<std::string> subs) { return ""; }
	shared_ptr<IEProgram> combine_subprograms(std::vector< shared_ptr<IEProgram> > subprograms) {return nullptr;}

	std::vector<std::shared_ptr<GeneralSyntaxLeftHandSide> > subexp_full;
};

class GeneralSyntaxLeftHandSide : public IESyntaxLeftHandSide {

	private:

	class InitializerState {
		public:
		std::map<std::string, std::shared_ptr<GeneralSyntaxLeftHandSide> > name_list;
		std::map<std::shared_ptr<GeneralSyntaxLeftHandSide>, std::shared_ptr<GeneralSyntaxLeftHandSide> > replace_list;
	};

	public:

	/* dummy initializer */
	GeneralSyntaxLeftHandSide() {
		is_term = true;
	}

	/* initialize from json */
	/* root will initialize names
	 * then recursively(by json's structure) call from_json() to initialize every node */
	void from_json(std::shared_ptr<GJson> j, std::shared_ptr<InitializerState> state) {
		
		if (state == nullptr) { /* this is the root */
			/* allocate context */
			state = std::shared_ptr<InitializerState>(new InitializerState());
			/* scan names */
			name = SYNTAX_ROOT_NAME;
			state->name_list[name] = std::static_pointer_cast<GeneralSyntaxLeftHandSide>(shared_from_this());
			scan_names(j, state);
			/* init grammar tree */
			std::shared_ptr<GJson> names = j->get(0)->value();
			for (int i=0; i<names->size(); i++) {
				state->name_list[names->get(i)->name()]->from_json(names->get(i), state);
			}
			/* init incomplete execution */
//			std::shared_ptr<GJson> exec = j->get(1)->value();
//			for (int i=0; i<exec->size(); i++) {
//			}
		}
		else { /* this is not the root */
//			cout<<"initializing LHS "<<name<<endl;
			std::shared_ptr<GJson> mutations = j->value();
			for (int i=0; i<mutations->size(); i++)
			{
				std::shared_ptr<GJson> rhs = mutations->get(i);
				std::shared_ptr<GeneralSyntaxRightHandSide> r(new GeneralSyntaxRightHandSide());
				for (int j=0; j<rhs->size(); j++)
				{
					std::shared_ptr<GeneralSyntaxLeftHandSide> l = state->name_list[rhs->get(j)->name()];
					if (l->is_functional())
						r->subexp.push_back(l);
					r->subexp_full.push_back(l);
				}
//				cout<<"#"<<i<<" subexp size: "<<r->subexp.size()<<endl;
//				cout<<"#"<<i<<" subexp full size: "<<r->subexp_full.size()<<endl;
				option.push_back(r);
			}
//			cout<<"total options: "<<option.size()<<endl;
		}
	}

	bool is_functional() { 
		return functional;
	}

	std::string equivalent_complete_program;

	private:

	bool functional; /* example of non-functional symbol : "(", ")", ",", non-functional symbols start with "$" */

	/* will allocate space for all LHS except for root(terminals are considered to be LHS)
	 * will determine is_term for all LHS
	 * will determine functional for all LHS */
	void scan_names(std::shared_ptr<GJson> j, std::shared_ptr<InitializerState> state) {
		shared_ptr<GJson> syntax = j->get(0)->value();
		shared_ptr<GJson> exec = j->get(1)->value();
		/* syntax */
		for (int i=0; i<syntax->size(); i++)
		{
			/* Add LHS to the name list
			 * LHS must be non-terminal and functional */
			std::string name = syntax->get(i)->name();
//			cout<<name<<endl;
			if (state->name_list.count(name) == 0)
			{
				state->name_list[name] = shared_ptr<GeneralSyntaxLeftHandSide>(new GeneralSyntaxLeftHandSide());
				state->name_list[name]->name = name;
//				cout<<"this name is: "<<name<<endl;
			}
			state->name_list[name]->is_term = false;
			state->name_list[name]->functional = true;
			/* Add elements in RHS to the name list 
			 * is_term is by default true, not functional if name start with '$'*/
			shared_ptr<GJson> rhs = syntax->get(i)->value();
			for (int j=0; j<rhs->size(); j++)
			{
				shared_ptr<GJson> rhs_entry = rhs->get(j);
				for (int k=0; k<rhs_entry->size(); k++)
				{
					std::string name2 = rhs_entry->get(k)->name();
					if (state->name_list.count(name2) == 0)
					{
						state->name_list[name2] = shared_ptr<GeneralSyntaxLeftHandSide>(new GeneralSyntaxLeftHandSide());
						state->name_list[name2]->name = name2;
//						cout<<"this name is: "<<name2<<endl;
					}
					if (name2[0] == '$')
					{
						state->name_list[name2]->functional = false;
						state->name_list[name2]->name = name2.substr(1, name2.length()-1);
					}
					else
						state->name_list[name2]->functional = true;
				}
			}
		}
		/* execution */
		for (int i=0; i<exec->size(); i++)
		{
			std::string name = exec->get(i)->name();
			state->name_list[name]->equivalent_complete_program = exec->get(i)->value()->name();
		}
	}

	shared_ptr<IEProgram> to_program() {return nullptr;}
};

class GeneralSyntaxTree : public IESyntaxTree {
	public:
	GeneralSyntaxTree(shared_ptr<SyntaxTree> src):	IESyntaxTree(src) {}
	GeneralSyntaxTree(shared_ptr<SyntaxTreeNode> root): IESyntaxTree(root) {}
	shared_ptr<IEProgram> to_program() {
		return shared_ptr<IEProgram>(new GeneralProgram(to_code(), is_complete()));
	}

	std::string to_code() {
		std::string s;
		if (root->get_type()->is_term) 
			s = root->get_type()->name;
		else if (root->get_option() == SyntaxLeftHandSide::NoOption)
			s = std::static_pointer_cast<GeneralSyntaxLeftHandSide>(root->get_type())->equivalent_complete_program;
		else
		{
			auto rhs = std::static_pointer_cast<GeneralSyntaxRightHandSide> (root->get_type()->option[root->get_option()]);
			int j = 0;
			for (int i=0; i<rhs->subexp_full.size(); i++)	{
				if (rhs->subexp_full[i]->is_functional()) {
					s = s + " " + (std::static_pointer_cast<GeneralSyntaxTree>(subtree[j])->to_code());
					j++;
				}
				else {
					s = s + " " + (rhs->subexp_full[i]->name);
				}
			}
		}
		return s;
	}


	std::string to_string() {
		std::string s;
		if (root->get_type()->is_term) 
			s = root->get_type()->name;
		else if (root->get_option() == SyntaxLeftHandSide::NoOption)
			s = std::static_pointer_cast<GeneralSyntaxLeftHandSide>(root->get_type())->name;
		else
		{
			auto rhs = std::static_pointer_cast<GeneralSyntaxRightHandSide> (root->get_type()->option[root->get_option()]);
			int j = 0;
			for (int i=0; i<rhs->subexp_full.size(); i++)	{
				if (rhs->subexp_full[i]->is_functional()) {
					s = s + (subtree[j]->to_string());
					j++;
				}
				else {
					s = s + (rhs->subexp_full[i]->name);
				}
			}
		}
		return s;
	}

	void copy_initializer(shared_ptr<SyntaxTree> src) {
		for (int i=0; i<src->subtree.size(); i++)
			subtree.push_back(shared_ptr<GeneralSyntaxTree>(new GeneralSyntaxTree(src->subtree[i])));
	}
};

class GeneralSyntaxTreeFactory : public SyntaxTreeFactory {
	public:

	shared_ptr<SyntaxTree> get_new( shared_ptr<SyntaxTreeNode> root) 
	{
		return shared_ptr<GeneralSyntaxTree>(new GeneralSyntaxTree(root));
	}

	shared_ptr<SyntaxTree> get_new( shared_ptr<SyntaxTree> src) 
	{
		return shared_ptr<GeneralSyntaxTree>(new GeneralSyntaxTree(src));
	}
};

std::unique_ptr<SyntaxTreeFactory> SyntaxTree::factory = unique_ptr<GeneralSyntaxTreeFactory>(new GeneralSyntaxTreeFactory());

#endif
