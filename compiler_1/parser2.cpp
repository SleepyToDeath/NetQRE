#include <iostream>
#include <sstream>
#include <fstream>
#include <stdio.h>
#include <map>
#include <list>
#include "ast.h"
//#include "packet.h"
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

//map<string, Var*> re_var_map;

// substitute rhs of expr in preds by variables
void genarateVar() {
//    for (list<list<Expr*>*>::iterator it=pred_list->begin();
//	    it!=pred_list->end(); it++) {
//	for (list<Expr*>::iterator pit=(*it)->begin(); pit!=(*it)->end(); pit++) {
//	    ostringstream convert;
//
//	    BiopExpr* biExpr = (BiopExpr*)(*pit);
//	    Expr* right = biExpr->right;
//	    if (dynamic_cast<ValExpr*>(right)==NULL) {
//		right->emit(convert);
//
//		Var* x = new Var();
//		x->name = convert.str();
//
//		re_var_map[x->name] = x;
//	    }
//	}
//    }
}

//void genarateVar() {
//    for (list<list<Expr*>*>::iterator it=pred_list->begin();
//	    it!=pred_list->end(); it++) {
//	for (list<Expr*>::iterator pit=(*it)->begin(); pit!=(*it)->end(); pit++) {
//	    ostringstream convert;
//
//	    BiopExpr* biExpr = (BiopExpr*)(*pit);
//	    string biExpr->right->emit(convert);
//
//	    Var* x = new Var();
//	    x->name = convert.str();
//
//	    re_var_map[x->name] = x;
//	}
//    }
//}

list<string> variables;

bool is_number(const std::string& s)
{
    std::string::const_iterator it = s.begin();
    while (it != s.end() && std::isdigit(*it)) ++it;
    return !s.empty() && it == s.end();
}

void generateDataStructureType(ostream& out=cout) {
    int i=tree->id_to_name.size()-1; 
    out << "// leaf level" << endl;
    out << "struct Node_" << i << "{" << endl;
    for (auto func : *func_list) {
	func->emitDataStructureType(out, i);
    }
    out << "};" << endl;

    for (i=tree->id_to_name.size()-2; i>-1; i--) {
	string var = tree->id_to_name[i];
	if (is_number(var)) {
	    continue;
	}
	out << "// " << tree->id_to_name[i] << endl;
	out << "struct Node_" << i << " {" << endl;
	// if this is time field, then use ordered map
	// note: temporarily searches key word "now" 
	// for this purpose, need to be changed in the 
	// future
	if (var.find("now")!=string::npos) {
	    out << "map<int, Node_" << i+1 << "> state_map;" << endl;
	} else {
	// else unordered_map 
	    out << "unordered_map<int, Node_" << i+1 << "> state_map;" << endl;
	}
	out << "Node_" << i+1 << " default_state;" << endl;
	for (auto func : *func_list) {
	    func->emitDataStructureType(out, i);
	}
	out << "};" << endl;
    }

    out << "Node_" << i+1 << " state;" << endl;
}

void generateCheck(ostream& out=cout) {
    out << endl;
    for (auto func : *func_list) {
	// declaration
	func->emitCheck(out, -1);
    }

    out << endl;
    out << "void check_state(";
    if (!variables.empty()) {
	out << *variables.begin();
	for (auto it=++variables.begin(); it!=variables.end(); it++)
	    out << "," << *it;
    }
    out << ") {" << endl;
    string lastDS = "state";

    for (auto func : *func_list) {
	func->emitCheck(out, 0);
    }

    for (int i=1; i<tree->id_to_name.size(); i++) {
	string var = tree->id_to_name[i-1];
	if (is_number(var))
	    continue;

	out << "Node_" << i 
	    << "& state_" << i << ";" 
	    << endl;
	out << "if (" << var << " in " 
	    << lastDS << ".state_map) {" << endl
	    << "state_" << i << " = "
	    << lastDS << ".state_map["
	    << var
	    << "];" << endl
	    << "} else {" << endl;
	out << "state_" << i << " = "
	    << lastDS << ".default_state;" << endl
	    << "}" << endl;
	for (auto func : *func_list) {
	    func->emitCheck(out, i);
	}

	ostringstream convert;
	convert << "state_" << i;
	lastDS = convert.str();
    }
    out << "}" << endl << endl;
}


void generateDataStructure(ostream& out=cout) {
    for (auto func : *func_list) {
	func->emitDataStructure(out);
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

void generateProcess(ostream& out=cout) {
    for (auto func : *func_list) {
	func->emit(out);
    }
}

void generateUpdate(ostream& out=cout) {
    for (auto func : *func_list) {
	func->emitUpdate(out);
    }

//    for (auto re : *re_list) {
//	ostringstream convert;
//	convert << "state_" << re;
//	re->emitUpdate(out, convert.str(), tree->root);
//    }
}

void generateFSM() {
    for (auto re : *re_list) {
	FSM* fsm = re->toFSM(tree);
	cout << "Generated fsm is: " << endl;
	fsm->print();
	fsm->populatePredToTree();
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

    // Process variables
//    cout << "Generating variable map..." << endl;
//    genarateVar();
//    for (auto it = re_var_map.begin(); 
//	    it != re_var_map.end();
//	    it++) {
//	cout << it->first << ": " << it->second;
//	cout << endl;
//    }

    cout << "Generating free variables..." << endl;
    getFreeVariables();

    cout << "Generating predicate tree..." << endl;
    tree = new Tree(variables);
    tree->print();

    cout << "Generating state machines..." << endl;
    generateFSM();

    tree->print();
    ofstream myfile;
    myfile.open ("a.cpp");


    cout << "Generating state..." << endl;
    generateDataStructureType(myfile);

//    cout << "Generating state..." << endl;
//    //generateDataStructure(cout);
//    generateDataStructure(myfile);
    cout << "Generating check function..." << endl;
    generateCheck(myfile);

    cout << "Generating update function..." << endl;
    generateUpdate(myfile);
    //generateUpdate(cout);
    //

    cout << "Generating process function..." << endl;
    generateProcess(myfile);

    myfile.close();
    return 0;
}

