#ifndef SYNTAX_TREE_H
#define SYNTAX_TREE_H
//#define DEBUG_PRINT
//#define DEBUG_PRINT_2
//#define DEBUG_PRINT_4
//#define DEBUG_PRINT_5
#define DEBUG_PRINT_9
#define DEBUG_PRINT_8

#include<vector>
#include<string>
#include<iostream>

class SyntaxRightHandSide;
class SyntaxLeftHandSide;
class SyntaxTreeNode;

enum SyntaxTreeCompleteness {
	UNKNOWN, COMPLETE, INCOMPLETE
};

/*	This data structure is immutable.
	Every time it mutates, you get a new one */
class SyntaxTree {
	public:
	std::vector<SyntaxTree*> subtree; // read only
	SyntaxTreeNode* root; // read only
	double weight;

	SyntaxTree(SyntaxTreeNode* root);
	SyntaxTree(SyntaxTree* src); /* copy constructor */
	~SyntaxTree();

	/* mutate a node of depth AT MOST `max_depth` into all possible RHS, and append the results to `queue` */
	bool multi_mutate(SyntaxTree* root, int max_depth, std::vector<SyntaxTree*> * queue);
	/* check if all leaf nodes are terminal */
	bool is_complete();

	double get_complexity();

	std::string to_string();

	bool equal(SyntaxTree* t);

	private:
	SyntaxTreeCompleteness complete;
	double complexity;

	void mutate(int option);
};

bool compare_syntax_tree(SyntaxTree* a, SyntaxTree* b);

class SyntaxTreeNode {

	public:
	SyntaxTreeNode(SyntaxLeftHandSide* type);
	SyntaxTreeNode(SyntaxTreeNode* src);
	SyntaxLeftHandSide* get_type();
	void set_option(int option);
	int get_option();

	bool equal(SyntaxTreeNode* n);
	
	private:
	SyntaxLeftHandSide* type;
	int option;
};

/* [TODO] useless? */
class LanguageSyntax
{
	public:
	int add_rule(SyntaxLeftHandSide* r);
	SyntaxLeftHandSide* get_rule(int id);
	int size();

	private:
	std::vector<SyntaxLeftHandSide*> rule;
};

class SyntaxLeftHandSide {
	public:
//	int id;
	int size();
	std::string name;
	std::vector<SyntaxRightHandSide*> option;
	bool is_term;

	static const int NoOption = -1;
};

class SyntaxRightHandSide {
	public:
//	int id;
	int size();
	std::string name;
	bool independent; /* only support one dependent subexp, which must be the only subexp */
	std::vector<SyntaxLeftHandSide*> subexp;

	virtual std::string to_string(std::vector<std::string> subs) {return "";};
};


#endif
