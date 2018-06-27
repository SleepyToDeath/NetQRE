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
#ifdef DEBUG_PRINT
	ctxt.indent = ctxt.indent + "|  ";
	std::cout<<ctxt.indent<<"LNode "<<syntax->name<<"\n";
	state->print_state(ctxt.indent);
#endif
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
	/* if it's terminal, accept */
	if (syntax->is_term) 
	{
#ifdef DEBUG_PRINT
	std::cout<<"["<<syntax->name<<" "<<t->to_string()<<" Rej!]\n";
#endif
		return true;
	}
	/* if not mutated, return result of this node */
	if (t->root->get_option() == SyntaxLeftHandSide::NoOption)
	{
#ifdef DEBUG_PRINT
	std::cout<<"["<<syntax->name<<" "<<t->to_string()<<(feasible?" Acc!":" Rej!")<<"]\n";
#endif
		return feasible;
	}
	DNode* op = option[t->root->get_option()];
	/* if the mutation option is not feasible, reject */
	if (!(op->is_feasible()))
		return false;
	/* if any dividing option below the mutation option is accepted, accept*/
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
		else 
		{
			/* if it's dependent, there's only one sub-syntax-tree, and it must be accepted by all sub-search-trees */
			for (int j=0; j<div->subexp.size(); j++)
				valid_subexp.push_back(div->subexp[j]->accept(t->subtree[0]));
		}
		if (op->divider->valid_combination(state, valid_subexp))
		{
#ifdef DEBUG_PRINT
	std::cout<<"["<<op->division.size()<<" "<<div->subexp.size()<<" "<<i<<" "<<syntax->name<<" "<<t->to_string()<<" Acc!]\n";
#endif
			return true;
		}
	}
#ifdef DEBUG_PRINT
	std::cout<<"["<<syntax->name<<" "<<t->to_string()<<" Rej!]\n";
#endif
	return false;
}

DNode::DNode(SyntaxRightHandSide* syn, SearchState* s) {
	syntax = syn;
	state = s;
	color = STWhite;
	feasible = false;
}

class DNodeDAGVertex;
class DNodeDAGPath;

class DNodeDAG {
	public:
	/* link list */
	std::vector< std::vector<int> > edge;
	std::vector<int> fan_in;

	/* map from int(index) to vertex */
	std::vector< DNodeDAGVertex > vertex;

	/* search queue */
	std::vector< int > queue;
};

class DNodeDAGPath {
	public:
	std::vector<int> vertex;
};

class DNodeDAGVertex {
	public:
	/* valid paths ending in this vertex */
	std::vector< DNodeDAGPath > path;
};

bool DNode::search(SearchTreeContext ctxt) {
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
	std::cout<<ctxt.indent<<(feasible?std::string("Acc!"):std::string("Rej!"))<<"\n";
#endif
		return feasible;
	}

	if (syntax->independent)
	{
		bool flag = false;
		std::vector< std::vector< SearchState* > > strategy = divider->get_indep_substates(state);
#ifdef DEBUG_PRINT
	std::cout<<ctxt.indent<<"num of branch: "<<strategy.size()<<std::endl;
#endif
		for (int i=0; i<strategy.size(); i++)
		{
#ifdef DEBUG_PRINT
	std::cout<<ctxt.indent<<"branch: "<<i<<std::endl;
#endif
			RNode* div = new RNode(syntax, state, strategy[i]);
			division.push_back(div);
			div->search(ctxt);
			flag = flag || div->is_feasible();
		}
		feasible = flag;
	}
	else
	{
		int min = divider->get_min(state);
		int max = divider->get_max(state);
		DNodeDAG* dag = new DNodeDAG();

		/* step 0, allocate edges and vertices */
		for (int i=0; i<=max; i++)
		{
			dag->edge.push_back(std::vector<int>());
			dag->vertex.push_back(DNodeDAGVertex());
			dag->fan_in.push_back(0);
		}

		/* step 1, build edges */
		for (int i=min; i<max; i++)
			for (int j=i+1; j<=max; j++) 
			{
				SearchState* edge_state = divider->get_dep_substates(state, i, j);

				if (ctxt.cache->count(edge_state) == 0)
				{
					LNode* exp = new LNode( syntax->subexp[0], edge_state );
					(*ctxt.cache)[edge_state] = exp;
					exp->search(ctxt);
				}

				if ((*ctxt.cache)[edge_state]->is_feasible())
				{
					dag->edge[i].push_back(j);
					dag->fan_in[j]++;
				}
			}

		/* step 2, DP on DAG */
		dag->queue.push_back(min);
		DNodeDAGPath empty_path;
		empty_path.vertex.push_back(min);
		dag->vertex[min].path.push_back(empty_path);
		{
			int i=0;
			int end=0;
			while (i<=end)
			{
				int current = dag->queue[i];
				for (int j=0; j<dag->edge[current].size(); j++)
				{
					int next = dag->edge[current][j];
					for (int k=0; k<dag->vertex[current].path.size(); k++)
					{
						DNodeDAGPath candidate_path = dag->vertex[current].path[k];
						candidate_path.vertex.push_back(next);

						/*
							Mismatch condition can be reduced to match condition in this case.
							Just need proper dividing strategy
						*/
						bool flag = false;
						{
							SearchGraph g(ctxt.search_depth, syntax->subexp[0], nullptr, nullptr);
							std::vector<SearchState*> substate;
							for (int l=0; l<candidate_path.vertex.size()-1; l++)
								substate.push_back(divider->get_dep_substates(state, candidate_path.vertex[l], candidate_path.vertex[l+1]));
							flag = (g.search_recursive(ctxt, substate) != nullptr);
						}

						if (flag)
							dag->vertex[next].path.push_back(candidate_path);
					}
					dag->queue.push_back(next);
				}
				i++;
			}
		}
		
		/* step 3, convert path to division */
		if (dag->vertex[max].path.size() == 0)
		{
			feasible = false;
		}
		else
		{
			feasible = true;
			for (int i=0; i<dag->vertex[max].path.size(); i++)
			{
				DNodeDAGPath path = dag->vertex[max].path[i];
				std::vector<SearchState*> substate;
				for (int j=0; j<path.vertex.size()-1; j++)
					substate.push_back(divider->get_dep_substates(state, path.vertex[j], path.vertex[j+1]));
				RNode* div = new RNode(syntax, state, substate);
				div->search(ctxt);
				division.push_back(div);
			}
		}
	}
	color = STBlack;
#ifdef DEBUG_PRINT
	std::cout<<ctxt.indent<<(feasible?std::string("Acc!"):std::string("Rej!"))<<"\n";
#endif
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

