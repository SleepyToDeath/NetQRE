#include <iostream>
#include <sstream>
#include <stdio.h>
#include <map>
#include "declarations.h"
#include "util.h"
//#include "packet.h"

using namespace std;

Block::Block(list<Decl*> *decllist, Expr* expr) : decl_list(decllist), final_expr(expr) {
    if (decllist != NULL) {
	for (auto decl : *decllist) {
	    decl->parent = this;
	}
    }
    expr->parent = this;
}

void Block::emit(ostream& out, string indent) {
    if (decl_list!=NULL) {
	for (list<Decl*>::iterator it=decl_list->begin();
		it!=decl_list->end(); it++) {
	    (*it)->emit(out, indent);
	    out << endl;
	}
    }
    string newIndent = INDENT + indent;
    //string newIndent(" ");
    //newIndent = newIndent + indent;

    if (dynamic_cast<const IfExpr*>(final_expr) != 0) {
	final_expr->emit(out, indent);
    } else {
	out << indent << "return ";
	final_expr->emit(out);
	out << ";";
    }
    //final_expr->emit(out);
    //out << endl;
}

void Block::emitUpdate(ostream& out) {
    if (decl_list!=NULL) {
	for (list<Decl*>::iterator it=decl_list->begin();
		it!=decl_list->end(); it++) {
	    (*it)->emitUpdate(out);
	    out << endl;
	}
    }

    final_expr->emitUpdate(out);
}

VarDecl::VarDecl(string type, string id, Expr* expr) : type(type), id(id), expr(expr) {}

void VarDecl::emit(ostream& out, string indent) {
    out << indent << type << " " << id << "=";
    expr->emit(out);
}

Decl::Decl() {}

Arg::Arg(string  type, string id) : type(type), id(id) {}

void Arg::emit(ostream& out, string indent) {
    out << indent << type << " " << id;
}