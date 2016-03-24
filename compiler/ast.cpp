#include <iostream>
#include <sstream>
#include <stdio.h>
#include <map>
#include "ast.h"
#include "util.h"
#include "expr.cpp"
#include "block.cpp"
#include "fun.cpp"
#include "regex.cpp"

using namespace std;

set<TreeNode*> addPredicateAt(TreeNode* node, list<Expr*> pred) {
    set<TreeNode*> ret;
    if (pred.empty()) {
	ret.insert(node);
	return ret;
    }

    Expr* front = *pred.begin();
    cout << "all expr" << endl;
    for (auto expr : pred) {
	expr->emit(cout);
	cout << endl;
    }

    BiopExpr* expr = (BiopExpr*)(front);
    Expr* right = expr->right;

    ostringstream convert;
    right->emit(convert);
    string name = convert.str();

    ostringstream convert2;
    expr->left->emit(convert2);
    string field = convert2.str();

    if (node->tree->name_to_id[node->name] < node->tree->name_to_id[name]) {
	bool hasChild = false;
	for (auto it = node->childrenMap.begin(); 
		it != node->childrenMap.end();
		it++) {
	    TreeNode *next = it->second;
	    hasChild = true;
	    set<TreeNode*> ret1 = addPredicateAt(next, pred);
	    ret.insert(ret1.begin(), ret1.end());
	}
	if (node->defaultNode!=NULL) {
	    set<TreeNode*> ret1 = addPredicateAt(node->defaultNode, pred);
	    ret.insert(ret1.begin(), ret1.end());
	} else if (!hasChild) {
	    node->defaultNode = node->tree->createChild(node);
	    return addPredicateAt(node->defaultNode, pred);
	}
	return ret;
    } 

    pred.pop_front();
    if (dynamic_cast<EqualExpr*>(expr) != NULL) {
	cout << "EQUAL" << endl;
	expr->emit(cout);
	cout << endl;
	auto it = node->childrenMap.find(field);
	cout << "DF " << node->name <<endl;
	if (it!=node->childrenMap.end()) {
	    TreeNode *next = it->second;
	    return addPredicateAt(next, pred);
	} else {
	    cout << "F" << endl;
	    TreeNode *child = node->tree->createChild(node);
	    cout << "SF" << endl;
	    node->childrenMap[field] = child;
	    cout << "SDFDF" << endl;
	    return addPredicateAt(child, pred);
	}
    }
    cout << "OK" << endl;

    if (dynamic_cast<NotEqualExpr*>(expr) != NULL) {
	cout << "NotEQUAL" << endl;
	auto findit = node->childrenMap.find(name);
	if (findit==node->childrenMap.end()) {
	    TreeNode *child = node->tree->createChild(node);
	    node->childrenMap[field] = child;
	}
	if (node->defaultNode==NULL) {
	    node->defaultNode = node->tree->createChild(node);
	}
	for (auto it=node->childrenMap.begin();
		it!=node->childrenMap.end();
		it++) {
	    if (it->first.compare(field)==0) {
		continue;
	    } else {
		set<TreeNode*> ret1 = addPredicateAt(it->second, pred);
		ret.insert(ret1.begin(), ret1.end());
	    }
	}
	auto ret1 = addPredicateAt(node->defaultNode, pred);
	ret.insert(ret1.begin(), ret1.end());
	return ret;
    }
    cout << "OK2" << endl;

    if (dynamic_cast<GreaterExpr*>(expr) != NULL) {
	cout << "GreaterExpr" << endl;
	auto findit = node->childrenMap.find(name);
	if (findit==node->childrenMap.end()) {
	    TreeNode *child = node->tree->createChild(node);
	    node->childrenMap[field] = child;
	}
	if (node->defaultNode==NULL) {
	    node->defaultNode = node->tree->createChild(node);
	}
	for (auto it=node->childrenMap.begin();
		it!=node->childrenMap.end();
		it++) {
	    if (it->first.compare(field)==0) {
		continue;
	    } else {
		set<TreeNode*> ret1 = addPredicateAt(it->second, pred);
		ret.insert(ret1.begin(), ret1.end());
	    }
	}
	auto ret1 = addPredicateAt(node->defaultNode, pred);
	ret.insert(ret1.begin(), ret1.end());
	return ret;
    }
}

void addAtomicPredicate(TreeNode *node, string field, TreeNode *child, BiopExpr *expr, list<Expr*> pred) {
    pred.pop_front();

    if (dynamic_cast<EqualExpr*>(expr) != NULL) {
	addPredicate(child, pred);
    } else if (dynamic_cast<NotEqualExpr*>(expr) != NULL) {
	for (auto it = node->childrenMap.begin(); 
		it != node->childrenMap.end();
		it++) {
	    TreeNode *next = it->second;
	    if (next != child) {
		addPredicate(next, pred);
	    }
	}

	addPredicate(node->defaultNode, pred);
    }
}

void addPredicate(TreeNode* node, list<Expr*> pred) {
    if (pred.empty()) {
	return;
    }

    Expr* front = *pred.begin();

    BiopExpr* expr = (BiopExpr*)(front);
    Expr* right = expr->right;

    ostringstream convert;
    right->emit(convert);
    string name = convert.str();

    ostringstream convert2;
    expr->left->emit(convert2);
    string field = convert2.str();

    if (node->tree->name_to_id[name] >  node->tree->name_to_id[node->name]) {
	bool hasChild = false;
	for (auto it = node->childrenMap.begin(); 
		it != node->childrenMap.end();
		it++) {
	    TreeNode *next = it->second;
	    hasChild = true;
	    addPredicate(next, pred);
	}
	if (node->defaultNode!=NULL) {
	    addPredicate(node->defaultNode, pred);
	} else if (!hasChild) {
	    node->defaultNode = node->tree->createChild(node);
	    addPredicate(node->defaultNode, pred);
	}
    } else if (node->tree->name_to_id[name] < node->tree->name_to_id[node->name]) {
	TreeNode* parent = node->parent;
	TreeNode* newNode = node->tree->createTreeNode(name);
	newNode->defaultNode = node;
	node->parent = newNode;
	newNode->parent = parent;
	
	for (auto it = parent->childrenMap.begin();
	    it != parent->childrenMap.end();
	    it++) {
	    if (it->second == node) {
		it->second = newNode;
		addPredicate(newNode, pred);
		return;
	    }
	}
	parent->defaultNode = newNode;
	addPredicate(newNode, pred);
    } else {
	if (node->childrenMap.empty()) {
	    TreeNode* child;
	    if (node->defaultNode!=NULL) {
		child = node->tree->copySubTree(node->defaultNode);
	    } else {
		node->defaultNode = node->tree->createTreeNode(name);
		child = node->tree->createTreeNode(name);
	    }
	    node->childrenMap[field] = child;
	    child->parent = node;

	    addAtomicPredicate(node, field, child, expr, pred);
	} else {
	    auto findIt = node->childrenMap.find(field);
	    if (findIt != node->childrenMap.end()) {
		addAtomicPredicate(node, field, findIt->second, expr, pred);
	    } else {
		for (auto it = node->childrenMap.begin();
			it != node->childrenMap.end();
			it++) {
		    if (it->second->name != name) {
			TreeNode *child = node->tree->createTreeNode(name);
			child->defaultNode = it->second;
			it->second->parent = child;
			it->second = child;
			child->parent = node;
		    }
		    addPredicate(it->second, pred);
		}

		if (node->defaultNode->name != name) {
		    TreeNode *child = node->tree->createTreeNode(name);
		    child->defaultNode = node->defaultNode;
		    node->defaultNode->parent = child;
		    node->defaultNode = child;
		    child->parent = node;
		}
		addPredicate(node->defaultNode, pred);
	    }
	}
    }
}

set<TreeNode*> getPredTreeNodes(TreeNode *node, list<Expr*> pred) {
    set<TreeNode*> ret;

    if (node->isLeaf()) {
	ret.insert(node);
	return ret;
    }

    if (pred.empty()) {
	for (auto it = node->childrenMap.begin();
	    it != node->childrenMap.end();
	    it++) {
	    set<TreeNode*> temp = getPredTreeNodes(it->second, pred);
	    ret.insert(temp.begin(), temp.end());
	}

	set<TreeNode*> temp = getPredTreeNodes(node->defaultNode, pred);
	ret.insert(temp.begin(), temp.end());

	return ret;
    }

    Expr* front = *pred.begin();

    BiopExpr* expr = (BiopExpr*)(front);
    Expr* right = expr->right;

    ostringstream convert;
    right->emit(convert);
    string name = convert.str();

    ostringstream convert2;
    expr->left->emit(convert2);
    string field = convert2.str();

    if (node->name != name) {
	for (auto it = node->childrenMap.begin();
	    it != node->childrenMap.end();
	    it++) {
	    set<TreeNode*> temp = getPredTreeNodes(it->second, pred);
	    ret.insert(temp.begin(), temp.end());
	}

	set<TreeNode*> temp = getPredTreeNodes(node->defaultNode, pred);
	ret.insert(temp.begin(), temp.end());

	return ret;
    }
    
    auto findIt = node->childrenMap.find(field);
    if (findIt != node->childrenMap.end()) {
	pred.pop_front();
	if (dynamic_cast<EqualExpr*>(expr) != NULL) {
	    return getPredTreeNodes(findIt->second, pred);
	} else if (dynamic_cast<NotEqualExpr*>(expr) != NULL) {
	    for (auto it = node->childrenMap.begin(); 
		    it != node->childrenMap.end();
		    it++) {
		TreeNode *next = it->second;
		if (it != findIt) {
		    set<TreeNode*> temp = getPredTreeNodes(next, pred);
		    ret.insert(temp.begin(), temp.end());
		}
	    }
	    return ret;
	}
    } else {
	for (auto it = node->childrenMap.begin();
	    it != node->childrenMap.end();
	    it++) {
	    set<TreeNode*> temp = getPredTreeNodes(it->second, pred);
	    ret.insert(temp.begin(), temp.end());
	}

	set<TreeNode*> temp = getPredTreeNodes(node->defaultNode, pred);
	ret.insert(temp.begin(), temp.end());

        return ret;
    }

    return ret;
}


