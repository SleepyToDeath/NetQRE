#include "syntax_tree.h"
#include <cmath>
#include <experimental/random>

#include <iostream>
using std::cout;
using std::endl;
using std::string;

bool compare_syntax_tree_complexity(shared_ptr<SyntaxTree> a, shared_ptr<SyntaxTree> b) {
	return a->get_complexity() > b->get_complexity();
}

size_t HashSyntaxTree::operator()(const shared_ptr<SyntaxTree> a) const {
	return a->hash();
}

bool CmpSyntaxTree::operator ()(const shared_ptr<SyntaxTree> a, const shared_ptr<SyntaxTree> b) const {
	return a->equal(b);
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
	/* clean up previous mutation */
	if (root->get_option() != SyntaxLeftHandSide::NoOption)
	{
		subtree.clear();
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

	/* not used */
	return 0;

	if (complexity == 0)
	{
		if (root->get_type()->is_term)
		{
			complexity = -100.0;
		}
		else if (root->get_option() == SyntaxLeftHandSide::NoOption)
		{
//			complexity = root->get_type()->option.size();
			complexity = 300.0;
		}
		else
		{
			complexity = 0;
			for (int i=0; i<subtree.size(); i++)
				complexity += subtree[i]->get_complexity();
				/*
			if (subtree.size() > 2)
				complexity -= 200;
//				complexity -= (subtree.size()) * 200;
			else
				*/
			complexity += (subtree.size()-1) * 100.0;
//			complexity -= 50.0;
		}
	}
	if (complexity == 0)
		complexity = 0.01;
	return complexity;
}

bool SyntaxTree::multi_mutate(shared_ptr<SyntaxTree> &_this, shared_ptr<SyntaxTree> top, int max_depth, shared_ptr<SyntaxTree::Queue> queue) {

	if (root->get_type()->is_term)
		return false;

	if (max_depth >= 0)
	{
		if (root->get_option() == SyntaxLeftHandSide::NoOption)
		{
			int branch_num = root->get_type()->option.size();
			for (int i=0; i<branch_num; i++)
			{
				mutate(i);
				queue->q.push_back(SyntaxTree::factory->get_new(top));
//				queue->q.back()->weight = top->weight/branch_num;
			}
			mutate(SyntaxLeftHandSide::NoOption);
			vector<shared_ptr<SyntaxTree> > &shortcut = root->get_type()->shortcut;
			shared_ptr<SyntaxTree> this_bak = _this;
			for (int i=0; i<shortcut.size(); i++)
			{
				_this = shortcut[i];
				queue->q.push_back(SyntaxTree::factory->get_new(top));
//				queue->q.back()->weight = top->weight/branch_num;
			}
			_this = this_bak;
			return true;
		}
		else
		{
			vector<int> subtree_bak;
			for (int i=0; i<subtree.size(); i++)
				subtree_bak.push_back(i);
			for (int i=0; i<subtree.size(); i++)
			{
//				int tmp = i;
				int l = std::experimental::randint(0,(int)subtree_bak.size()-1);
				int tmp = subtree_bak.back();
				subtree_bak[subtree_bak.size()-1] = subtree_bak[l];
				subtree_bak[l] = tmp;
				tmp = subtree_bak.back();

				if (subtree[tmp]->multi_mutate(subtree[tmp], top, max_depth-1, queue))
					return true;

				subtree_bak.pop_back();
			}
			return false;
		}
	}
	else
	{
		return false;
	}
}

bool SyntaxTree::contain_prefix(shared_ptr<SyntaxTreeTemplate> temp) {
	if (temp->is_variable())
		return root->get_type() == temp->root->get_type();
	if (!root->equal(temp->root))
		return false;
	for (int i=0; i<subtree.size(); i++)
		if (!subtree[i]->contain_prefix(std::static_pointer_cast<SyntaxTreeTemplate>(temp->subtree[i])))
			return false;
	return true;
}

shared_ptr<SyntaxTree> SyntaxTree::search_and_replace(
									shared_ptr<SyntaxTreeTemplate> temp_src, 
									shared_ptr<SyntaxTreeTemplate> temp_dst)
{
	/* 	SyntaxTree should show an immutable interface.
		Too error prone to backup and restore
		So search and replace on a copied new tree */
	auto candidate = factory->get_new(shared_from_this());
	if (candidate->real_search_and_replace(temp_src, temp_dst))
		return candidate;
	else
		return nullptr;
}

bool SyntaxTree::real_search_and_replace(
									shared_ptr<SyntaxTreeTemplate> temp_src, 
									shared_ptr<SyntaxTreeTemplate> temp_dst)
{
	/* enforce re-compute after tree is changed */
	complexity = 0;
	hash_value = 0;
	complete = UNKNOWN;
	bool matching_flag = false;

	{
		/* in either case, try subtrees */
		for (int i=0; i<subtree.size(); i++)
			if (subtree[i]->real_search_and_replace(temp_src, temp_dst))
//				return true;
				matching_flag = true;
	}


	if (contain_prefix(temp_src))
	{
		/* match, start to replace */
		auto vars = shared_ptr<VariableMap>(new VariableMap());
		if (collect_variable(vars, temp_src))
		{
			auto tree_dst = temp_dst->to_syntax_tree(vars, depth);
			root = tree_dst->root;
			subtree = tree_dst->subtree;
			complexity = 0;
			hash_value = 0;
			complete = UNKNOWN;
			matching_flag = true;
		}
	}
	
	return matching_flag;
}

bool SyntaxTree::collect_variable(shared_ptr<VariableMap> vars, shared_ptr<SyntaxTreeTemplate> temp) {
	if (temp->is_variable())
	{
		if (vars->map.count(temp->var_name) == 0)
		{
			vars->map[temp->var_name] = shared_from_this();
			return true;
		}
		else
			return equal(vars->map[temp->var_name]);
	}
	else {
		for (int i=0; i<subtree.size();i++)
			if (!subtree[i]->collect_variable(vars, std::static_pointer_cast<SyntaxTreeTemplate>(temp->subtree[i])))
				return false;
		return true;
	}
}

std::string SyntaxTreeTemplate::to_string() {
	std::string s;
//	cout<<root->get_type()->name<<"--> subtree size: "<<subtree.size()<<endl;
	if (root->get_type()->is_term) 
		s = root->get_type()->name;
	else if (root->get_option() == SyntaxLeftHandSide::NoOption)
		s = root->get_type()->name + "@" + var_name;
	else
	{
		auto rhs = (root->get_type()->option[root->get_option()]);
		s = root->get_type()->name;
		for (int i=0; i<rhs->subexp.size(); i++)	{
			s = s + (std::static_pointer_cast<SyntaxTreeTemplate>(subtree[i])->to_string());
		}
	}
	return s;
}

SyntaxTreeTemplate::SyntaxTreeTemplate(shared_ptr<SyntaxTreeNode> root):SyntaxTree(root, 0) {
}

bool SyntaxTreeTemplate::is_variable() {
	return (!root->get_type()->is_term) && (root->get_option() == SyntaxLeftHandSide::NoOption);
}

shared_ptr<SyntaxTree> SyntaxTreeTemplate::to_syntax_tree(shared_ptr<VariableMap> vars, int _depth) {
	if (is_variable())
		return factory->get_new(vars->map[var_name]);
	auto new_root = shared_ptr<SyntaxTreeNode>(new SyntaxTreeNode(root));
	auto new_tree = factory->get_new(new_root, _depth);
	for (int i=0; i<subtree.size(); i++)
		new_tree->subtree.push_back(std::static_pointer_cast<SyntaxTreeTemplate>(subtree[i])->to_syntax_tree(vars, _depth+1));
	return new_tree;
}

std::string SyntaxTree::to_string() {
	throw string("SyntaxTree::to_string This function shouldn't be called\n");
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

size_t SyntaxTree::hash() {
	if (hash_value != 0)
		return hash_value;
	size_t v = root->get_type()->id;
	for (int i=0; i<subtree.size(); i++)
		v = v*31 + subtree[i]->hash();
	hash_value = v;
	if (hash_value == 0)
		hash_value = 1;
	return hash_value;
}

bool SyntaxTree::equal(shared_ptr<SyntaxTree> t) {
	if (hash() != t->hash())
		return false;
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


