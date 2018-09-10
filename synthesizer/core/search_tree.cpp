#include "search_tree.h"
#include "search_graph.h"
#include <iostream>

SearchTree::SearchTree(SyntaxLeftHandSide* starting_symbol, 
						ExampleType* example, 
						RHSToDivider* r2d, 
						SearchTreeCacheFactory<LNode*>* cache_pool0, 
						int search_depth) {
#ifdef DEBUG_PRINT
	ctxt.indent = "";
#endif
	ctxt.example = example;
	ctxt.search_depth = search_depth;
	ctxt.r2d = r2d;
	ctxt.cache_pool = cache_pool0;
	ctxt.cache = cache_pool0->get_cache();
	SearchState* init_state = example->to_init_state();
	root = new LNode(starting_symbol, init_state);
	(*ctxt.cache)[init_state] = root;
}

SearchTree::SearchTree(SyntaxLeftHandSide* starting_symbol, SearchTreeContext ctxt0, SearchState* init_state) {
	ctxt = ctxt0;
	if (ctxt.cache->count(init_state)>0)
		root = (*ctxt.cache)[init_state];
	else 
	{
		root = new LNode(starting_symbol, init_state);
		(*ctxt.cache)[init_state] = root;
	}
}

/*
bool SearchTree::accept(SyntaxTree* t) {
	return root->accept(t);
}
*/

SearchTreeContext SearchTree::get_ctxt() {
	return ctxt;
}

bool SearchTree::search() {
	return root->search(ctxt);
}

LNode* SearchTree::get_root() {
	return root;
}

SearchTreeColor SearchTreeNode::get_color() {
	return color;
}

bool SearchTreeNode::is_feasible() {
	/* Don't need color condition since we assume it's a DAG, which has no back edge */
	/* now the color is used for on-demand generation of dependent nodes */
//	if (color == STBlack)
		return feasible;
//	else 
//		return false;
}

LNode::LNode(SyntaxLeftHandSide* syn, SearchState* s) {
	syntax = syn;
	state = s;
	color = STWhite;
	feasible = false;
}

bool LNode::search(SearchTreeContext ctxt) {
#ifdef DEBUG_PRINT
	ctxt.indent = ctxt.indent + "|  ";
	std::cout<<ctxt.indent<<"LNode "<<syntax->name<<"\n";
	state->print_state(ctxt.indent);
#endif
	if (state == nullptr)
	{
		color = STBlack;
		feasible = false;
		return feasible;
	}
	color = STGray;
	ctxt.search_depth--;

	if (syntax->is_term)
	{
		color = STBlack;
		feasible = ctxt.example->match(state, syntax);
#ifdef DEBUG_PRINT
	std::cout<<ctxt.indent<<(feasible?"Acc!":"Rej!")<<"\n";
#endif
		return feasible;
	}

	bool flag = false;
	for (int i=0; i<syntax->option.size(); i++)
	{
#ifdef DEBUG_PRINT
//	std::cout<<ctxt.indent<<"branch: "<<i<<std::endl;
#endif
		option.push_back(new DNode(syntax->option[i], state));
		flag = flag | option[i]->search(ctxt);
	}
	feasible = flag;
	color = STBlack;
#ifdef DEBUG_PRINT
	std::cout<<ctxt.indent<<(feasible?"Acc!":"Rej!")<<"\n";
#endif
	return feasible;
}

bool LNode::accept(SyntaxTree* t) {
	if (!feasible)
	{
		return false;
	}
	/* if it's terminal, accept */
	if (syntax->is_term) 
	{
		return true;
	}
	/* if not mutated, return result of this node */
	if (t->root->get_option() == SyntaxLeftHandSide::NoOption)
	{
//		if (state->is_positive())
			return feasible;
//		else
//			return !feasible;
	}
	DNode* op = option[t->root->get_option()];
	/* if the mutation option is not feasible, reject */
	if (!(op->is_feasible()))
		return false;
	/* if any dividing option below the mutation option is accepted, accept*/
	if (op->syntax->independent) 
	{
		int dsize = op->division.size();
		for (int i=0; i<dsize; i++)
		{
			RNode* div = op->division[i];
			std::vector<bool> valid_subexp;
			if (op->syntax->independent) 
			{
				/* if it's independent, each sub-search-tree must accept corresponding sub-syntax-tree */
				int esize = div->subexp.size();
				for (int j=0; j<esize; j++)
					valid_subexp.push_back(div->subexp[j]->accept(t->subtree[j]));
			}

			if (op->divider->valid_combination(state, valid_subexp))
			{
				return true;
			}
		}
		return false;
	}
	else
	{
		if (t->is_complete())
		{
			for (int i=0; i<op->extra.size(); i++)
			{
				if (op->extra[i]->accept(t->subtree[0]))
				{
					return false;
				}
			}
		}

		int min = op->divider->get_min(state);
		int max = op->divider->get_max(state);
		std::vector<bool> acc_pre(max-min+1, false);
		acc_pre[0] = true;

		for (int i0=min; i0<max; i0++)
		{
			int i = i0 - min;
			if (acc_pre[i])
			{
				for (int j0=i0+1; j0<=max; j0++)
				{
					int j = j0 - min;
					if (op->segment[i][j]!=nullptr)
					{
						acc_pre[j] = acc_pre[j] || op->segment[i][j]->is_feasible();
					}
				}
			}
		}
		if (!acc_pre[max-min])
		{
			return false;
		}

		std::vector<bool> acc(max-min+1, false);
		acc[0] = true;

/*
		for (int i0=min; i0<max; i0++)
		{
			int i = i0 - min;
			if (acc[i])
			{
				for (int j0=i0+1; j0<=max; j0++)
				{
					int j = j0 - min;
					if (op->segment[i][j]!=nullptr)
					{
						acc[j] = acc[j] || op->segment[i][j]->accept(t->subtree[0]);
					}
				}
			}
		}
		*/

		for (int j=1; j<max-min; j++)
			for (int i0=min; i0+j<=max; i0++)
			{
				int i = i0 - min;
				if (acc[i] && (op->segment[i][i+j]!=nullptr))
				{
					acc[i+j] = acc[i+j] || op->segment[i][i+j]->accept(t->subtree[0]);
				}
				if (acc[max-min])
				{
					return true;
				}
			}

		return acc[max-min];
	}
}

DNode::DNode(SyntaxRightHandSide* syn, SearchState* s) {
	syntax = syn;
	state = s;
	color = STWhite;
	feasible = false;
}

bool DNode::search(SearchTreeContext ctxt) {
	/* ========== Prepare ========== */

	if (state == nullptr)
	{
		color = STBlack;
		feasible = false;
		return feasible;
	}
#ifdef DEBUG_PRINT
	ctxt.indent = ctxt.indent + "|  ";
	std::cout<<ctxt.indent<<"DNode "<<syntax->name<<"\n";
	state->print_state(ctxt.indent);
#endif

	color = STGray;
	divider = ctxt.r2d->get_divider(syntax);

	if (!(divider->valid_state(state)))
	{
		feasible = false;
#ifdef DEBUG_PRINT
	std::cout<<ctxt.indent<<"1"<<(feasible?std::string("Acc!"):std::string("Rej!"))<<"\n";
#endif
		return feasible;
	}

	if (syntax->independent)
	{
		/* ========== Handle Independent ========== */
		bool flag = false;
		std::vector< std::vector< SearchState* > > strategy = divider->get_indep_substates(state);
		for (int i=0; i<strategy.size(); i++)
		{
			RNode* div = new RNode(syntax, state, strategy[i]);
			division.push_back(div);
			div->search(ctxt);
			flag = flag || div->is_feasible();
		}
		feasible = flag;
	}
	else
	{
		/* ========== Handle Dependent ========== */
		int min = divider->get_min(state);
		int max = divider->get_max(state);
		for (int i=min; i<max; i++)
		{
			std::vector<LNode*> row(i-min+1, nullptr);
			for (int j=i+1; j<=max; j++)
			{
				SearchState* substate = divider->get_dep_substate(state, i, j);
				if (substate!=nullptr)
				{
					if (ctxt.cache->count(substate)==0)
					{
						LNode* exp = new LNode(syntax->subexp[0],substate);
						(*(ctxt.cache))[substate] = exp;
						exp->search(ctxt);
					}
					row.push_back((*ctxt.cache)[substate]);
				}
				else
				{
					row.push_back(nullptr);
				}
			}
			segment.push_back(row);
		}

		std::vector<SearchState*> extra_state = divider->get_dep_extra_states(state, ctxt);
		for (int i=0; i<extra_state.size(); i++)
		{
			SearchState* substate = extra_state[i];
			if (substate!=nullptr)
			{
				if (ctxt.cache->count(substate)==0)
				{
					LNode* exp = new LNode(syntax->subexp[0],substate);
					(*(ctxt.cache))[substate] = exp;
					exp->search(ctxt);
				}
				extra.push_back((*ctxt.cache)[substate]);
			}
		}

		feasible = true;
	}

#ifdef DEBUG_PRINT
	std::cout<<ctxt.indent<<"1"<<(feasible?std::string("Acc!"):std::string("Rej!"))<<"\n";
#endif

	color = STBlack;
	return feasible;
}

RNode::RNode(SyntaxRightHandSide* syn, SearchState* state0, std::vector<SearchState*> substate0) {
	syntax = syn;
	state = state0;
	substate = substate0;
	color = STWhite;
	feasible = false;
}

bool RNode::search(SearchTreeContext ctxt) {
#ifdef DEBUG_PRINT
	ctxt.indent = ctxt.indent + "|  ";
	std::cout<<ctxt.indent<<"RNode "<<syntax->name<<"\n";
	for (int i=0; i<substate.size(); i++)
		substate[i]->print_state(ctxt.indent);
#endif
	if (state == nullptr)
	{
		color = STBlack;
		feasible = false;
		return feasible;
	}
	color = STGray;
	divider = ctxt.r2d->get_divider(syntax);
	std::vector<bool> valid_subexp;
	for (int i=0; i<substate.size(); i++)
	{
		if (ctxt.cache->count(substate[i])==0)
		{
			LNode* exp = new LNode(syntax->subexp[syntax->independent?i:0],substate[i]);
			(*(ctxt.cache))[substate[i]] = exp;
			exp->search(ctxt);
		}
		subexp.push_back((*ctxt.cache)[substate[i]]);
		valid_subexp.push_back(subexp[i]->is_feasible());
	}
	feasible = divider->valid_combination(state, valid_subexp);
	color = STBlack;
#ifdef DEBUG_PRINT
	std::cout<<ctxt.indent<<(feasible?"Acc!":"Rej!")<<"\n";
#endif
	return feasible;
}

DivideStrategy* RHSToDivider::get_divider(SyntaxRightHandSide* rhs)
{
	return r2dmap[rhs];
}

void RHSToDivider::register_divider(SyntaxRightHandSide* rhs, DivideStrategy* divider)
{
	r2dmap[rhs] = divider;
}

