#ifndef SYNTAX_TREE_H
#define SYNTAX_TREE_H

#include<vector>

class SyntaxTreeNode;
class SyntaxRule;

class SyntaxTree {
	public:
	SyntaxTreeNode* root;
	std::vector<SyntaxTree*> subtree;

	SyntaxTree(SyntaxTreeNode* root);
	SyntaxTree(SyntaxTree* src); /* copy constructor */
	~SyntaxTree();

	void mutate(int option);
	/* mutate a node of depth AT MOST `max_depth` into all possible RHS, and append the results to `queue` */
	bool multi_mutate(SyntaxTree* root, int max_depth, std::vector<SyntaxTree*> * queue);
	/* check if all leaf nodes are terminal */
	bool complete();
};

class SyntaxTreeNode {

	public:
	SyntaxTreeNode(SyntaxLeftHandSide* type);
	SyntaxTreeNode(SyntaxTreeNode* src);
	SyntaxLeftHandSide* get_type();
	void set_option(int option);
	int get_option();
	
	private:
	SyntaxRule* type;
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
	vector<SyntaxLeftHandSide*> rule;
};

class SyntaxLeftHandSide {
	public:
	int id;
	int size();
	vector<SyntaxRightHandSide*> option;
	bool is_term;

	const int NoOption = -1;
};

class SyntaxRightHandSide {
	public:
	int size();
	bool independent; /* only support one dependent subexp, which must be the only subexp */
	vector<SyntaxLeftHandSide*> subexp;
};


#endif
