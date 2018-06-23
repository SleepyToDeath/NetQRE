#ifndef SEARCH_TREE_H
#define SEARCH_TREE_H

#include "syntax_tree.h"
#include <map>

enum SearchTreeColor {
	STWhite, STBlack, STGray
};

class LNode;
class RNode;
class DNode;
class SearchState;
template<class T>
class SearchTreeCache;
template<class T>
class SearchTreeCacheFactory;
class ExampleType;
class RHSToDivider;
class DivideStrategy;

class SearchTreeContext {
	public:
	RHSToDivider* r2d;
	ExampleType* example;
	int search_depth;
	SearchTreeCache<LNode*>* cache;
	SearchTreeCacheFactory<LNode*>* cache_pool;
};

/* 
	Different from syntax tree, this is a global structure.
	It stores the context for a task.
	So there's only one instance for each task.
*/
class SearchTree {
	public:
	SearchTree(SyntaxLeftHandSide* starting_symbol, 
				ExampleType* example, 
				RHSToDivider* r2d, 
				SearchTreeCacheFactory<LNode*>* cache_pool, 
				int search_depth); /* top level search */

	SearchTree(SyntaxLeftHandSide* starting_symbol, 
				SearchTreeContext ctxt, 
				SearchState* init_state); /* recursive search */

	LNode* get_root();
//	bool accept(SyntaxTree* t);
	bool search();

	private:
	SearchTreeContext ctxt;
	LNode* root;
};

class SearchTreeNode
{
	public:
	SearchState* state;

	virtual bool search(SearchTreeContext ctxt) = 0;
	bool is_feasible();
	SearchTreeColor get_color();

	protected:
	SearchTreeColor color;
	bool feasible;
};

class LNode: public SearchTreeNode {
	public:
	SyntaxLeftHandSide* syntax;
	std::vector<DNode*> option;

	LNode(SyntaxLeftHandSide* syntax, SearchState* state);
	bool search(SearchTreeContext ctxt);
	bool accept(SyntaxTree* t);
};

class DNode: public SearchTreeNode {
	public:
	SyntaxRightHandSide* syntax;
	DivideStrategy* divider;
	std::vector<RNode*> division;

	DNode(SyntaxRightHandSide* syntax, SearchState* state);
	bool search(SearchTreeContext ctxt);
};

class RNode: public SearchTreeNode {
	public:
	SyntaxRightHandSide* syntax;
	DivideStrategy* divider;
	std::vector<LNode*> subexp;
	std::vector<SearchState*> substate;

	RNode(SyntaxRightHandSide* syntax, std::vector<SearchState*> substate);
	bool search(SearchTreeContext ctxt);
};

class RHSToDivider {
	public:
	DivideStrategy* get_divider(SyntaxRightHandSide* rhs);
	void register_divider(SyntaxRightHandSide* rhs, DivideStrategy* divider);

	private:
	std::map<SyntaxRightHandSide*, DivideStrategy*> r2dmap;
};
/* ========================= Domain Specific Contents ==================== */

/* one for each language */
class ExampleType {
	public:
	virtual SearchState* to_init_state() = 0;
	virtual bool match(SearchState* state, SyntaxLeftHandSide* terminal) = 0;
};


/* one for each language */
class SearchState {
};

template<class T>
class SearchTreeCacheFactory {
	public:
	virtual SearchTreeCache<T>* get_cache() = 0;
};

/* 
	one for each language
	simulate std::map 
	but key should be content of the state instead of the pointer
*/
template<class T>
class SearchTreeCache {
	public:
	virtual T& operator [](SearchState* state) = 0;
	virtual int count(SearchState* state) = 0;
};


/* one for each right hand side option */
class DivideStrategy {
	public:
	/* for independent rule */
	/* substates[i,j]: in ith dividing option, jth subexp get this state */
	virtual std::vector< std::vector<SearchState*> > get_indep_substates(SearchState* s) = 0;

	/* for dependent rule */
	/* We model this kind of search as finding proper smaller non-overlapping intervals
		that together FULLY covers a whole larger interval.
		e.g. the large interval can be [0,10), smaller intervals can be [0,3),[3,6),[6,10)
		get_min returns the lower bound of the larger interval.
		get_max returns the upper bound of the larger interval.
		get_dep_substates returns all possible substates that can cover a small interval.
		All intervals here are left-closed right-open. */
	virtual int get_min(SearchState* s) = 0;
	virtual int get_max(SearchState* s) = 0;
	virtual SearchState* get_dep_substates(SearchState* s, int min, int max) = 0;

	/* for gathering results */
	virtual bool valid_combination(std::vector<bool> valid_subexp) = 0;
};

/* ======================================================================= */

#endif
