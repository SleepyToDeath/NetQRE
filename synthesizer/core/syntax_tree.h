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

using std::shared_ptr;
using std::vector;

class SyntaxRightHandSide;
class SyntaxLeftHandSide;
class SyntaxTreeNode;

enum SyntaxTreeCompleteness {
	UNKNOWN, COMPLETE, INCOMPLETE
};

/*	This data structure is immutable.
	Every time it mutates, you get a new one */
class SyntaxTree: public std::enable_shared_from_this<SyntaxTree> {
	public:
	std::vector< shared_ptr<SyntaxTree> > subtree; // read only
	shared_ptr<SyntaxTreeNode> root; // read only
	double weight;

	SyntaxTree(shared_ptr<SyntaxTreeNode> root);
	SyntaxTree(shared_ptr<SyntaxTree> src); /* copy constructor */
	~SyntaxTree();

	class Queue
	{
		public:
		std::vector< shared_ptr<SyntaxTree> > q;
	};

	/* mutate a node of depth AT MOST `max_depth` into all possible RHS, and append the results to `queue` */
	bool multi_mutate(shared_ptr<SyntaxTree> root, int max_depth, shared_ptr<Queue> queue);
	/* check if all leaf nodes are terminal */
	bool is_complete();

	double get_complexity();

	std::string to_string();

	bool equal(shared_ptr<SyntaxTree> t);

	private:
	SyntaxTreeCompleteness complete;
	double complexity;

	void mutate(int option);

	virtual void copy_initializer(shared_ptr<SyntaxTree> src);
};

bool compare_syntax_tree(shared_ptr<SyntaxTree> a, shared_ptr<SyntaxTree> b);

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

class SyntaxLeftHandSide: public std::enable_shared_from_this<SyntaxLeftHandSide> {
	public:
//	int id;
	int size();
	std::string name;
	std::vector<std::shared_ptr<SyntaxRightHandSide> > option;
	bool is_term;

	static const int NoOption = -1;
};

class SyntaxRightHandSide: public std::enable_shared_from_this<SyntaxRightHandSide> {
	public:
//	int id;
	int size();
	std::string name;
	bool independent; /* only support one dependent subexp, which must be the only subexp */
	std::vector<std::shared_ptr<SyntaxLeftHandSide> > subexp;

	virtual std::string to_string(std::vector<std::string> subs) {return "";};
};


#endif