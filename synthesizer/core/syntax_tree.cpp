#include "syntax_tree.h"
#include <cmath>

bool compare_syntax_tree(SyntaxTree* a, SyntaxTree* b) {
	return a->get_complexity() > b->get_complexity();
}

SyntaxTree::SyntaxTree(SyntaxTreeNode* r) {
	root = r;
	complete = UNKNOWN;
	complexity = 0;
}

SyntaxTree::SyntaxTree(SyntaxTree* t) {
	/* shared part */
	root = new SyntaxTreeNode(t->root);
	this->weight = t->weight;
	complete = UNKNOWN;
	complexity = 0;
	/* to be overridden */
	copy_initializer(t);
}

void SyntaxTree::copy_initializer(SyntaxTree* t) {
	for (int i=0; i<t->subtree.size(); i++)
		subtree.push_back(new SyntaxTree(t->subtree[i]));
}

SyntaxTree::~SyntaxTree() {
	delete root;
	for (int i=0; i<subtree.size(); i++)
		delete subtree[i];
}

void SyntaxTree::mutate(int option) {
	/* clean up previous mutation */
	if (root->get_option() != SyntaxLeftHandSide::NoOption)
	{
		while(subtree.size()>0)
		{
			delete subtree.back();
			subtree.pop_back();
		}
	}

	root->set_option(option);

	/* new mutation */
	if (option != SyntaxLeftHandSide::NoOption)
	{
		SyntaxRightHandSide* r = root->get_type()->option[option];
		for (int i=0; i<r->subexp.size(); i++)
			subtree.push_back(new SyntaxTree(new SyntaxTreeNode(r->subexp[i])));
	}

}

bool SyntaxTree::is_complete() {
	
	if (complete == UNKNOWN)
	{
		if (root->get_type()->is_term)
			complete = COMPLETE;
		else if (root->get_option() == SyntaxLeftHandSide::NoOption)
			complete = INCOMPLETE;
		else
		{
			complete = COMPLETE;
			for (int i=0; i<subtree.size(); i++)
				if (!subtree[i]->is_complete())
				{
					complete = INCOMPLETE;
					break;
				}
		}
	}
	return complete == COMPLETE;
}

double SyntaxTree::get_complexity() {
	if (complexity == 0)
	{
		if (root->get_type()->is_term)
			complexity = 1;
		else if (root->get_option() == SyntaxLeftHandSide::NoOption)
		{
			complexity = root->get_type()->option.size();
//			complexity = 1;
		}
		else
		{
			complexity = 0;
			for (int i=0; i<subtree.size(); i++)
				complexity += subtree[i]->get_complexity();
			if (root->get_type()->option[root->get_option()]->independent)
			{
				complexity *= subtree.size();
			}
			else
			{
				complexity *= sqrt(complexity*4);
			}
		}
	}
	return complexity;
}

bool SyntaxTree::multi_mutate(SyntaxTree* top, int max_depth, std::vector<SyntaxTree*> * queue) {

	if (root->get_type()->is_term)
		return false;

	if (max_depth >= 0)
	{
		if (root->get_option() == SyntaxLeftHandSide::NoOption)
		{
			int branch_num = root->get_type()->option.size();
			for (int i=0; i<branch_num; i++)
			{
				mutate(i);
				queue->push_back(new SyntaxTree(top));
				queue->back()->weight = top->weight/branch_num;
			}
			mutate(SyntaxLeftHandSide::NoOption);
			return true;
		}
		else
		{
			for (int i=0; i<subtree.size(); i++)
				if (subtree[i]->multi_mutate(top, max_depth-1, queue))
					return true;
			return false;
		}
	}
	else
	{
		return false;
	}
}

std::string SyntaxTree::to_string() {
	std::string s;
	if (root->get_type()->is_term) 
		s = root->get_type()->name;
	else if (root->get_option() == SyntaxLeftHandSide::NoOption)
		s = root->get_type()->name;
	else
	{
//		if ((subtree.size() > 1) || (!root->get_type()->option[root->get_option()]->independent))
		if ((!root->get_type()->option[root->get_option()]->independent))
			s = root->get_type()->option[root->get_option()]->name + "( ";
		for (int i=0; i<this->subtree.size(); i++)
			s = s+subtree[i]->to_string() + (i==subtree.size()-1?"":" ");
//		if ((subtree.size() > 1) || (!root->get_type()->option[root->get_option()]->independent))
		if ((!root->get_type()->option[root->get_option()]->independent))
			s = s + " )";
	}
	return s;
}

bool SyntaxTree::equal(SyntaxTree* t) {
	if (!root->equal(t->root))
		return false;
	if (subtree.size() != t->subtree.size())
		return false;
	for (int i=0; i<subtree.size(); i++)
		if (!subtree[i]->equal(t->subtree[i]))
			return false;
	return true;
}

SyntaxTreeNode::SyntaxTreeNode(SyntaxLeftHandSide* l) {
	type = l;
	option = SyntaxLeftHandSide::NoOption;
}

SyntaxTreeNode::SyntaxTreeNode(SyntaxTreeNode* src) {
	option = src->option;
	type = src->type;
}

bool SyntaxTreeNode::equal(SyntaxTreeNode* n) {
	return (option == n->option) && (type == n->type);
}

SyntaxLeftHandSide* SyntaxTreeNode::get_type() {
	return type;
}

void SyntaxTreeNode::set_option(int op) {
	option = op;
}

int SyntaxTreeNode::get_option() {
	return option;
}

int LanguageSyntax::add_rule(SyntaxLeftHandSide* r) {
	rule.push_back(r);
	return rule.size()-1;
}

SyntaxLeftHandSide* LanguageSyntax::get_rule(int id) {
	return rule[id];
}

int LanguageSyntax::size() {
	return rule.size();
}

int SyntaxLeftHandSide::size() {
	return option.size();
}

int SyntaxRightHandSide::size() {
	return subexp.size();
}

