#ifndef LANG_GENERAL_HPP
#define LANG_GENERAL_HPP

#include "json_wrapper.h"
#include <memory>

const std::string SYNTAX_ROOT_NAME = "program";

class GeneralInterpreter {
	public:
	virtual bool accept(std::string code, IEExample* input);
};


/* [TODO] change to smart pointer */
/* somehow build source code when initializing
 * accept() will feed the source code and input to external interpreter */
class GeneralProgram: public IEProgram {
	public:
	GeneralProgram(std::string src) {
		source_code = src;
	}

	bool accept(IEExample* e) {
		return interpreter->accept(source_code, input);
	}

	/* set this to the interpreter of your specific language */
	static std::unique_ptr<GeneralInterpreter> interpreter;

	std::string source_code;
};

/* [TODO] change to smart pointer */
class GeneralSyntaxTree : public IESyntaxTree {
	GeneralSyntaxTree(SyntaxTree* src):	IESyntaxTree(src) {}
	IEProgram* to_program() {
		return new GeneralProgram(to_string());
	}

	std::string to_string() {
		std::string s;
		if (root->get_type()->is_term) 
			s = root->get_type()->name;
		else if (root->get_option() == SyntaxLeftHandSide::NoOption)
			s = root->get_type()->name;
		else
		{
			shared_ptr<GeneralSyntaxRightHandSide> rhs = root->get_type()->option[root->get_option()];
			int j = 0;
			for (int i=0; i<->rhs->subexp_full.size(); i++)	{
				if (rhs->subexp_full[i]->functional) {
					s = s+subtree[j]->to_string()
					j++;
				}
				else {
					s = s+rhs->subexp_full[i]->name;
				}
			}
		}
		return s;
	}
};

class GeneralSyntaxRightHandSide;

class GeneralSyntaxLeftHandSide : public IESyntaxLeftHandSide {

	private:

	class InitializerState {
		public:
		std::map<std::string, std::shared_ptr<GeneralSyntaxLeftHandSide>> name_list;
		std::map<std::shared_ptr<GeneralSyntaxLeftHandSide>, std::shared_ptr> replace_list;
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
			state->name_list[name] = shared_from_this();
			scan_names(j, state);
			/* init grammar tree */
			std::shared_ptr<GJson> names = j->get(0)->value();
			for (int i=0; i<names->size(); i++) {
				state->name_list[names->get(i)->name()]->from_json(names[i], state);
			}
			/* init incomplete execution */
			std::shared_ptr<GJson> exec = j->get(1)->value();
			for (int i=0; i<exec->size(); i++) {

			}
		}
		else { /* this is not the root */
			std::shared_ptr<GJson> mutations = j->value();
			for (int i=0; i<mutations->size(); i++)
			{
				std::shared_ptr<GJson> rhs = mutations->get(i);
				std::shared_ptr<GeneralSyntaxRightHandSide> r(new GeneralSyntaxRightHandSide());
				for (int j=0; j<rhs->size(); j++)
				{
					std::shared_ptr<GeneralSyntaxLeftHandSide> l = state->name_list[rhs->get(i)->name()];
					if (l->functional())
						r->subexp.push_back(l);
					r->subexp_full.push_back(l);
				}
				option.push_back(r);
			}
		}
	}

	bool functional() { 
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
			if (state->name_list.count(name) == 0)
				state->name_list[name] = shared_ptr<GeneralSyntaxLeftHandSide>(new GeneralSyntaxLeftHandSide());
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
					std::string name2 = rhs_entry->get(k);
					if (state->name_list.count(name2) == 0)
						state->name_list[name2] = shared_ptr<GeneralSyntaxLeftHandSide>(new GeneralSyntaxLeftHandSide());
					if (name2[0] == '$')
						state->name_list[name2]->functional = false;
					else
						state->name_list[name2]->functional = true;
				}
			}
		}
		/* execution */
		for (int i=0; i<exec->size(); i++)
		{
			std::string name = exec->get(i)->name();
			state->name_list[name]->equivalent_complete_program = exec->get(i)->value();
		}
	}

	IEProgram* to_program() {return nullptr;}
};

class GeneralSyntaxRightHandSide : public IESyntaxRightHandSide {
	GeneralSyntaxRightHandSide() { name = ""; }
	std::string to_string(std::vector<std::string> subs) { return ""; }
	IEProgram* combine_subprograms(std::vector<IEProgram*> subprograms) { return nullptr; }

	std::vector<std::shared_ptr<SyntaxLeftHandSide> > subexp_full;
};

#endif
