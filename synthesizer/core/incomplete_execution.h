#ifndef INCOMPLETE_EXECUTION_H
#define INCOMPLETE_EXECUTION_H

#include "syntax_tree.h"
#include <memory>

using std::shared_ptr;

class IEExample {
};

class IEProgram {
	public:
	virtual bool accept( shared_ptr<IEExample> e) = 0;
};

class IESyntaxTree : public SyntaxTree {
	public:
	IESyntaxTree( shared_ptr<SyntaxTreeNode> root);
	IESyntaxTree( shared_ptr<SyntaxTree> src); /* copy constructor */

	virtual shared_ptr<IEProgram> to_program();

	virtual std::string to_string();

	void copy_initializer(shared_ptr<SyntaxTree> src);

	private:
	shared_ptr<IEProgram> p;
};

class IESyntaxLeftHandSide : public SyntaxLeftHandSide {
	public:
	virtual shared_ptr<IEProgram> to_program() = 0;
};

class IESyntaxRightHandSide : public SyntaxRightHandSide {
	public:
	virtual shared_ptr<IEProgram> combine_subprograms(std::vector< shared_ptr<IEProgram> > subprograms) = 0;
};

#endif
