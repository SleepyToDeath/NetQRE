#include <iostream>
#include <sstream>
#include <fstream>
#include <stdio.h>
#include <map>
#include <list>
#include "ast.h"
#include "predtree.h"
#include "util.h"

using namespace std;

extern FILE *yyin;
extern int yyparse();

// settings
// substitude every functions for its call if compose=true
bool compose = false;
// evaluate agg functions online
bool online = true;

map<string, FunBase*> funTable;
list<Decl*>* func_list;
list<RE*>* re_list;
map<RE*, int> re_id_map;
list<list<Expr*>*> *pred_list;
map<Expr*, int> var_map;

Tree *tree;

list<string> variables;

void addScopeToVariables() {
    for (auto func : *func_list) {
	//passing emty scope for root
	func->addScopeToVariables("");
    }
}


void getFreeVariables() {
    for (auto func : *func_list) {
	func->getFreeVariables();
    }

    cout << "Free variables in re: " << endl;
    for (auto re : *re_list) {
	append(variables, re->freeVariables);
	for (auto s : re->freeVariables) {
	    cout << s << " ";
	}
	cout << endl;
    }
}

void emitStateTree(ostream& out) { 
    for (auto func : *func_list) {
	func->genStateTree();
	func->emitStateTree(out);
    }
}

void genPredTree() {
    for (auto re : *re_list) {
	re->genPredTree();
	cout << "Generated tree is: " << endl;
	re->predTree->print();
    }
}

void genFSM() {
    for (auto re : *re_list) {
	FSM* fsm = re->toFSM(re->predTree);
	fsm->assignStateId();
	fsm->populatePredToTree();
	cout << "Generated fsm is: " << endl;
	fsm->print();

	re->simplifyPredTree(re->predTree->root);
	re->predTree->print();
	//	re->genPredTree();
	//	re->toFSM(re->predTree);
    }
}

void emitUpdate(ostream &out) {
    for (auto func : *func_list) {
	func->emitUpdate(out);
    }
}

void emitEval(ostream &out) {
    for (auto func : *func_list) {
	func->emitEval(out);
    }
}

void addExternalInfo(ostream& out, string file) {
    ifstream f(file);
    string line;

    while (getline(f, line)) {
	out << line << endl;
    }

    f.close();
}

int main(int argc, char **argv) {
    ++argv, --argc;  /* skip over program name */
    if ( argc > 0 )
	yyin = fopen( argv[0], "r" );
    else
	yyin = stdin;

    re_list = new list<RE*>();
    pred_list = new list<list<Expr*>*>();

    if (yyparse())
	return -1;

    ofstream out;
    out.open ("a.cpp");

//    cout<<"Renaming variables by adding scope..."<<endl;
//    addScopeToVariables();

    cout << "Getting parameters..." << endl;
    getFreeVariables();

    cout << "Generating predicate tree..." << endl;
    genPredTree();

    cout << "Generating FSM..." << endl;
    genFSM();

    cout << "Adding information at the beginning..." << endl;
    addExternalInfo(out, "pre.cpp");

    cout << "Emiting state tree..." << endl;
    emitStateTree(out);

    cout << "Emitting update code..." << endl;
    emitUpdate(out);

    cout << "Emitting evaluation code..." << endl;
    emitEval(out);

    cout << "Adding information at the end..." << endl;
    addExternalInfo(out, "post.cpp");
    
    out.close();
    return 0;
}

