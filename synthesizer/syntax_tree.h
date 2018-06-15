#ifndef SYNTAX_TREE_H
#define SYNTAX_TREE_H

#include<vector>

class SyntaxNode;
class SyntaxRule;

class SyntaxTree {
	public:
	std::vector<

};

class SyntaxNode {

	public:
	SyntaxRule type;

	private:

};

class SyntaxRule {
	public:
	int num_sub_exp;
	bool terminal;
};

#endif
