#include "syntax_tree.h"

SyntaxTree::SyntaxTree(SyntaxTreeNode* r) {
	root = r;
}

SyntaxTree::SyntaxTree(SyntaxTree* t) {
	root = new SyntaxTreeNode(t->root)
	for (int i=0; i<t->subtree.size()
		subtree[i] = new SyntaxTree(t->subtree[i]);
}

SyntaxTree::~SyntaxTree() {
	delete root;
	for (int i=0; i<subtree.size(); i++)
		delete subtree[i];
}

void SyntaxTree::mutate(int option) {
	root->set_option(option);
	SyntaxRightHandSide* r = root->get_type()->option[option];
	for (int i=0; i<r->subexp.size(); i++)
		subtree[i] = new SyntaxTree(new SyntaxTreeNode(r->subexp[i]));
}

SyntaxTreeNode::SyntaxTreeNode(SyntaxLeftHandSide* l) {
	type = l;
	option = SyntaxLeftHandSide::NoOption;
}

SyntaxTreeNode::SyntaxTreeNode(SyntaxTreeNode* src) {
	option = src->option;
	type = src->type;
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

