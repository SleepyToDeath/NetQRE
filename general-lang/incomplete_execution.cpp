#include "incomplete_execution.h"

/*
std::string IESyntaxTree::to_string() {
	std::string s;
	if (root->get_type()->is_term) 
		s = root->get_type()->name;
	else if (root->get_option() == SyntaxLeftHandSide::NoOption)
		s = root->get_type()->name;
	else
	{
		SyntaxRightHandSide* rhs = root->get_type()->option[root->get_option()];
		vector<std::string> subs;

		for (int i=0; i<this->subtree.size(); i++)
			subs.push_back(((IESyntaxTree*)(subtree[i]))->to_string());

		s = rhs->to_string(subs);
	}
	return s;
}
*/

IESyntaxTree::IESyntaxTree(shared_ptr<SyntaxTreeNode> root, int depth)
:SyntaxTree(root, depth)
{
	p = nullptr;
}

IESyntaxTree::IESyntaxTree(shared_ptr<SyntaxTree> src)
:SyntaxTree(src)
{
	p = nullptr;
}

void IESyntaxTree::copy_initializer(shared_ptr<SyntaxTree> src) {
	for (int i=0; i<src->subtree.size(); i++)
		subtree.push_back(shared_ptr<IESyntaxTree>(new IESyntaxTree(src->subtree[i])));
}

shared_ptr<IEProgram> IESyntaxTree::to_program() {
	return nullptr;
}

Rubify::string IESyntaxTree::to_string() {
	return "";
}

Rubify::string IESyntaxTree::marshall() {
	return "";
}

/*
IEProgram* IESyntaxTree::to_program() {
	if (p != nullptr)
		return p;
	if ((root->get_type()->is_term) || (root->get_option() == SyntaxLeftHandSide::NoOption))
	{
//std::cerr<<"IEProgram* IESyntaxTree::to_program branch 1\n";
		p =((IESyntaxLeftHandSide*)(root->get_type()))->to_program();
	}
	else
	{
//std::cerr<<"IEProgram* IESyntaxTree::to_program branch 2\n";
		vector<IEProgram*> subprograms;
		for (int i=0; i<subtree.size(); i++)
			subprograms.push_back(((IESyntaxTree*)(subtree[i]))->to_program());
		p = ((IESyntaxRightHandSide*)(root->get_type()->option[root->get_option()]))->combine_subprograms(subprograms);
	}
	return p;
}
*/

