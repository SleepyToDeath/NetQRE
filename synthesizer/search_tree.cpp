#include "search_tree.h"

SearchTree::SearchTree(SyntaxLeftHandSide* starting_symbol, ExampleType* example, RHSToDivider* r2d, int search_depth) {
	ctxt->search_depth = search_depth;
	ctxt->r2d = r2d;
	ctxt->cache_pool = cache_pool0;
	ctxt->cache = cache_pool0->get_cache();
	SearchState* init_state = example->to_init_state();
	root = new LNode(starting_symbol, init_state);
	(*cache)[init_state] = root;
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

bool SearchTree::search() {
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

class DNodeDAGVertex;
class DNodeDAGPath;

class DNodeDAG {
	public:
	/* link list */
	std::vector< std::vector<int> > edge;
	std::vector<int> fan_in;

	/* map from int(index) to vertex */
	std::vector< DNodeDAGNode > vertex;

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
	color = STGray;
	bool flag = false;
	divider = ctxt->r2d->get_divider(syntax);
	if (syntax->independent)
	{
		std::vector< std::vector< SearchState* > > strategy = divider->get_indep_substates();
		for (int i=0; i<strategy.size(); i++)
		{
			RNode* div = new RNode(syntax, strategy[i]);
			division.push_back = div;
			flag = flag || div->search(ctxt);
		}
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
			dag->vertex.push_back(DNodeDAGNode());
			fan_in.push_back(0);
		}

		/* step 1, build edges */
		for (int i=min; i<max; i++)
			for (int j=i+1; j<=max; j++) 
			{
				SearchState* edge_state = divider->get_dep_substates(state, i, j);

				if (ctxt->cache->count(edge_state) == 0)
				{
					LNode* exp = new LNode( syntax->subexp[0], edge_state );
					(*ctxt->cache)[edge_state] = exp;
					exp->search(ctxt);
				}

				if (ctxt->cache->is_feasible())
				{
					dag->edge[i].push_back(j);
					dag->fan_in[j]++;
				}
			}

		/* step 2, DP on DAG */
		dat->queue.push_back(min);
		DNodeDAGPath empty_path;
		empty_path.push_back(min);
		dag->vertex[min].path.push_back(empty_path);
		{
			int i=0;
			int end=0;
			while (i<=end)
			{
				int current = queue[i];
				for (j=0; j<dag->edge[current].size(); j++)
				{
					int next = dag->edge[current][j];
					for (k=0; k<dag->vertex[current].path.size(); k++)
					{
						DNodeDAGPath candidate_path = dag->vertex[current].path[k];
						candidate_path.push_back(next);

						/*
							Mismatch condition can be reduced to match condition in this case.
							Just need proper dividing strategy
						*/
						bool flag = false;
						{
							SearchGraph g(ctxt.search_depth, syntax, nullptr, nullptr);
							std::vector<SearchState*> substate;
							for (int l=0; l<candidate_path.size()-1; l++)
								substate.push_back(divider->get_dep_substates(state, candidate_path[l], candidate_path[l+1]));
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
			is_feasible = false;
		}
		else
		{
			is_feasible = true;
			for (int i=0; i<dag->vertex[max].path.size(); i++)
			{
				DNodeDAGPath path = dag->vertex[max].path[i];
				std::vector<SearchState*> substate;
				for (int j=0; j<path.size()-1; j++)
					substate.push_back(divider->get_dep_substates(state, path[j], path[j+1]));
				RNode* div = new RNode(syntax, substate);
				div->search(ctxt);
				division.push_back(div);
			}
		}
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
	divider = ctxt->r2d->get_divider(syntax);
	std::vector<bool> valid_subexp;
	for (int i=0; i<substate.size(); i++)
	{
		if (ctxt->cache->count(substate[i])==0)
		{
			LNode* exp = new LNode(syntax->subexp[syntax->independent?i:0],substate[i]);
			(*ctxt->cache)[substate[i]] = exp;
			exp->search(ctxt);
		}
		subexp.push_back((*ctxt->cache)[substate[i]]);
		valid_.push_back(subexp[i]->is_feasible());
	}
	feasible = divider->valid_combination(valid_subexp);
	color = STBlack;
	return feasible;
}

DivideStrategy* RHSToDivider::get_divider(SyntaxRightHandSide* rhs)
{
	return r2dmap[rhs];
}

void RHSToDivide::register_divider(SyntaxRightHandSide* rhs, DivideStrategy* divider)
{
	r2dmap[rhs] = divider;
}

