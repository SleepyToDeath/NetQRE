#include "incomplete_execution.h"
IESyntaxTree::~IESyntaxTree() {
	delete p;
}

std::string IESyntaxTree::to_string() {
	std::string s;
	if (root->get_type()->is_term) 
		s = root->get_type()->name;
	else if (root->get_option() == SyntaxLeftHandSide::NoOption)
		s = root->get_type()->name;
	else
	{
		SyntaxRightHandSide* rhs = root->get_type()->option[root->get_option()];
		std::vector<std::string> subs;

		for (int i=0; i<this->subtree.size(); i++)
			subs.push_back(((IESyntaxTree*)(subtree[i]))->to_string());

		s = rhs->to_string(subs);
	}
	return s;
}



IESyntaxTree::IESyntaxTree(SyntaxTreeNode* root)
:SyntaxTree(root)
{
	p = nullptr;
}

IESyntaxTree::IESyntaxTree(SyntaxTree* src)
:SyntaxTree(src->root)
{
	root = new SyntaxTreeNode(src->root);
	weight = src->weight;
	p = nullptr;
	for (int i=0; i<src->subtree.size(); i++)
		subtree.push_back(new IESyntaxTree(src->subtree[i]));
}

IEProgram* IESyntaxTree::to_program() {
	if (p != nullptr)
		return p;
	if ((root->get_type()->is_term) || (root->get_option() == SyntaxLeftHandSide::NoOption))
	{
//std::cout<<"IEProgram* IESyntaxTree::to_program branch 1\n";
		p =((IESyntaxLeftHandSide*)(root->get_type()))->to_program();
	}
	else
	{
//std::cout<<"IEProgram* IESyntaxTree::to_program branch 2\n";
		std::vector<IEProgram*> subprograms;
		for (int i=0; i<subtree.size(); i++)
			subprograms.push_back(((IESyntaxTree*)(subtree[i]))->to_program());
		p = ((IESyntaxRightHandSide*)(root->get_type()->option[root->get_option()]))->combine_subprograms(subprograms);
	}
	return p;
}

