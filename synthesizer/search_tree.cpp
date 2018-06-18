#include "search_tree.h"

SearchTree::SearchTree(SyntaxLeftHandSide* root_syntax, ExampleType* example, int search_depth) {
	ctxt->cache = &cache;
	ctxt->search_depth = search_depth;
	SearchState* init_state = example->to_init_state();
	root = new LNode(root_syntax, init_state);
	cache[init_state] = root;
}

SearchTree::SearchTree(SyntaxLeftHandSide* root_syntax, SearchTreeContext ctxt0, SearchState* init_state) {
	ctxt = ctxt0;
	if (ctxt.cache->count(init_state)>0)
		root = ctxt.cache[init_state];
	else 
	{
		root = new LNode(root_syntax, init_state);
		ctxt.cache[init_state] = root;
	}
}

bool SearchTree::accept(SyntaxTree* t) {
	return root->accept(t);
}

bool SearchTree::search(SearchTreeContext ctxt) {
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

LNode::LNode(SyntaxLeftHandSide* syn, SearchState* s) {
	syntax = syn;
	state = s;
	color = STWhite;
	feasible = false;
}

bool LNode::search(SearchTreeContext ctxt) {
	color = STGray;
	ctxt->search_depth--;

	if (syntax->is_term)
		return ctxt.example->match(state, syntax);

	bool flag = false;
	for (int i=0; i<syntax->option.size(); i++)
	{
		option.push_back = new DNode(syntax->option[i], state);
		flag = flag | option[i]->search(ctxt);
	}
	feasible = flag;
	color = STBlack;
}

bool LNode::accept(SyntaxTree* t) {
	/* if it's terminal, accept */
	if (syntax->is_term) 
		return true;
	DNode* op = option[t->root->get_option()];
	/* if the mutation option is not feasible, reject */
	if (!(op->is_feasible()))
		return false;
	/* if any dividing option below the mutation option is accepted, accept*/
	for (int i=0; i<op->division.size(); i++)
	{
		RNode* div = op->division[i];
		bool flag = true;
		if (op->syntax->independent) 
		{
			/* if it's independent, each sub-search-tree must accept corresponding sub-syntax-tree */
			for (int j=0; j<div->subexp.size(); j++)
				if (!(div->subexp[j]->accep(t->subtree[j])))
				{
					flag = false;
					break;
				}
		}
		else 
		{
			/* if it's dependent, there's only one sub-syntax-tree, and it must be accepted by all sub-search-trees */
			for (int j=0; j<div->subexp.size(); j++)
				if (!(div->subexp[j]->accept(t->subtree[0])))
				{
					flag = false;
					break;
				}
		}
		if (flag)
			return true;
	}
	return false;
}

DNode::DNode(SyntaxRightHandSide* syn, SearchState* s) {
	syntax = syn;
	state = s;
	color = STWhite;
	feasible = false;
}

bool DNode::search(SearchTreeContext ctxt) {
	color = STGray;
	bool flag = false;
	if (syntax->independent)
	{
		std::vector< std::vector< SearchState* > > strategy = get_indep_substates();
		for (int i=0; i<strategy.size(); i++)
		{
			RNode* div = new RNode(syntax, strategy[i]);
			division.push_back = div;
			flag = flag || div->search(ctxt);
		}
	}
	else
	{
		/* [TODO] reuse search graph */
	}
	color = STBlack;
	return flag;
}

RNode::RNode(SyntaxRightHandSide* syn, SearchState* s) {
	syntax = syn;
	state = s;
	color = STWhite;
	feasible = false;
}

bool RNode::search(SearchTreeContext ctxt) {
	color = STGray;
	bool flag = true;
	for (int i=0; i<substate.size(); i++)
	{
		if (ctxt->cache->count(substate[i])==0)
		{
			LNode* exp = new LNode(syntax->subexp[syntax->independent?i:0],substate[i]);
			ctxt->cache[substate[i]] = exp;
			exp->search(ctxt);
		}
		subexp.push_back(ctxt->cache[substate[i]]);
		flag = flag && subexp[i]->is_feasible();
	}
	color = STBlack;
}
