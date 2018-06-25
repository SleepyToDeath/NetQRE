#include "syntax_tree.h"

SyntaxTree::SyntaxTree(SyntaxTreeNode* r) {
	root = r;
}

SyntaxTree::SyntaxTree(SyntaxTree* t) {
	root = new SyntaxTreeNode(t->root);
	for (int i=0; i<t->subtree.size(); i++)
		subtree.push_back(new SyntaxTree(t->subtree[i]));
#ifdef DEBUG_PRINT
//	std::cout<< "copying syntax tree\n";
//	std::cout<<"src: "<<t->to_string()<<"\n";
//	std::cout<<"dst: "<<to_string()<<"\n";
#endif 
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

bool SyntaxTree::complete() {

	if (root->get_type()->is_term)
		return true;

	if (root->get_option() == SyntaxLeftHandSide::NoOption)
		return false;

	for (int i=0; i<subtree.size(); i++)
		if (!subtree[i]->complete())
			return false;

	return true;
}

bool SyntaxTree::multi_mutate(SyntaxTree* top, int max_depth, std::vector<SyntaxTree*> * queue) {

	if (root->get_type()->is_term)
		return false;

	if (max_depth >= 0)
	{
		if (root->get_option() == SyntaxLeftHandSide::NoOption)
		{
			for (int i=0; i<root->get_type()->option.size(); i++)
			{
				mutate(i);
				queue->push_back(new SyntaxTree(top));
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
		if (subtree.size() > 1)
			s = root->get_type()->option[root->get_option()]->name + "(";
		for (int i=0; i<this->subtree.size(); i++)
			s = s+subtree[i]->to_string() + (i==subtree.size()-1?"":",");
		if (subtree.size() > 1)
			s = s + ")";
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

