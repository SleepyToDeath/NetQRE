#ifndef SEARCH_TREE_H
#define SEARCH_TREE_H

#include "syntax_tree.h"

enum SearchNodeColor {
	White, Black, Gray
};

class LNode;
class RNode;
class DNode;
class SearchState;

class SearchTree {
	pulic:
	SearchTree(ExampleType* example); /* top level search */
	SearchTree(SearchTreeContext ctxt, SearchState init_state); /* recursive search */
	bool accept(SyntaxTree* t);

	private:
	std::map<SearchState,LNode*> cache;
	SearchTreeContext ctxt;
	LNode* root;
}

class SearchTreeNode
{
	public:
	SearchState state;

	SearchTreeNode();
	virtual bool search(SearchTreeContext ctxt) = 0;
	bool is_feasible();
	SearchNodeColor get_color();

	protected:
	SearchNodeColor color;
	bool feasible;
};

class LNode: public SearchTreeNode {
	public:
	SyntaxLeftHandSide* syntax;
	std::vector<DNode*> option;

	bool search(SearchTreeContext ctxt) = 0;
	bool accept(SyntaxTree* t);
};

class DNode: public SearchTreeNode {
	public:
	SyntaxRightHandSide* syntax;
	DivideStrategy* divider;
	std::vector<RNode*> option;

	bool search(SearchTreeContext ctxt) = 0;
};

class RNode: public SearchTreeNode {
	public:
	SyntaxRightHandSide* syntax;
	std::vector<LNode*> subexp;

	bool search(SearchTreeContext ctxt) = 0;
};

class SearchTreeContext {
	std::map<SearchState,LNode*>* cache;
};

/* ========================= Domain Specific Contents ==================== */

/* one for each language */
class ExampleType {
	public:
	virtual SearchState to_init_state() = 0;
};


/* one for each language */
class SearchState {
};

/* one for each right hand side option */
class DivideStrategy {
	public:
	/* for independent rule */
	/* substates[i,j]: in ith dividing option, jth subexp get this state */
	virtual std::vector< std::vector<SearchState> > get_indep_substates(SearchState s) = 0;

	/* for dependent rule */
	/* We model this kind of search as finding proper smaller non-overlapping intervals
		that togeter FULLY covers a whole larger interval.
		get_min returns the lower bound of the larger interval.
		get_max returns the upper bound of the larger interval.
		get_dep_substates returns all possible substates that can cover a small interval.
		All intervals here are left-closed right-open. */
		
	virtual int get_min(SearchState s) = 0;
	virtual int get_max(SearchState s) = 0;
	virtual std::vector<SearchState> get_dep_substates(SearchState s, int min, int max) = 0;
};

/* ======================================================================= */

#endif
