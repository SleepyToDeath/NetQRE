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
class DNodeDAGVertex;
class DNodeDAGPath;
class DNodeDAG;

class SearchTreeContext {
	public:
#ifdef DEBUG_PRINT
	std::string indent;
#endif
	RHSToDivider* r2d;
	ExampleType* example;
	int search_depth;
	SearchTreeCache<LNode*>* cache;
	SearchTreeCacheFactory<LNode*>* cache_pool;
	std::vector<DNodeDAG*> dag;
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

	SearchTreeContext get_ctxt();

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

	RNode(SyntaxRightHandSide* syntax, SearchState* state, std::vector<SearchState*> substate);
	bool search(SearchTreeContext ctxt);
};


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
	virtual bool is_positive() = 0;
};


/* one for each language */
class SearchState {
	public:
	virtual void print_state(std::string indent) = 0;
	virtual bool is_positive() = 0;
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
	virtual int size() = 0;
};


/* one for each right hand side option */
class DivideStrategy {
	public:
	virtual bool valid_state(SearchState* s) = 0;
	/* for independent rule */
	/* substates[i,j]: in ith dividing option, jth subexp get this state */
	virtual std::vector< std::vector<SearchState*> > get_indep_substates(SearchState* s) = 0;

	/* for dependent rule */
	/* We model this kind of search as finding proper smaller non-overlapping intervals
		that together FULLY covers a whole larger interval.
		e.g. for string abaabaabba the large interval can be [0,10), 
		smaller intervals can be [0,3),[3,6),[6,10)
		corresponding to split aba | aba | abba 
		( (ab*a)* )
		get_min returns the lower bound of the larger interval.
		get_max returns the upper bound of the larger interval.
		get_dep_substates returns all possible substates that can cover a small interval.
		All intervals here are left-closed right-open. */
	/* Some language may need to do extra search to avoid ambiguity.
		e.g abaabaabaabba matching [0,9) with a kleene star (that matches till the longest possible string)
		correct: (aba)*
		incorrect: (ab*a)*
		substates: [0,3) yes, [3,6) yes, [6,9) yes
		extra states: [9,10) no, [9,11) no, [9,12) no, [9,13) no
		Use get_dep_extra_states() for that.*/
	virtual int get_min(SearchState* s) = 0;
	virtual int get_max(SearchState* s) = 0;
	virtual SearchState* get_dep_substates(SearchState* s, int min, int max) = 0;
	virtual std::vector<SearchState*> get_dep_extra_states(SearchState* s, SearchTreeContext ctxt) = 0;

	/* for gathering results */
	virtual bool valid_combination(SearchState* state, std::vector<bool> valid_subexp) = 0;
};

/* ======================================================================= */

#endif
