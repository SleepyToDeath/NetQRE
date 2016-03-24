#include <iostream>
#include <sstream>
#include <stdio.h>
#include <map>
#include "ast.h"
#include "util.h"
//#include "packet.h"

using namespace std;


/**********************************
 * Function declaritions
 * *******************************/
FunBase::FunBase(string type, string id, list<Arg*> *args, Block *block) : type(type), id(id), arglist(args), block(block) {
    if (args != NULL) {
	for (auto arg : *args) {
	    arg->parent = this;
	}
    }
    block->parent = this;
}

void FunBase::emit(ostream& out, string indent) {
    // emit function head
    out << indent << type << " " << id;
    out << '(';
    if (arglist != NULL) {
	for (list<Arg*>::iterator it=arglist->begin();
		it!=arglist->end(); it++) {
	    (*it)->emit(out);
	    out << ", ";
	}
    }
    out << "Packet* last";
    out << ") {" << endl;
    // emit function body
    string newIndent(INDENT+indent);
    block->emit(out, newIndent);
    out << endl << indent << "}" << endl << endl;
}

Fun::Fun(string type, string id, list<Arg*> *args, Block *block) : FunBase(type, id, args, block) {
    cout << "new function" << endl;
}

SFun::SFun(string type, string id, list<Arg*> *args, Block *block) : FunBase(type, id, args, block) {
    cout << "new stream function" << endl;
    funTable[id] = this;
}

void SFun::emitDataStructureType(ostream& out, int level) {
    block->final_expr->emitDataStructureType(out, level);
}

void SFun::emitDataStructure(ostream& out) {
    block->final_expr->emitDataStructure(out);
}

void SFun::emitUpdate(ostream& out) {
    out << "bool " << id << "_update(Packet *last) {" << endl;
    block->emitUpdate(out);
    out << "return true;" << endl;
    out << "}" << endl << endl;
}

void SFun::emitCheck(ostream& out, int level) {
    block->final_expr->emitCheck(out, level);
}

void SFun::getFreeVariables() {
    block->final_expr->getFreeVariables();
}

list<string> SFun::getSortedVariables(const list<string>* variables) {
    return block->final_expr->getSortedVariables(variables);
}

void SFun::genStateTree() {
    block->final_expr->genStateTree();
}

void SFun::emitStateTree(ostream& out) {
    block->final_expr->emitStateTree(out);
}

