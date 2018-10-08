#include "syntax_tree.h"
#include <cmath>

#include <iostream>
using std::cout;
using std::endl;

bool compare_syntax_tree(shared_ptr<SyntaxTree> a, shared_ptr<SyntaxTree> b) {
	return a->get_complexity() > b->get_complexity();
}

SyntaxTree::SyntaxTree(shared_ptr<SyntaxTreeNode> r, int depth) {
	root = r;
	complete = UNKNOWN;
	complexity = 0;
	this->depth = depth;
}

SyntaxTree::SyntaxTree(shared_ptr<SyntaxTree> t) {
	/* shared part */
	root = shared_ptr<SyntaxTreeNode>(new SyntaxTreeNode(t->root));
	this->weight = t->weight;
	this->depth = t->depth;
	complete = UNKNOWN;
	complexity = 0;
	/* to be overridden */
	copy_initializer(t);
}

void SyntaxTree::copy_initializer(shared_ptr<SyntaxTree> t) {
	for (int i=0; i<t->subtree.size(); i++)
		subtree.push_back(SyntaxTree::factory->get_new(t->subtree[i]));
}

SyntaxTree::~SyntaxTree() {
}

void SyntaxTree::mutate(int option) {
//	cout<<"doing mutation "<<option<<endl;
	/* clean up previous mutation */
	if (root->get_option() != SyntaxLeftHandSide::NoOption)
	{
		while(subtree.size()>0)
		{
			subtree.pop_back();
		}
	}

	root->set_option(option);

	/* new mutation */
	if (option != SyntaxLeftHandSide::NoOption)
	{
		shared_ptr<SyntaxRightHandSide> r = root->get_type()->option[option];
		for (int i=0; i<r->subexp.size(); i++)
			subtree.push_back(
				SyntaxTree::factory->get_new( 
					shared_ptr<SyntaxTreeNode>(new SyntaxTreeNode(r->subexp[i])),
					depth+1
				));
//		cout<<"subtree size: "<<subtree.size()<<endl;
	}

}

bool SyntaxTree::is_complete() {
	
	if (complete == UNKNOWN)
	{
		if (root->get_type()->is_term)
			complete = COMPLETE;
		else if (root->get_option() == SyntaxLeftHandSide::NoOption)
			complete = INCOMPLETE;
		else
		{
			complete = COMPLETE;
			for (int i=0; i<subtree.size(); i++)
				if (!subtree[i]->is_complete())
				{
					complete = INCOMPLETE;
					break;
				}
		}
	}
	return complete == COMPLETE;
}

double SyntaxTree::get_complexity() {
	if (complexity == 0)
	{
		if (root->get_type()->is_term)
		{
			if (depth>1)
				complexity = -100.0;
		}
		else if (root->get_option() == SyntaxLeftHandSide::NoOption)
		{
//			complexity = root->get_type()->option.size();
			if (depth>1)
				complexity = 100.0;
		}
		else
		{
			complexity = 0;
			for (int i=0; i<subtree.size(); i++)
				complexity += subtree[i]->get_complexity();
			if (subtree.size() > 2)
				complexity -= 200;
//				complexity -= (subtree.size()) * 200;
			else
				complexity += (subtree.size()-1) * 100.0;
		}
	}
	if (complexity == 0)
		complexity = 0.01;
	return complexity;
}

bool SyntaxTree::multi_mutate(shared_ptr<SyntaxTree> top, int max_depth, shared_ptr<SyntaxTree::Queue> queue) {

	if (root->get_type()->is_term)
		return false;

	if (max_depth >= 0)
	{
		if (root->get_option() == SyntaxLeftHandSide::NoOption)
		{
			int branch_num = root->get_type()->option.size();
//			if(root->get_type()->name == "char")
//				cout<<"branch_num: "<<branch_num<<endl;
			for (int i=0; i<branch_num; i++)
			{
				mutate(i);
				queue->q.push_back(SyntaxTree::factory->get_new(top));
				queue->q.back()->weight = top->weight/branch_num;
//				cout<<"[New] "<<queue->q.back()->to_string()<<endl;
			}
			mutate(SyntaxLeftHandSide::NoOption);
			return true;
		}
		else
		{
			for (int i=0; i<subtree.size(); i++)
				if (subtree[i]->multi_mutate(top, max_depth-1, queue))
					return true;
			return false;
		}
	}
	else
	{
		return false;
	}
}

std::string SyntaxTree::to_string() {
	std::string s;
	if (root->get_type()->is_term) 
		s = root->get_type()->name;
	else if (root->get_option() == SyntaxLeftHandSide::NoOption)
		s = root->get_type()->name;
	else
	{
//		if ((subtree.size() > 1) || (!root->get_type()->option[root->get_option()]->independent))
		if ((!root->get_type()->option[root->get_option()]->independent))
			s = root->get_type()->option[root->get_option()]->name + "( ";
		for (int i=0; i<this->subtree.size(); i++)
			s = s+subtree[i]->to_string() + (i==subtree.size()-1?"":" ");
//		if ((subtree.size() > 1) || (!root->get_type()->option[root->get_option()]->independent))
		if ((!root->get_type()->option[root->get_option()]->independent))
			s = s + " )";
	}
	return s;
}

bool SyntaxTree::equal(shared_ptr<SyntaxTree> t) {
	if (!root->equal(t->root))
		return false;
	if (subtree.size() != t->subtree.size())
		return false;
	for (int i=0; i<subtree.size(); i++)
		if (!subtree[i]->equal(t->subtree[i]))
			return false;
	return true;
}

SyntaxTreeNode::SyntaxTreeNode(shared_ptr<SyntaxLeftHandSide> l) {
	type = l;
	option = SyntaxLeftHandSide::NoOption;
}

SyntaxTreeNode::SyntaxTreeNode(shared_ptr<SyntaxTreeNode> src) {
	option = src->option;
	type = src->type;
}

bool SyntaxTreeNode::equal(shared_ptr<SyntaxTreeNode> n) {
	return (option == n->option) && (type == n->type);
}

shared_ptr<SyntaxLeftHandSide> SyntaxTreeNode::get_type() {
	return type;
}

void SyntaxTreeNode::set_option(int op) {
	option = op;
}

int SyntaxTreeNode::get_option() {
	return option;
}

int SyntaxLeftHandSide::size() {
	return option.size();
}

int SyntaxRightHandSide::size() {
	return subexp.size();
}

