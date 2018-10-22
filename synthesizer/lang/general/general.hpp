#ifndef LANG_GENERAL_HPP
#define LANG_GENERAL_HPP

#include "../../core/incomplete_execution.h"
#include "../../core/redundancy.h"
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

class AbstractCode {
	public:
	AbstractCode(std::string _pos, std::string _neg):pos(_pos),neg(_neg){}
	std::string pos;
	std::string neg;
};

class GeneralInterpreter {
	public:
	virtual bool accept(AbstractCode code, bool complete,  shared_ptr<GeneralExample> input, IEConfig cfg) = 0;
	virtual double extra_complexity(AbstractCode code) { return 0.0; }
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
	GeneralProgram(AbstractCode _src, bool _complete):
	source_code(_src), complete(_complete) {
	}

	bool accept( shared_ptr<IEExample> input, IEConfig cfg = DEFAULT_IE_CONFIG) {
		return interpreter->accept(source_code, complete, std::static_pointer_cast<GeneralExample>(input), cfg);
	}

	/* set this to the interpreter of your specific language */
	static std::unique_ptr<GeneralInterpreter> interpreter;

	bool complete;
	AbstractCode source_code;
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

	public:

	/* dummy initializer */
	GeneralSyntaxLeftHandSide() {
		is_term = true;
	}

	bool is_functional() { 
		return functional;
	}

	std::string positive_abstract_code;
	std::string negative_abstract_code;
	bool functional; /* example of non-functional symbol : "(", ")", ",", non-functional symbols start with "$" */

	private:


	shared_ptr<IEProgram> to_program() {return nullptr;}
};

class GeneralConfigParser {
	public:

	class State {
		public:
		map<std::string, shared_ptr<GeneralSyntaxLeftHandSide> > name_list;
	};

	class Token {
		public:
		bool variable;
		std::string name;
		shared_ptr<GeneralSyntaxLeftHandSide> lhs;
	};

	shared_ptr<State> state;
	shared_ptr<GeneralSyntaxLeftHandSide> root;
	shared_ptr<RedundancyPlan> rp;

	bool is_id(char c) {
		return (c>='0' && c<='9') || (c>='a' && c<='z') || (c>='A' && c<='Z') || (c=='_');
	}

	shared_ptr<Token> tokenize_next(string code, shared_ptr<int> cursor) {
		auto t = shared_ptr<Token>(new Token());
		int i = (*cursor);

		cout<<code.length()<<endl;

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
		shared_ptr<GJson> syntax = json->get(0)->value();
		/* syntax */
		for (int i=0; i<syntax->size(); i++)
		{
			/* Add LHS to the name list
			 * LHS must be non-terminal and functional */
			std::string name = syntax->get(i)->name();
			if (state->name_list.count(name) == 0)
			{
				cout<<"new name: "<<name<<endl;
				state->name_list[name] = shared_ptr<GeneralSyntaxLeftHandSide>(new GeneralSyntaxLeftHandSide());
			}
			state->name_list[name]->name = name;
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
					std::string real_name2 = name2;
					if (name2[0] == '$')
						real_name2 = name2.substr(1, name2.length()-1);
					if (state->name_list.count(real_name2) == 0)
					{
						cout<<"new name: "<<real_name2<<endl;
						state->name_list[real_name2] = shared_ptr<GeneralSyntaxLeftHandSide>(new GeneralSyntaxLeftHandSide());
						state->name_list[real_name2]->name = real_name2;
					}
					if (name2[0] == '$')
					{
						state->name_list[real_name2]->functional = false;
					}
					else
						state->name_list[real_name2]->functional = true;
				}
			}
		}
	}

	shared_ptr<SyntaxTreeTemplate> parse_template_recursive(string code, shared_ptr<int> cursor) {
		shared_ptr<Token> t = tokenize_next(code, cursor);
		cout<<t->variable<<" | "<<t->name<<endl;
		if (t->variable || t->lhs->is_term)
		{
			auto root = shared_ptr<SyntaxTreeNode>(new SyntaxTreeNode(t->lhs));
			auto temp = shared_ptr<SyntaxTreeTemplate>(new SyntaxTreeTemplate(root));
			temp->var_name = t->name;
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
						cout<<"name: "<<temp->root->get_type()->name<<endl;
						cout<<"subtree size"<<temp->subtree.size()<<endl;
						cout<<"full subtree size"<<subexp_full.size()<<endl;
						*/
						temp->root->set_option(i);
						return temp;
					}
				}
			}
		}
		throw string("Unable to parse template. Invalid program.\n");
		return nullptr;
	}

	shared_ptr<SyntaxTreeTemplate> parse_template(std::string code) {
		auto cursor = shared_ptr<int>(new int(0));
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
					temp->all_example = true;
					break;
				case 'E':
					temp->all_example = false;
					break;
				default:
					throw string("Condition format incorrect.\n");
			}
			switch (condition[1]) {
				case 'A':
					temp->all_program = true;
					break;
				case 'E':
					temp->all_program = false;
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
			cout<<"CTemplate: "<< endl<<temp->temp->to_string()<<endl;
			for (int i=0; i<temp->checklist.size(); i++)
				cout<<temp->checklist[i]->to_string()<<" | ";
			cout<<endl;
		}

		/* parse unconditional */
		for (int i=0; i<ucnd_json->size(); i++)
		{
			auto temp_json = ucnd_json->get(i);
			auto temp = shared_ptr<UnconditionalRedundancyTemplate>(new UnconditionalRedundancyTemplate());
			temp->temp_src = parse_template(temp_json->name());
			temp->temp_dst = parse_template(temp_json->value()->name());

			plan->ucnd.push_back(temp);
			cout<<"UTemplate: " <<endl<<temp->temp_src->to_string()<<endl;
			cout<<temp->temp_dst->to_string()<<endl;
		}

		return plan;
	}

	void parse_abstract_program(shared_ptr<GJson> json) {
		shared_ptr<GJson> pos_json = json->get(1)->value();
		shared_ptr<GJson> neg_json = json->get(2)->value();
		for (int i=0; i<pos_json->size(); i++)
		{
			std::string name = pos_json->get(i)->name();
			state->name_list[name]->positive_abstract_code = pos_json->get(i)->value()->name();
		}
		for (int i=0; i<neg_json->size(); i++)
		{
			std::string name = neg_json->get(i)->name();
			state->name_list[name]->negative_abstract_code = neg_json->get(i)->value()->name();
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
//					cout<<lhs->name<<"-->"<<name<<endl;
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

	void parse_config(shared_ptr<GJson> json) {
		init_parsing_state();
		scan_names(json);
		parse_grammar(json);
		parse_abstract_program(json);
		rp = parse_redundancy_plan(json);
		root = state->name_list[SYNTAX_ROOT_NAME];
	}
};

class GeneralSyntaxTree : public IESyntaxTree {
	public:
	GeneralSyntaxTree(shared_ptr<SyntaxTree> src):	IESyntaxTree(src) {}
	GeneralSyntaxTree(shared_ptr<SyntaxTreeNode> root, int depth): IESyntaxTree(root, depth) {}
	shared_ptr<IEProgram> to_program() {
		return shared_ptr<IEProgram>(new GeneralProgram(to_code(), is_complete()));
	}

	virtual double get_complexity() {
		if (complexity == 0)
		{
			if (root->get_type()->is_term)
			{
				complexity = -100.0;
			}
			else if (root->get_option() == SyntaxLeftHandSide::NoOption)
			{
				complexity = 200.0;
			}
			else
			{
				complexity = 0;
				for (int i=0; i<subtree.size(); i++)
					complexity += subtree[i]->get_complexity();
					/*
				if (subtree.size() > 2)
					complexity -= 200;
	//				complexity -= (subtree.size()) * 200;
				else
					*/
					complexity += (subtree.size()-1) * 150.0;
//				complexity -= 100.0;
			}
			if (depth == 0)
				complexity += GeneralProgram::interpreter->extra_complexity(to_code());
		}
		if (complexity == 0)
			complexity = 0.01;
		return complexity;
	}


	AbstractCode to_code() {
		std::string pos;
		std::string neg;
		if (root->get_type()->is_term) 
		{
			pos = root->get_type()->name;
			neg = pos;
		}
		else if (root->get_option() == SyntaxLeftHandSide::NoOption)
		{
			pos = std::static_pointer_cast<GeneralSyntaxLeftHandSide>(root->get_type())->positive_abstract_code;
			neg = std::static_pointer_cast<GeneralSyntaxLeftHandSide>(root->get_type())->negative_abstract_code;
		}
		else
		{
			auto rhs = std::static_pointer_cast<GeneralSyntaxRightHandSide> (root->get_type()->option[root->get_option()]);
			int j = 0;
			for (int i=0; i<rhs->subexp_full.size(); i++)	{
				if (rhs->subexp_full[i]->is_functional()) {
					auto sub = (std::static_pointer_cast<GeneralSyntaxTree>(subtree[j])->to_code());
					pos = pos + sub.pos;
					neg = neg + sub.neg;
					j++;
				}
				else {
					pos = pos + (rhs->subexp_full[i]->name);
					neg = neg + (rhs->subexp_full[i]->name);
				}
			}
		}
		return AbstractCode(pos, neg);
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

	shared_ptr<SyntaxTree> get_new( shared_ptr<SyntaxTreeNode> root, int depth) 
	{
		return shared_ptr<GeneralSyntaxTree>(new GeneralSyntaxTree(root, depth));
	}

	shared_ptr<SyntaxTree> get_new( shared_ptr<SyntaxTreeNode> root) 
	{
		return shared_ptr<GeneralSyntaxTree>(new GeneralSyntaxTree(root, 0));
	}

	shared_ptr<SyntaxTree> get_new( shared_ptr<SyntaxTree> src) 
	{
		return shared_ptr<GeneralSyntaxTree>(new GeneralSyntaxTree(src));
	}
};

std::unique_ptr<SyntaxTreeFactory> SyntaxTree::factory = unique_ptr<GeneralSyntaxTreeFactory>(new GeneralSyntaxTreeFactory());

#endif
