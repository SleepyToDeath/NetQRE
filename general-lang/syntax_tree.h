#ifndef SYNTAX_TREE_H
#define SYNTAX_TREE_H
//#define DEBUG_PRINT
//#define DEBUG_PRINT_2
//#define DEBUG_PRINT_4
//#define DEBUG_PRINT_5
#define DEBUG_PRINT_9
#define DEBUG_PRINT_8

#include <string>
#include <iostream>
#include <memory>
#include <map>
#include "rubify.hpp"

using std::shared_ptr;
using Rubify::vector;

class SyntaxRightHandSide;
class SyntaxLeftHandSide;
class SyntaxTreeNode;

enum SyntaxTreeCompleteness {
	UNKNOWN, COMPLETE, INCOMPLETE
};

const std::string MarshallDelimiter = ",";
const std::string MarshallLeft = "{";
const std::string MarshallRight = "}";

class SyntaxTreeFactory;
class SyntaxTreeTemplate;

/*	This data structure is immutable.
	Every time it mutates, you get a new one */
class SyntaxTree: public std::enable_shared_from_this<SyntaxTree> {
	public:
	vector< shared_ptr<SyntaxTree> > subtree; // read only
	shared_ptr<SyntaxTreeNode> root; // read only
	double weight;

	SyntaxTree(shared_ptr<SyntaxTreeNode> root, int depth);
	SyntaxTree(shared_ptr<SyntaxTree> src); /* copy constructor */
	~SyntaxTree();

	class Queue
	{
		public:
		vector< shared_ptr<SyntaxTree> > q;
	};

	class VariableMap
	{
		public:
		/* <var name, var value> */
		std::map<Rubify::string, shared_ptr<SyntaxTree> > map;
	};

	/* mutate a node of depth AT MOST `max_depth` into all possible RHS, and append the results to `queue` */
	bool multi_mutate(shared_ptr<SyntaxTree> &_this, shared_ptr<SyntaxTree> top, int max_depth, shared_ptr<SyntaxTree::Queue> queue);

	bool contain_prefix(shared_ptr<SyntaxTreeTemplate> temp);

	shared_ptr<SyntaxTree> search_and_replace(shared_ptr<SyntaxTreeTemplate> temp_src, shared_ptr<SyntaxTreeTemplate> temp_dst);

	/* check if all leaf nodes are terminal */
	bool is_complete();
	virtual double get_complexity();
	Rubify::string to_string(); //for human
	Rubify::string marshall(); //for parser
	size_t hash();
	bool equal(shared_ptr<SyntaxTree> t);

	static std::unique_ptr<SyntaxTreeFactory> factory;

	protected:

	double complexity = 0;
	int depth; // depth of the root
	size_t hash_value = 0;
	SyntaxTreeCompleteness complete;

	private:

	bool real_search_and_replace(shared_ptr<SyntaxTreeTemplate> temp_src, shared_ptr<SyntaxTreeTemplate> temp_dst);
	void mutate(int option);
	virtual void copy_initializer(shared_ptr<SyntaxTree> src);
	bool collect_variable(shared_ptr<VariableMap> vars, shared_ptr<SyntaxTreeTemplate> temp);
};

bool compare_syntax_tree_complexity(shared_ptr<SyntaxTree> a, shared_ptr<SyntaxTree> b);

class HashSyntaxTree {
	public:
	size_t operator ()(const shared_ptr<SyntaxTree> a) const;
};

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
	SyntaxTreeTemplate(shared_ptr<SyntaxTreeNode> root);
	Rubify::string var_name;
	Rubify::string to_string();
	bool is_variable(); /* non-terminal and no option in a template means a variable */
	shared_ptr<SyntaxTree> to_syntax_tree(shared_ptr<VariableMap> vars, int _depth);
};

class SyntaxLeftHandSide: public std::enable_shared_from_this<SyntaxLeftHandSide> {
	public:
	int id;
	Rubify::string name;
	vector<std::shared_ptr<SyntaxRightHandSide> > option;
	vector<std::shared_ptr<SyntaxTree> > shortcut;
	bool is_term;

	static const int NoOption = -1;
};

class SyntaxRightHandSide: public std::enable_shared_from_this<SyntaxRightHandSide> {
	public:
	int id;
	Rubify::string name;
	bool independent; /* only support one dependent subexp, which must be the only subexp */
	vector<std::shared_ptr<SyntaxLeftHandSide> > subexp;

	virtual Rubify::string to_string(vector<Rubify::string> subs) {return "";};
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
