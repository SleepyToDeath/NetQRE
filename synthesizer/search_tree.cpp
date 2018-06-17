#include "search_tree.h"

SearchTree::SearchTree(SyntaxLeftHandSide* root_syntax, ExampleType* example) {
	ctxt = &cache;
	SearchState init_state = example->to_init_state();
	root = new LNode(root_syntax, init_state);
	cache[init_state] = root;
}

SearchTree::SearchTree(SyntaxLeftHandSide* root_syntax, SearchTreeContext ctxt0, SearchState init_state) {
	ctxt = ctxt0;
	if (ctxt.cache->count(init_state)>0)
		root = ctxt.cache[init_state];
	else {
		root = new LNode(root_syntax, init_state);
		ctxt.cache[init_state] = root;
	}
}

bool SearchTree::accept(SyntaxTree* t) {
	return root->accept(t);
}

bool search(SearchTreeContext ctxt) {
	return root->search(ctxt);
}

SearchTreeColor SearchTreeNode::get_color() {
	return color;
}

bool SearchTreeNode::is_feasible() {
	if (color == STBlack)
		return feasible;
	else 
		return false;
}

LNode::LNode(SyntaxLeftHandSide* syn, SearchState s) {
	syntax = syn;
	state = s;
	color = STWhite;
	feasible = false;
}

bool LNode::search(SearchTreeContext ctxt) {

}

bool LNode::accept(SyntaxTree* t) {

}

DNode::DNode(SyntaxRightHandSide* syn, SearchState s) {
	syntax = syn;
	state = s;
	color = STWhite;
	feasible = false;
}

bool DNode::search(SearchTreeContext ctxt) {

}

RNode::RNode(SyntaxRightHandSide* syn, SearchState s) {
	syntax = syn;
	state = s;
	color = STWhite;
	feasible = false;
}

bool RNode::search(SearchTreeContext ctxt) {

}
