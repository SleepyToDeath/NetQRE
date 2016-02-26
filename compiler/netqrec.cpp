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
// substitude every functions for its call
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

    cout << "Getting parameters..." << endl;
    getFreeVariables();

    return 0;
}

