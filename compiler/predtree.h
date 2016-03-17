#ifndef _PREDTREE_H
#define _PREDTREE_H

#include <map>

class Expr;
class EqualExpr;
class BiopExpr;

using namespace std;

class Tree;
class TreeNode {
    public:
	string name;

	Tree* tree;
	map<string, TreeNode*> childrenMap;
	TreeNode* defaultNode;

	list<pair<void*, void*>> transitions;
	
	//static const string INF;
	static const int LESS_THAN_EQUAL_TO = 0;
	static const int LESS_THAN = 1;
	static const int GREATER_THAN_EQUAL_TO = 2;
	static const int GREATER_THAN = 3;

	int leftOP;
	string leftBound;

	int rightOP;
	string rightBound;

    public:
	TreeNode() : name("Leaf") {}
	TreeNode(string name) : name(name) {}

	void printStructure() {
	    cout << name << "@" << this << endl;
	    for (auto it=childrenMap.begin(); it!=childrenMap.end(); it++) {
		cout << "\t" << it->first << ":\t" << it->second << endl;
	    }
	    if (defaultNode != NULL)
		cout << "\t" << "default" << ":\t" << defaultNode << endl;
	    cout << endl;
	    for (auto it=childrenMap.begin(); it!=childrenMap.end(); it++) {
		it->second->printStructure();
	    }
	    if (defaultNode != NULL)
		defaultNode->printStructure();
	}

	void printTransitions() {
	    cout << this << endl;
	    for (auto tran : transitions) {
		cout << "\t" << tran.first << "->" << tran.second << endl;
	    }
	    for (auto it=childrenMap.begin(); it!=childrenMap.end(); it++) {
		it->second->printTransitions();
	    }
	    if (defaultNode != NULL)
		defaultNode->printTransitions();
	}
};

//const string TreeNode::INF = string("INF");

class Tree{
    public:
	TreeNode *root;
	map<string, int> name_to_id;
	vector<string> id_to_name;
	
    
    public:
	TreeNode* createTreeNode(string name) {
	    TreeNode *node = new TreeNode(name);
	    node->tree = this;
	    return node;
	}

	TreeNode* createTreeNode() {
	    TreeNode *node = new TreeNode();
	    node->tree = this;
	    return node;
	}

	Tree(const list<string>& orderedVariableList) {
	    root = NULL;
	    TreeNode *parent = NULL;
	    int n = 0;
	    for (auto var : orderedVariableList) {
		cout << n << " : " << var << endl;
		id_to_name.push_back(var);
		name_to_id[var] = n++;
		TreeNode *node = createTreeNode(var);
		if (root == NULL) {
		    root = node;
		} else {
		    parent->defaultNode = node;
		}
		parent = node;
	    }


	    TreeNode *node = createTreeNode();
	    id_to_name.push_back("Leaf");
	    name_to_id["Leaf"] = n++;

	    if (root == NULL) {
		root = node;
	    } else {
		parent->defaultNode = node;
	    }
	}

	TreeNode* createChild(TreeNode* node) {
	    TreeNode* child = createTreeNode(id_to_name[name_to_id[node->name]+1]);
	    return child;
	}

	bool isRoot(TreeNode* node) {
	    return node==root;
	}
//	set<TreeNode*> addPredicate(list<Expr*> pred) {
//	    addPredicateAt(root, pred);
//	}

	void print() {
	    cout << "---------- Tree ---------" << endl;
	    root->printStructure();
	    root->printTransitions();
	    cout << "---------- Tree ---------" << endl;
	}
};

#endif
