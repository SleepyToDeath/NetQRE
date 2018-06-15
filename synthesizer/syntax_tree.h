#ifndef SYNTAX_TREE_H
#define SYNTAX_TREE_H

#include<vector>

class SyntaxTreeNode;
class SyntaxRule;

class SyntaxTree {
	public:
	SyntaxTree();
	SyntaxTree(SyntaxTree* t); /* copy constructor */

	void mutate(int option);
	
	SyntaxTreeNode* root;
	std::vector<SyntaxTree*> subtree;
};

class SyntaxTreeNode {

	public:
	SyntaxTreeNode(SyntaxRule* type);
	SyntaxRule* get_type();
	void set_option();
	int get_option();
	
	private:
	SyntaxRule* type;
	int option;
};

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
