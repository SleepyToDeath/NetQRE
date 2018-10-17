#ifndef INCOMPLETE_EXECUTION_H
#define INCOMPLETE_EXECUTION_H

#include "syntax_tree.h"
#include <memory>

using std::shared_ptr;
using std::unique_ptr;

class IEExample {
};

class IEConfig {
	public:
	bool pos_check;
	bool pos_all;
	bool pos_accept;
	bool neg_check;
	bool neg_all;
	bool neg_accept;
};

const IEConfig DEFAULT_IE_CONFIG = {true, true, true, true, true, .neg_accept = false};

class IEProgram {
	public:
	/* can choose between all/exist example accepted/rejected by the program */
	virtual bool accept( shared_ptr<IEExample> e, IEConfig cfg = DEFAULT_IE_CONFIG) = 0;
};

class IESyntaxTreeFactory;

class IESyntaxTree : public SyntaxTree {
	public:
	IESyntaxTree( shared_ptr<SyntaxTreeNode> root, int depth);
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
