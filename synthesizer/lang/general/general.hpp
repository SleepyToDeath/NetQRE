#ifndef LANG_GENERAL_HPP
#define LANG_GENERAL_HPP

#include "../../../general-lang/incomplete_execution.h"
#include "../../../general-lang/general.hpp"
#include "../../core/redundancy.h"
#include "json_wrapper.h"
#include <memory>
#include <iostream>

/* 
	This file converts a json configuration into grammar.
*/

using std::cerr;
using std::endl;
using Rubify::string;
using std::static_pointer_cast;

const string SYNTAX_ROOT_NAME = "program";

class GeneralConfigParser {
	private:

	class State {
		public:
		map<string, shared_ptr<GeneralSyntaxLeftHandSide> > name_list;
		map<int, shared_ptr<GeneralSyntaxLeftHandSide> > dep_list; /* input dependent LHS list */
	};

	class Token {
		public:
		bool variable;
		string name;
		shared_ptr<GeneralSyntaxLeftHandSide> lhs;
	};

	shared_ptr<State> state;

	bool is_id(char c) {
		return (c>='0' && c<='9') || (c>='a' && c<='z') || (c>='A' && c<='Z') || (c=='_') || (c=='#');
	}

	shared_ptr<Token> tokenize_next(string code, shared_ptr<int> cursor) {
		auto t = shared_ptr<Token>(new Token());
		int i = (*cursor);

		cerr<<code.length()<<endl;

		/* parse type */
		while (i<code.length() && code[i] == ' ')
			i++;
		if (code[i] == '@')
			throw string("Invalid LHS name. Can't start with '@'.\n");
		else
		{
			t->variable = false;
			int j = i;
			while ((j<code.length()) && (code[j] != ' ') && (code[j] != '@') && (state->name_list.count(code.substr(i, j-i)) == 0))
				j++;
			if (state->name_list.count(code.substr(i, j-i)) == 0)
				throw string("Invalid LHS name. Not found: ") + code.substr(i, j-i) + "\n";
			t->name = code.substr(i,j-i);
			t->lhs = state->name_list[t->name];
			i=j;
		}

		/* parse variable(if it is) */
		while (i<code.length() && code[i] == ' ')
			i++;
		if (code[i] == '@')
		{
			t->variable = true;
			i++;
			int j = i;
			while (j<code.length() && code[j]>='0' && code[j]<='9')
				j++;
			if (i==j)
				throw string("Invalid variable name. Must be numbers.\n");
			t->name = code.substr(i,j-i);
			i=j;
		}

		while (i<code.length() && code[i] == ' ')
			i++;
		(*cursor) = i;
		return t;
	}

	/* will allocate space for all LHS except for root(terminals are considered to be LHS)
	 * will determine is_term for all LHS
	 * will determine functional for all LHS */
	void scan_names(std::shared_ptr<GJson> json) {

		auto parse_non_term = [&](string name) {
			if (state->name_list.count(name) == 0)
			{
				cerr<<"new name: "<<name<<endl;
				state->name_list[name] = shared_ptr<GeneralSyntaxLeftHandSide>(new GeneralSyntaxLeftHandSide());
			}
			state->name_list[name]->name = name;
			state->name_list[name]->is_term = false;
			state->name_list[name]->functional = true;
		};

		auto parse_maybe_term = [&](string name) {
			string real_name = name;
			if (name[0] == '$')
				real_name = name.substr(1, name.length()-1);
			/* add new LHS to list */
			if (state->name_list.count(real_name) == 0)
			{
				cerr<<"new name: "<<real_name<<endl;
				state->name_list[real_name] = shared_ptr<GeneralSyntaxLeftHandSide>(new GeneralSyntaxLeftHandSide());
				state->name_list[real_name]->name = real_name;
				/* check input dependency */
				if (name.start_with("\\r"))
				{
					state->name_list[real_name]->is_term = false;
					int handle = Rubify::string(name.substr(2, name.size()-2)).to_i();
					state->dep_list[handle] = (state->name_list[real_name]);
				}
			}
			/* set functional */
			if (name[0] == '$')
			{
				state->name_list[real_name]->functional = false;
			}
			else
				state->name_list[real_name]->functional = true;

		};

		shared_ptr<GJson> syntax = json->get(0)->value();
		/* syntax */
		for (int i=0; i<syntax->size(); i++)
		{
			/* Add LHS to the name list
			 * LHS must be non-terminal and functional */
			string name = syntax->get(i)->name();
			parse_non_term(name);

			/* Add elements in RHS to the name list 
			 * is_term is by default true, not functional if name start with '$'*/
			shared_ptr<GJson> rhs = syntax->get(i)->value();
			for (int j=0; j<rhs->size(); j++)
			{
				shared_ptr<GJson> rhs_entry = rhs->get(j);
				for (int k=0; k<rhs_entry->size(); k++)
				{
					string name2 = rhs_entry->get(k)->name();
					parse_maybe_term(name2);
				}
			}
		}
	}

	shared_ptr<SyntaxTreeTemplate> parse_template_recursive(string code, shared_ptr<int> cursor) {
		cerr<<code.substr((*cursor), code.size()-(*cursor))<<endl;
		shared_ptr<Token> t = tokenize_next(code, cursor);
		cerr<<t->variable<<" | "<<t->name<<endl;
		if (t->variable || t->lhs->is_term)
		{
			auto root = shared_ptr<SyntaxTreeNode>(new SyntaxTreeNode(t->lhs));
			auto temp = shared_ptr<SyntaxTreeTemplate>(new SyntaxTreeTemplate(root));
			temp->var_name = t->name;
			cerr<<"<< "<<t->variable<<" | "<<t->name<<endl;
			return temp;
		}
		else
		{
			auto root = shared_ptr<SyntaxTreeNode>(new SyntaxTreeNode(t->lhs));
			auto temp = shared_ptr<SyntaxTreeTemplate>(new SyntaxTreeTemplate(root));
			vector<shared_ptr<SyntaxTreeTemplate> > subexp_full;
			while((*cursor) < code.length())
			{
				subexp_full.push_back(parse_template_recursive(code, cursor));
				for (int i=0; i<t->lhs->option.size(); i++)
				{
					auto rhs = std::static_pointer_cast<GeneralSyntaxRightHandSide>(t->lhs->option[i]);
					bool match_flag = true;
					if (rhs->subexp_full.size() != subexp_full.size())
						match_flag = false;
					else 
					{
						for (int j=0; j<subexp_full.size(); j++)
							if (subexp_full[j]->root->get_type() != rhs->subexp_full[j])
							{
								match_flag = false;
								break;
							}
					}
					if (match_flag)
					{
						vector<shared_ptr<SyntaxTree> > subexp;
						for (int j=0; j<subexp_full.size(); j++)
						{
							auto candidate = std::static_pointer_cast<GeneralSyntaxLeftHandSide>(subexp_full[j]->root->get_type());
							if (candidate->is_functional())
								subexp.push_back(subexp_full[j]);
						}
						temp->subtree = subexp;
						/*
						cerr<<"name: "<<temp->root->get_type()->name<<endl;
						cerr<<"subtree size"<<temp->subtree.size()<<endl;
						cerr<<"full subtree size"<<subexp_full.size()<<endl;
						*/
						temp->root->set_option(i);
						cerr<<"<< "<<t->variable<<" | "<<t->name<<endl;
						return temp;
					}
				}
			}
		}
		cerr<<"<< "<<t->variable<<" | "<<t->name<<endl;
		throw string("Unable to parse template. Invalid program.\n");
		return nullptr;
	}

	shared_ptr<SyntaxTreeTemplate> parse_template(string code) {
		cerr<<"parsing template "<<code<<endl;
		auto cursor = shared_ptr<int>(new int(0));
		if (code.length() == 0)
			return nullptr;
		return parse_template_recursive(code, cursor);
	}

	shared_ptr<RedundancyPlan> parse_redundancy_plan(shared_ptr<GJson> json) {
		shared_ptr<GJson> cnd_json = json->get(3)->value();
		shared_ptr<GJson> ucnd_json = json->get(4)->value();
		auto plan = shared_ptr<RedundancyPlan>(new RedundancyPlan());

		/* parse conditional */
		for (int i=0; i<cnd_json->size(); i++)
		{
			auto temp = shared_ptr<ConditionalRedundancyTemplate>(new ConditionalRedundancyTemplate());
			auto temp_json = cnd_json->get(i);
			temp->temp = parse_template(temp_json->get(0)->value()->name());
			auto checklist_json = temp_json->get(1)->value();
			for (int j=0; j<checklist_json->size(); j++)
				temp->checklist.push_back(parse_template(checklist_json->get(j)->name()));
			string condition = temp_json->get(2)->value()->name();
			switch (condition[0]) {
				case 'A':
					temp->all_program = true;
					break;
				case 'E':
					temp->all_program = false;
					break;
				default:
					throw string("Condition format incorrect.\n");
			}
			switch (condition[1]) {
				case 'A':
					temp->all_example = true;
					break;
				case 'E':
					temp->all_example = false;
					break;
				default:
					throw string("Condition format incorrect.\n");
			}
			switch (condition[2]) {
				case 'A':
					temp->accept = true;
					break;
				case 'R':
					temp->accept = false;
					break;
				default:
					throw string("Condition format incorrect.\n");
			}

			plan->cnd.push_back(temp);
			cerr<<"CTemplate: "<< endl<<temp->temp->to_string()<<endl;
			for (int i=0; i<temp->checklist.size(); i++)
				cerr<<temp->checklist[i]->to_string()<<" | ";
			cerr<<endl;
		}

		/* parse unconditional */
		for (int i=0; i<ucnd_json->size(); i++)
		{
			auto temp_json = ucnd_json->get(i);
			auto temp = shared_ptr<UnconditionalRedundancyTemplate>(new UnconditionalRedundancyTemplate());
			temp->temp_src = parse_template(temp_json->name());
			temp->temp_dst = parse_template(temp_json->value()->name());

			plan->ucnd.push_back(temp);
			cerr<<"UTemplate: " <<endl<<temp->temp_src->to_string()<<endl;
			cerr<<temp->temp_dst->to_string()<<endl;
		}

		return plan;
	}

	void parse_abstract_program(shared_ptr<GJson> json) {
		shared_ptr<GJson> pos_json = json->get(1)->value();
		shared_ptr<GJson> neg_json = json->get(2)->value();
		for (int i=0; i<pos_json->size(); i++)
		{
			string name = pos_json->get(i)->name();
			auto temp = parse_template(pos_json->get(i)->value()->name());
			state->name_list[name]->positive_abstract_code = (temp == nullptr) ? "" : temp->marshall();
		}
		for (int i=0; i<neg_json->size(); i++)
		{
			string name = neg_json->get(i)->name();
			auto temp = parse_template(neg_json->get(i)->value()->name());
			state->name_list[name]->negative_abstract_code = (temp == nullptr) ? "" : temp->marshall();
		}
	}

	void parse_grammar(shared_ptr<GJson> json) {
		/* init grammar tree */
		std::shared_ptr<GJson> rules = json->get(0)->value();
		for (int i=0; i<rules->size(); i++) {
			auto lhs_json = rules->get(i);
			auto lhs = state->name_list[lhs_json->name()];

			auto mutations = lhs_json->value();
			for (int j=0; j<mutations->size(); j++)
			{
				std::shared_ptr<GJson> rhs_json = mutations->get(j);
				std::shared_ptr<GeneralSyntaxRightHandSide> rhs(new GeneralSyntaxRightHandSide());
				for (int k=0; k<rhs_json->size(); k++)
				{
					string name = rhs_json->get(k)->name();
					if (name[0] == '$')
						name = name.substr(1, name.length()-1);
//					cerr<<lhs->name<<"-->"<<name<<endl;
					std::shared_ptr<GeneralSyntaxLeftHandSide> sub_lhs = state->name_list[name];
					if (sub_lhs->is_functional())
					{
						rhs->subexp.push_back(sub_lhs);
					}
					rhs->subexp_full.push_back(sub_lhs);
				}
				lhs->option.push_back(rhs);
			}
		}
	}

	void init_parsing_state() {
		/* allocate context */
		state = std::shared_ptr<State>(new State());
		/* scan names */
		string name = SYNTAX_ROOT_NAME;
		state->name_list[name] = shared_ptr<GeneralSyntaxLeftHandSide>(new GeneralSyntaxLeftHandSide());
	}

	public:

	shared_ptr<GeneralSyntaxLeftHandSide> root;
	shared_ptr<RedundancyPlan> rp;

	void generate_input_dependent_syntax(shared_ptr<GeneralExample> example) {
		state->dep_list.each( [&](int handle, shared_ptr<GeneralSyntaxLeftHandSide> lhs) 
		{
			auto range = GeneralProgram::interpreter->get_range(handle, example);
			range.each( [&] (auto name)
			{
				auto new_rhs = shared_ptr<GeneralSyntaxRightHandSide>(new GeneralSyntaxRightHandSide());
				lhs->option.push_back(new_rhs);
				auto new_lhs = shared_ptr<GeneralSyntaxLeftHandSide>(new GeneralSyntaxLeftHandSide());
				new_lhs->is_term = true;
				new_lhs->functional = true;
				new_lhs->name = name;
				new_rhs->subexp.push_back(new_lhs);
				new_rhs->subexp_full.push_back(new_lhs);
			});
		});
	}

	void parse_config(shared_ptr<GJson> json) {
		init_parsing_state();
		scan_names(json);
		parse_grammar(json);
		parse_abstract_program(json);
		rp = parse_redundancy_plan(json);
		root = state->name_list[SYNTAX_ROOT_NAME];
	}
};

#endif
