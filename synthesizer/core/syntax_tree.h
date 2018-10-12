#ifndef SYNTAX_TREE_H
#define SYNTAX_TREE_H
//#define DEBUG_PRINT
//#define DEBUG_PRINT_2
//#define DEBUG_PRINT_4
//#define DEBUG_PRINT_5
#define DEBUG_PRINT_9
#define DEBUG_PRINT_8

#include <vector>
#include <string>
#include <iostream>
#include <memory>
#include <map>

using std::shared_ptr;
using std::vector;

class SyntaxRightHandSide;
class SyntaxLeftHandSide;
class SyntaxTreeNode;

enum SyntaxTreeCompleteness {
	UNKNOWN, COMPLETE, INCOMPLETE
};

class SyntaxTreeFactory;
class SyntaxTreeTemplate;

/*	This data structure is immutable.
	Every time it mutates, you get a new one */
class SyntaxTree: public std::enable_shared_from_this<SyntaxTree> {
	public:
	std::vector< shared_ptr<SyntaxTree> > subtree; // read only
	shared_ptr<SyntaxTreeNode> root; // read only
	double weight;

	SyntaxTree(shared_ptr<SyntaxTreeNode> root, int depth);
	SyntaxTree(shared_ptr<SyntaxTree> src); /* copy constructor */
	~SyntaxTree();

	class Queue
	{
		public:
		std::vector< shared_ptr<SyntaxTree> > q;
	};

	class VariableMap
	{
		public:
		/* <var name, var value> */
		std::map<std::string, shared_ptr<SyntaxTree> > map;
	};

	/* mutate a node of depth AT MOST `max_depth` into all possible RHS, and append the results to `queue` */
	bool multi_mutate(shared_ptr<SyntaxTree> root, int max_depth, shared_ptr<Queue> queue);

	bool contain_prefix(shared_ptr<SyntaxTreeTemplate> temp);

	shared_ptr<SyntaxTree> search_and_replace(shared_ptr<SyntaxTreeTemplate> temp_src, shared_ptr<SyntaxTreeTemplate> temp_dst);

	/* check if all leaf nodes are terminal */
	bool is_complete();
	virtual double get_complexity();
	virtual std::string to_string();
	size_t hash();
	bool equal(shared_ptr<SyntaxTree> t);

	static std::unique_ptr<SyntaxTreeFactory> factory;

	protected:

	double complexity;
	int depth; // depth of the root
	size_t hash_value = 0;
	SyntaxTreeCompleteness complete;

	private:

	bool real_search_and_replace(shared_ptr<SyntaxTreeTemplate> temp_src, shared_ptr<SyntaxTreeTemplate> temp_dst);
	void mutate(int option);
	virtual void copy_initializer(shared_ptr<SyntaxTree> src);
	void collect_variable(shared_ptr<VariableMap> vars, shared_ptr<SyntaxTreeTemplate> temp);
};

bool compare_syntax_tree_complexity(shared_ptr<SyntaxTree> a, shared_ptr<SyntaxTree> b);

class CmpSyntaxTree {
	public:
	bool operator ()(const shared_ptr<SyntaxTree> a, const shared_ptr<SyntaxTree> b) const;
};

class SyntaxTreeNode {

	public:
	SyntaxTreeNode(shared_ptr<SyntaxLeftHandSide> type);
	SyntaxTreeNode(shared_ptr<SyntaxTreeNode> src);
	shared_ptr<SyntaxLeftHandSide> get_type();
	void set_option(int option);
	int get_option();

	bool equal(shared_ptr<SyntaxTreeNode> n);
	
	private:
	shared_ptr<SyntaxLeftHandSide> type;
	int option;
};

class SyntaxTreeTemplate: public SyntaxTree {
	public:
	std::string var_name;
	bool is_variable(); /* non-terminal and no option in a template means a variable */
	shared_ptr<SyntaxTree> to_syntax_tree(shared_ptr<VariableMap> vars, int _depth);
};

class SyntaxLeftHandSide: public std::enable_shared_from_this<SyntaxLeftHandSide> {
	public:
	int id;
	int size();
	std::string name;
	std::vector<std::shared_ptr<SyntaxRightHandSide> > option;
	bool is_term;

	static const int NoOption = -1;
};

class SyntaxRightHandSide: public std::enable_shared_from_this<SyntaxRightHandSide> {
	public:
	int id;
	int size();
	std::string name;
	bool independent; /* only support one dependent subexp, which must be the only subexp */
	std::vector<std::shared_ptr<SyntaxLeftHandSide> > subexp;

	virtual std::string to_string(std::vector<std::string> subs) {return "";};
};

class SyntaxTreeFactory {
	public:

	virtual shared_ptr<SyntaxTree> get_new( shared_ptr<SyntaxTreeNode> root, int depth) 
	{
		return shared_ptr<SyntaxTree>(new SyntaxTree(root,depth));
	}

	virtual shared_ptr<SyntaxTree> get_new( shared_ptr<SyntaxTreeNode> root) 
	{
		return shared_ptr<SyntaxTree>(new SyntaxTree(root,0));
	}

	virtual shared_ptr<SyntaxTree> get_new( shared_ptr<SyntaxTree> src) 
	{
		return shared_ptr<SyntaxTree>(new SyntaxTree(src));
	}
};


#endif
