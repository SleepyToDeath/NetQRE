#ifndef INCOMPLETE_EXECUTION_H
#define INCOMPLETE_EXECUTION_H

#include "syntax_tree.h"

class IEExample {
};

class IEProgram {
	public:
	virtual bool accept(IEExample* e) = 0;
};

class IESyntaxTree : public SyntaxTree {
	public:
	IESyntaxTree(SyntaxTreeNode* root);
	IESyntaxTree(SyntaxTree* src); /* copy constructor */

	IEProgram* to_program();

	private:
	IEProgram* p;
};

class IESyntaxLeftHandSide : public SyntaxLeftHandSide {
	public:
	virtual IEProgram* to_program() = 0;
};

class IESyntaxRightHandSide : public SyntaxRightHandSide {
	public:
	virtual IEProgram* combine_subprograms(std::vector<IEProgram*> subprograms) = 0;
};

#endif
