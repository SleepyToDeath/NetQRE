#include "incomplete_execution.h"

IESyntaxTree::IESyntaxTree(SyntaxTreeNode* root)
:SyntaxTree(root)
{
	p = nullptr;
}

IESyntaxTree::IESyntaxTree(IESyntaxTree* src)
:SyntaxTree(src->root)
{
	p = nullptr;
	for (int i=0; i<src->subtree.size(); i++)
		subtree.push_back(new IESyntaxTree((IESyntaxTree*)(src->subtree[i])));
}

IEProgram* IESyntaxTree::to_program() {
	if (p != nullptr)
		return p;
	if (root->get_type()->is_term || root->get_option() == SyntaxLeftHandSide::NoOption)
	{
		p =((IESyntaxLeftHandSide*)(root->get_type()))->to_program();
	}
	else
	{
		std::vector<IEProgram*> subprograms;
		for (int i=0; i<subtree.size(); i++)
			subprograms.push_back((IESyntaxTree*)subtree[i]->to_program());
		p = ((IESyntaxRightHandSide*)(root->get_type()->option[root->get_option()]))->combine_subprograms(subprograms);
	}
	return p;
}

