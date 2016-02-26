#include <iostream>
#include <sstream>
#include <stdio.h>
#include <map>
#include "ast.h"
#include "util.h"

using namespace std;

// settings
extern bool compose;
extern bool online;

extern map<Expr*, int> var_map;
extern Tree* tree;
extern bool is_number(const string&);
//extern map<string, Var*> re_var_map;
extern map<string, FunBase*> funTable;

ValExpr::ValExpr(double value) : value(value) {}

double ValExpr::getValue() {
    return value;
}

void ValExpr::emitEval(ostream & out, string indent) {
    out << value;
}

BoolExpr::BoolExpr(bool value) : value(value) {}

void BoolExpr::emitEval(ostream & out, string indent) {
    out << indent << value;
}

IdExpr::IdExpr(string id) : id(id) {}

void IdExpr::emitUpdate(ostream& out) {
    if (funTable.find(id)!=funTable.end()) {
	out << id << "_update(last);" << endl;
    } 
}

void IdExpr::emitEval(ostream & out, string indent) {
    // If a streaming func
    if (funTable.find(id)!=funTable.end()) {
	SFun *sfun = (SFun*)funTable[id];
	out << id << '(';
	if (sfun->arglist != NULL) {
	    for (list<Arg*>::iterator it=sfun->arglist->begin();
		    it!=sfun->arglist->end(); it++) {
		out << (*it)->id;
		out << ", ";
	    }
	}
	out << "last)";
    } else {
	out << indent << id;
    }
}

MemExpr::MemExpr(Expr* expr, string id) : main_expr(expr), id(id) {
    expr->parent = this;
}

void MemExpr::emitEval(ostream& out, string indent) {
    out << indent;
    main_expr->emitEval(out);
    out << "." << id;
}

FunCallExpr::FunCallExpr(string id, list<Expr *> *args) : id(id), args(args) {
    if (args == NULL)
	return;
    for (auto arg : *args) {
	arg->parent = this;
    }
}

void FunCallExpr::emitEval(ostream & out, string indent) {
    if (compose) {
	funTable[id]->block->final_expr->emitEval(out);
    } else {
	out << indent << id << "(";
	if (args != NULL) {
	    list<Expr*>::iterator it=args->begin();
	    (*it)->emitEval(out);
	    for (list<Expr*>::iterator it= ++args->begin(); it!=args->end(); it++) {
		out << ",";
		(*it)->emitEval(out);
	    }
	}
	out << ")";
    }
}

void FunCallExpr::getFreeVariables() {
    for (auto expr : *args) {
	append(expr->freeVariables, freeVariables);
	expr->getFreeVariables();
	append(freeVariables, expr->freeVariables);
    }
}

void FunCallExpr::emitUpdate(ostream& out) {
    if (funTable.find(id)!=funTable.end()) {
	out << id << "_update(last);" << endl;
    } 

    for (auto expr : *args) {
	expr->emitUpdate(out);
    }
}

void FunCallExpr::emitDataStructureType(ostream& out, int level) {
    for (auto expr : *args) {
	expr->emitDataStructureType(out, level);
    }
}

void FunCallExpr::emitCheck(ostream& out, int level) {
    for (auto expr : *args) {
	expr->emitCheck(out, level);
    }
}

BiopExpr::BiopExpr(Expr* expr1, Expr* expr2) : left(expr1), right(expr2) {
    expr1->parent = this;
    expr2->parent = this;
}

void BiopExpr::emitUpdate(ostream& out) {
    left->emitUpdate(out);
    right->emitUpdate(out);
}

void BiopExpr::getFreeVariables() {
    append(left->freeVariables, freeVariables);
    append(right->freeVariables, freeVariables);
    left->getFreeVariables();
    for (string var : left->freeVariables) {
	freeVariables.push_back(var);
    }
    right->getFreeVariables();
    for (string var : right->freeVariables) {
	bool exist = false;
	for (string var2 : freeVariables) {
	    if (var2.compare(var)==0) {
		exist = true;
		break;
	    }
	}
	if (!exist)
	    freeVariables.push_back(var);
    }
}

void BiopExpr::emitDataStructureType(ostream& out, int level) {
    left->emitDataStructureType(out, level);
    right->emitDataStructureType(out, level);
}

void BiopExpr::emitCheck(ostream& out, int level) {
    left->emitCheck(out, level);
    right->emitCheck(out, level);
}

PlusExpr::PlusExpr(Expr* expr1, Expr* expr2) : BiopExpr(expr1, expr2) {}

void PlusExpr::emitEval(ostream & out, string indent) {
    left->emitEval(out, indent);
    out << "+";
    right->emitEval(out);
}

MinusExpr::MinusExpr(Expr* expr1, Expr* expr2) :BiopExpr(expr1, expr2) {}

void MinusExpr::emitEval(ostream & out, string indent) {
    left->emitEval(out, indent);
    out << "-";
    right->emitEval(out);
}

TimesExpr::TimesExpr(Expr* expr1, Expr* expr2) : BiopExpr(expr1, expr2) {}

void TimesExpr::emitEval(ostream & out, string indent) {
    left->emitEval(out, indent);
    out << "*";
    right->emitEval(out);
}

void TimesExpr::emitUpdateChange(ostream& out, Node* child, string oldValue, string newValue) {
    //TODO 123
    //parent->emitUpdateChange(out, this, "", "");
}

DivideExpr::DivideExpr(Expr* expr1, Expr* expr2) : BiopExpr(expr1, expr2) {}

void DivideExpr::emitEval(ostream & out, string indent) {
    left->emitEval(out, indent);
    out << "/";
    right->emitEval(out);
}

LessExpr::LessExpr(Expr* expr1, Expr* expr2) : BiopExpr(expr1, expr2) {}

void LessExpr::emitEval(ostream & out, string indent) {
    left->emitEval(out, indent);
    out << "<";
    right->emitEval(out);
}

void LessExpr::emitRevPred(ostream & out, string indent) {
    out << "{ ";
    right->emitEval(out);
    out << "," << " {" ;
    out << "Field::";
    left->emitEval(out, indent);
    out << ", ";
    out << "Field::naf";
    out << "}";
    out << "}";
}

GreaterExpr::GreaterExpr(Expr* expr1, Expr* expr2) : BiopExpr(expr1, expr2) {}

void GreaterExpr::emitEval(ostream & out, string indent) {
    left->emitEval(out, indent);
    out << ">";
    right->emitEval(out);
}

void GreaterExpr::emitRevPred(ostream & out, string indent) {
    out << "{ ";
    right->emitEval(out);
    out << "," << " {" ;
    out << "Field::naf";
    out << ", ";
    out << "Field::";
    left->emitEval(out, indent);
    out << "}";
    out << "}";
}


EqualExpr::EqualExpr(Expr* expr1, Expr* expr2) : BiopExpr(expr1, expr2) {}

void EqualExpr::emitEval(ostream& out, string indent) {
    left->emitEval(out, indent);
    out << "==";
    right->emitEval(out);
}

void EqualExpr::emitRevPred(ostream & out, string indent) {
    right->emitEval(out);
    out << "," << " {" ;
    out << "Field::";
    left->emitEval(out, indent);
    out << ", ";
    out << "Field::";
    left->emitEval(out, indent);
    out << "}";
}

//TODO
TreeNode* EqualExpr::addToTree(TreeNode* root) {
//    ostringstream convert;
//    right->emitEval(convert);
//    string name = convert.str();
//    Var *var = re_var_map[name];
//
//    if (root->var == var) {
//	// find a 
//	right 
//	
//    }
}

NotEqualExpr::NotEqualExpr(Expr* expr1, Expr* expr2) : BiopExpr(expr1, expr2) {}

void NotEqualExpr::emitEval(ostream& out, string indent) {
    left->emitEval(out, indent);
    out << " != ";
    right->emitEval(out);
}

AndExpr::AndExpr(Expr* expr1, Expr* expr2) : BiopExpr(expr1, expr2) {}

void AndExpr::emitEval(ostream& out, string indent) {
    left->emitEval(out, indent);
    out << "&&";
    right->emitEval(out);
}


OrExpr::OrExpr(Expr* expr1, Expr* expr2) : BiopExpr(expr1, expr2) {}

void OrExpr::emitEval(ostream& out, string indent) {
    left->emitEval(out, indent);
    out << "||";
    right->emitEval(out);
}


PipeExpr::PipeExpr(Expr* left, Expr* right) : left(left), right(right) {
    left->parent = this;
    right->parent = this;
}

void PipeExpr::getFreeVariables() {
    append(left->freeVariables, freeVariables);
    append(right->freeVariables, freeVariables);
    left->getFreeVariables();
    for (string var : left->freeVariables) {
	freeVariables.push_back(var);
    }
    append(right->freeVariables, left->freeVariables);
    right->getFreeVariables();
    for (string var : right->freeVariables) {
	bool exist = false;
	for (string var2 : freeVariables) {
	    if (var2.compare(var)==0) {
		exist = true;
		break;
	    }
	}
	if (!exist)
	    freeVariables.push_back(var);
    }
}

void PipeExpr::emitUpdate(ostream& out) {
    cout << "PipeExpr: emitUpdate" << endl;
    // This is an almost right version. ..
    //left->emitUpdate(out);
    //out << "last = ";
    //left->emitEval(out);
    //out << ";" << endl;
    // if no packet output, then return
    //out << "if (last!=NULL) {" << endl;
    //right->emitUpdate(out);
    //out << "}" << endl;

    // This implementation assumes left is a single packet filter
    //left->emitUpdate(out);
    //out << "last = ";
    //left->emitEval(out);
    //out << ";" << endl;
    // if no packet output, then return
    //out << "if (last!=NULL) {" << endl;
    right->emitUpdate(out);
    //out << "}" << endl;
}

void PipeExpr::emitEval(ostream& out, string) {
    //left->emitEval(out);
    right->emitEval(out);
}

void PipeExpr::emitCheck(ostream& out, int level) {
    left->emitCheck(out, level);
    right->emitCheck(out, level);
}

void PipeExpr::emitDataStructureType(ostream& out, int level) {
    left->emitDataStructureType(out, level);
    right->emitDataStructureType(out, level);
}

void PipeExpr::emitUpdateChange(ostream&, Node*, string, string) {
    cout << "Pipe: emitUpdateChange" << endl;
}

UnaryExpr::UnaryExpr(Expr* expr) : sub_expr(expr) {
    expr->parent = this;
}

void UnaryExpr::emitUpdate(ostream& out) {
    sub_expr->emitUpdate(out);
}

void UnaryExpr::getFreeVariables() {
    append(sub_expr->freeVariables, freeVariables);

    sub_expr->getFreeVariables();
    for (string var : sub_expr->freeVariables) {
	freeVariables.push_back(var);
    }
}

void UnaryExpr::emitDataStructureType(ostream& out, int level) {
    sub_expr->emitDataStructureType(out, level);
}

void UnaryExpr::emitCheck(ostream& out, int level) {
    sub_expr->emitCheck(out, level);
}

UnaryMinusExpr::UnaryMinusExpr(Expr* expr) : UnaryExpr(expr) {}

void UnaryMinusExpr::emitEval(ostream& out, string indent) {
    out << indent <<  "-";
    sub_expr->emitEval(out);
}


NegateExpr::NegateExpr(Expr* expr) : UnaryExpr(expr) {}

void NegateExpr::emitEval(ostream& out, string indent) {
    out << indent <<  "!";
    sub_expr->emitEval(out);
}


IfExpr::IfExpr(Expr* test, Block* block1, Block* block2) : test(test), then_block(block1), else_block(block2) {
    test->parent = this;
    block1->parent = this;
    if (block2!=NULL)
	block2->parent = this;
}

void IfExpr::emitEval(ostream& out, string indent) {
    out << indent << "if " << "(";
    test->emitEval(out);
    out << ")";
    out << " {" << endl;
    //string newIndent = indent + "  ";
    then_block->emitEval(out, indent+INDENT);
    out << endl;
    out << indent << "}"; 
    if (else_block != NULL) {
	out << " else {" << endl;
	else_block->emitEval(out, indent+INDENT);
	out << endl <<indent << "}"; 
    }
}

void IfExpr::emitDataStructure(ostream& out) {
    test->emitDataStructure(out);
    then_block->final_expr->emitDataStructure(out);
    if (else_block != NULL)
	else_block->final_expr->emitDataStructure(out);
}

void IfExpr::emitDataStructureType(ostream& out, int level) {
    test->emitDataStructureType(out, level);
    then_block->final_expr->emitDataStructureType(out, level);
    if (else_block != NULL)
	else_block->final_expr->emitDataStructureType(out, level);
}


void IfExpr::emitUpdate(ostream& out) {
    test->emitUpdate(out);
    then_block->final_expr->emitUpdate(out);
    if (else_block != NULL)
	else_block->final_expr->emitUpdate(out);
}

void IfExpr::emitCheck(ostream& out, int level) {
    test->emitCheck(out, level);
    then_block->final_expr->emitCheck(out, level);
    if (else_block != NULL)
	else_block->final_expr->emitCheck(out, level);
}

ChoiceExpr::ChoiceExpr(Expr* test, Expr* expr1, Expr* expr2) : test(test), yes_expr(expr1), no_expr(expr2) {
    test->parent = this;
    expr1->parent = this;
    if (expr2 != NULL)
	expr2->parent = this;
    cout << "new ChoiceExpr" << endl; 
}

void ChoiceExpr::emitEval(ostream& out, string indent) {
    out << "(";
    test->emitEval(out, indent);
    out << ")";
    out << " ? ";
    yes_expr->emitEval(out);
    if (no_expr != NULL) {
	out << " : ";
	no_expr->emitEval(out);
    } else {
	out << " : 0";
    }
}

void ChoiceExpr::emitDataStructureType(ostream& out, int level) {
    test->emitDataStructureType(out, level);
    yes_expr->emitDataStructureType(out, level);
    if (no_expr != NULL)
	no_expr->emitDataStructureType(out, level);
}

void ChoiceExpr::emitDataStructure(ostream& out) {
    test->emitDataStructure(out);
    yes_expr->emitDataStructure(out);
    if (no_expr != NULL)
	no_expr->emitDataStructure(out);
}

void ChoiceExpr::emitUpdate(ostream& out) {
    cout << "ChoiceExpr: emitUpdate" << endl;
    test->emitUpdate(out);
    yes_expr->emitUpdate(out);
    if (no_expr != NULL)
	no_expr->emitUpdate(out);
}

void ChoiceExpr::emitUpdateChange(ostream& out, Node* child, string oldValue, string newValue) {
    cout << "Choice: emitUpdateChange" << endl;
    ostringstream convert1;
    yes_expr->emitEval(convert1);
    string yes = convert1.str();
    string no;

    if (no_expr == NULL) {
	no = "0";
    } else {
	ostringstream convert2;
	no_expr->emitEval(convert2);
	no = convert2.str();
    }

    if (oldValue.compare("old")==0) {
	parent->emitUpdateChange(out, this, no, yes);
    } else {
	parent->emitUpdateChange(out, this, yes, no);
    }
}

void ChoiceExpr::emitResetState(ostream& out) {
    test->emitResetState(out);
    yes_expr->emitResetState(out);
    if (no_expr != NULL) 
	no_expr->emitResetState(out);
}

void ChoiceExpr::emitCheck(ostream& out, int level) {
    test->emitCheck(out, level);
    yes_expr->emitCheck(out, level);
    if (no_expr != NULL) 
	no_expr->emitCheck(out, level);
}

AggExpr::AggExpr(string aggop, Expr* expr, string varID) : aggop(aggop), expr(expr), varID(varID) {
    expr->parent = this;
}

//TODO
void AggExpr::emitEval(ostream& out, string indent) {
    out << "ret_sum_" << this;
}

void AggExpr::emitDataStructureType(ostream& out, int level) {
    if (freeVariables.empty()) {
	if (level==0) {
	    out << "int sum_" << this 
		<< " = 0;" << endl;
	}
    } else {
	string var = freeVariables.back();
	if (tree->name_to_id[var]+1 == level) {
	    out << "int sum_" << this 
		<< " = 0;" << endl;
	}
    }
    expr->emitDataStructureType(out, level);
}

void AggExpr::emitCheck(ostream& out, int level) {
    if (level < 0) {
	out << "int ret_sum_" << this << " = 0;"
	    << endl;
	return;
    }

    if (freeVariables.empty()) {
	if (level==0) {
	    out << "ret_sum_" << this 
		<< " = " << "state" 
		<< ".sum_" << this << ";"
		<< endl;
	}
    } else {
	string var = freeVariables.back();
	if (tree->name_to_id[var]+1 == level) {
	    out << "ret_sum_" << this 
		<< " = " << "state_" << level
		<< ".sum_" << this << ";"
		<< endl;
	}
    }
    //expr->emitCheck(out, level);
}


void AggExpr::getFreeVariables() {
    append(expr->freeVariables, freeVariables);
    expr->getFreeVariables();
    for (auto var : expr->freeVariables) {
	if (var.compare(varID)!=0)
	    freeVariables.push_back(var);
    }
}

void AggExpr::emitUpdate(ostream& out) {
    cout << "AggExpr: emitUpdate" << endl;
    expr->emitUpdate(out);
}

void AggExpr::emitUpdateChange(ostream& out, Node* child, string oldValue, string newValue) {
    if (freeVariables.empty()) {
	if (aggop.compare("sum")==0) {
	    out << "state" << ".sum_" << this
		<< " += " << newValue 
		<< " - " << oldValue << ";" << endl;
	    parent->emitUpdateChange(out, this, "", "");
	}
    } else {
	if (aggop.compare("sum")==0) {
	    string lastVar = freeVariables.back();
	    out << "state_" << tree->name_to_id[lastVar]+1 << ".sum_" << this
		<< " += " << newValue 
		<< " - " << oldValue << ";" << endl;
	    parent->emitUpdateChange(out, this, "", "");
	} else if (aggop.compare("select")==0) {
	    out << "state[x,y,z] = " << newValue 
		<< ";" << endl;
	} else if (aggop.compare("max")==0) {
	    out << "TODO;" << endl;
	} else if (aggop.compare("min")==0) {
	    out << "TODO;" << endl;
	} else if (aggop.compare("avg")==0) {
	    out << "TODO;" << endl;
	} 
    }
}

void AggExpr::emitResetState(ostream& out) {
    string lastVar = freeVariables.back();
    int level = tree->name_to_id[lastVar]+1;
    if (aggop.compare("sum")==0) {
	out << "state_" << level << ".sum_" << this
	    << " = 0;" << endl;
	expr->emitResetState(out);
    } else if (aggop.compare("select")==0) {
	out << "TODO;" << endl;
    } else if (aggop.compare("max")==0) {
	out << "TODO;" << endl;
    } else if (aggop.compare("min")==0) {
	out << "TODO;" << endl;
    } else if (aggop.compare("avg")==0) {
	out << "TODO;" << endl;
    } 
}

SplitExpr::SplitExpr(string aggop, Expr* expr1, Expr* expr2) : aggop(aggop), expr1(expr1), expr2(expr2) {
    expr1->parent = this;
    expr2->parent = this;
}

void SplitExpr::getFreeVariables() {
    append(expr1->freeVariables, freeVariables);
    append(expr2->freeVariables, freeVariables);

    expr1->getFreeVariables();
    for (string var : expr1->freeVariables) {
	freeVariables.push_back(var);
    }
    expr2->getFreeVariables();
    for (string var : expr2->freeVariables) {
	bool exist = false;
	for (string var2 : freeVariables) {
	    if (var2.compare(var)==0) {
		exist = true;
		break;
	    }
	}
	if (!exist)
	    freeVariables.push_back(var);
    }
}

void SplitExpr::emitDataStructureType(ostream& out, int level) {
    expr1->emitDataStructureType(out, level);
    expr2->emitDataStructureType(out, level);
}

void SplitExpr::emitCheck(ostream& out, int level) {
    expr1->emitCheck(out, level);
    expr2->emitCheck(out, level);
}

IterExpr::IterExpr(string aggop, Expr* expr) : aggop(aggop), expr(expr) {
    expr->parent = this;
}

void IterExpr::getFreeVariables() {
    append(expr->freeVariables, freeVariables);
    expr->getFreeVariables();
    for (string var : expr->freeVariables) {
	freeVariables.push_back(var);
    }
}

void IterExpr::emitDataStructureType(ostream& out, int level) {
    if (freeVariables.empty()) {
	if (level==0) {
	    out << "int sum_" << this 
		<< " = 0;" << endl;
	}
    } else {
	string var = freeVariables.back();
	if (tree->name_to_id[var]+1 == level) {
	    out << "int sum_" << this 
		<< " = 0;" << endl;
	}
    }
    expr->emitDataStructureType(out, level);
}

void IterExpr::emitCheck(ostream& out, int level) {
    if (level < 0) {
	out << "int ret_sum_" << this 
	    << " = 0;"
	    << endl;
	return;
    }

    if (freeVariables.empty()) {
	if (level==0) {
	    out << "ret_sum_" << this;
	    out << " = " << "state";
	    out << ".sum_" << this << ";"
		<< endl;
	}
	return;
    }

    string var = freeVariables.back();
    if (tree->name_to_id[var]+1 == level) {
	out << "ret_sum_" << this 
	    << " = " << "state_" << level
	    << ".sum_" << this << ";"
	    << endl;
    }
//    expr->emitCheck(out, level);
}

void IterExpr::emitUpdate(ostream& out) {
    expr->emitUpdate(out);
}

void IterExpr::emitEval(ostream& out, string indent) {
    out << "ret_sum_" << this;
}

void IterExpr::emitUpdateChange(ostream& out, Node* child, string oldValue, string newValue) {
    if (freeVariables.empty()) {
	if (aggop=="sum") {
	    out << "state" << ".sum_" << this 
		<< "+=" << newValue << ";" << endl;
	}
    } else {
	string lastVar = freeVariables.back();
	int level = tree->name_to_id[lastVar]+1;
	if (aggop=="sum") {
	    out << "state_" << level << ".sum_" << this 
		<< "+=" << newValue << ";" << endl;
	}
    }
    expr->emitResetState(out);
    parent->emitUpdateChange(out, this, "", "");
}

Block::Block(list<Decl*> *decllist, Expr* expr) : decl_list(decllist), final_expr(expr) {
    if (decllist != NULL) {
	for (auto decl : *decllist) {
	    decl->parent = this;
	}
    }
    expr->parent = this;
}

void Block::emitEval(ostream& out, string indent) {
    if (decl_list!=NULL) {
	for (list<Decl*>::iterator it=decl_list->begin();
		it!=decl_list->end(); it++) {
	    (*it)->emitEval(out, indent);
	    out << endl;
	}
    }
    string newIndent = INDENT + indent;
    //string newIndent(" ");
    //newIndent = newIndent + indent;

    if (dynamic_cast<const IfExpr*>(final_expr) != 0) {
	final_expr->emitEval(out, indent);
    } else {
	out << indent << "return ";
	final_expr->emitEval(out);
	out << ";";
    }
    //final_expr->emitEval(out);
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

void VarDecl::emitEval(ostream& out, string indent) {
    out << indent << type << " " << id << "=";
    expr->emitEval(out);
}

Decl::Decl() {}

Arg::Arg(string  type, string id) : type(type), id(id) {}

void Arg::emitEval(ostream& out, string indent) {
    out << indent << type << " " << id;
}


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

void FunBase::emitEval(ostream& out, string indent) {
    // emit function head
    out << indent << type << " " << id;
    out << '(';
    if (arglist != NULL) {
	for (list<Arg*>::iterator it=arglist->begin();
		it!=arglist->end(); it++) {
	    (*it)->emitEval(out);
	    out << ", ";
	}
    }
    out << "Packet* last";
    out << ") {" << endl;
    // emit function body
    string newIndent(INDENT+indent);
    block->emitEval(out, newIndent);
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

// RE
extern map<RE*, int> re_id_map;
// This is for parser.cpp
//void RE::emitEval(ostream& out, string indent) {
//    out << indent << "sm" << re_id_map[this]
//	<< "->check()";
//}

// The version for parser2.cpp
void RE::emitEval(ostream& out, string indent) {
    out << "ret_state_" << this;
}

SingleRE::SingleRE(list<Expr*>* expr) : pred(expr) {}

//void SingleRE::emitEval(ostream& out, string indent) {
//    out << indent << "SingleRE";
//}

void SingleRE::emitInitCode(ostream& out, string indent) {
    out << indent << "new SingleRE(" << endl;
    emitPred(pred, out, indent+INDENT);
    out << endl << indent << ")";
}

void RE::emitDataStructure(ostream& out) {
    emitDataStructure(out, freeVariables.begin());
}

void RE::emitDataStructureType(ostream& out, int level) {
    if (freeVariables.empty()) {
	if (level>0)
	    return;
	out << "int"
	    << " state_" << this << " = " 
	    <<fsm->initState << ";" << endl;
	return;
    }

    string var = freeVariables.back();
    if (tree->name_to_id[var]+1 == level) {
	out << "int"
	    << " state_" << this << " = " 
	    <<fsm->initState << ";" << endl;
    }
}

void RE::emitCheck(ostream& out, int level) {
    if (level < 0) {
	out << "bool ret_state_" << this 
	    << " = false;" << endl; 
	return;
    }

    if (freeVariables.empty()) {
	if (level==0) {
	    out << "if (state" << ".state_" << this
		<< "=="
		<< *fsm->finalStates.begin()
		<< ") {" << endl;
	    out << "ret_state_" << this << " = " 
		<< "true;" << endl << "}" << endl;
	}
    }

    string var = freeVariables.back();
    if (tree->name_to_id[var]+1 == level) {
	out << "if (state_" << level << ".state_" << this
	    << "=="
	    << *fsm->finalStates.begin()
	    << ") {" << endl;
	out << "ret_state_" << this << " = " 
	    << "true;" << endl << "}" << endl;
    }
}


void RE::emitDataStructureType(ostream& out, list<string>::iterator startit) {
    for (auto s = startit; 
	    s != freeVariables.end();
	    s++) {
	out << "map<int, ";
    }
    out << "int";
    for (auto s = startit; 
	    s != freeVariables.end();
	    s++) {
	out << ">";
    }
}

void RE::emitDataStructure(ostream& out, list<string>::iterator startit) {
    // normal state map
    emitDataStructureType(out, startit);
    out << " state_" << this << endl;

    // default state map
    emitDataStructureType(out, ++startit);
    out << " state_" << this << "_default" << endl;
}

/* 
 * This version output a single function for returning state
 * 
 * */
//void RE::emitCheck(ostream& out) {
//    out << "bool check_state_" << this << "(Packet* last) {" << endl;
//    string nodeName = "state";
//
//    for (auto var : freeVariables) {
//	if (is_number(var))
//	    continue;
//	ostringstream convert;
//	convert << "state_" << tree->name_to_id[var]+1;
//	string subNodeName = convert.str();
//
//	out << "Node_" << tree->name_to_id[var]+1 
//	    << "& state_" << tree->name_to_id[var]+1
//	    << ";" << endl;
//
//	out << "if (" << var << " not in " 
//	    << nodeName << ".state_map) {" 
//	    << endl;
//	out << "state_" << tree->name_to_id[var]+1
//	    << " = " << nodeName << ".default_state;" 
//	    << endl;
//	out << "} else {" << endl
//	    << "state_" << tree->name_to_id[var]+1
//	    << " = " << nodeName 
//	    << ".state_map[" << var << "];"
//	    << endl;
//	out << "}" << endl;
//	nodeName = subNodeName;
//    }
//
//    out << "if (" << nodeName << ".state_" << this 
//	<< "==" << *fsm->finalStates.begin()
//	<< ") {" << endl
//	<< "return true;" << endl << "}" << endl;
//    out << "return false;" << endl;
//    out << "}" << endl << endl;
//}

void RE::emitUpdate(ostream& out) {
    emitUpdate(out, "state", tree->root); 
}


/*
 * Only handles re's with equalities 
   TODO:: general cases
 */
void RE::emitUpdate(ostream& out, string DSname, TreeNode *node) {
    cout << "RE update at Node: " << node->name << endl;

    // case 1: no free variables 
    if (freeVariables.empty()) {
	for (auto trans : node->transitions) {
	    void* from = trans.first;
	    void* to = trans.second;
	    out << "if (" 
		<< DSname << ".state_" << this
	       	<< " == " << from << ") {" << endl;
	    out << DSname << ".state_" << this
		<< " = " << to << ";" << endl;
	    if (online)
		parent->emitUpdateChange(out, this, "old", "new");
	    out << "}" << endl;
	}
	return;
    }

    string lastVar = freeVariables.back();


    if (node->tree->name_to_id[node->name]
	    == node->tree->name_to_id[lastVar]+1) {
	for (auto trans : node->transitions) {
	    void* from = trans.first;
	    void* to = trans.second;
	    out << "if (" 
		<< DSname << ".state_" << this
	       	<< " == " << from << ") {" << endl;
	    out << DSname << ".state_" << this
		<< " = " << to << ";" << endl;
	    if (online)
		parent->emitUpdateChange(out, this, "old", "new");
	    out << "}" << endl;
	}
	return;
    }

    // If this node is not a variable for this RE,
    // then go through all its children
    if (!isIn(node->name, freeVariables)) {
	if (is_number(node->name)) {
	    for (auto it=node->childrenMap.begin();
		    it!=node->childrenMap.end();
		    it++) {
		string field = it->first;
		TreeNode* child = it->second;
		emitUpdate(out, "state", child);
	    }
	    return;
	} else {
//	    for (auto it=node->childrenMap.begin();
//		    it!=node->childrenMap.end();
//		    it++) {
//		string field = it->first;
//		TreeNode* child = it->second;
//		emitUpdate(out, DSname, child);
//	    }

	    for (auto it=node->childrenMap.begin();
		    it!=node->childrenMap.end();
		    it++) {
		string field = it->first;
		TreeNode* child = it->second;

		// This is a hack, it assumes that this case 
		// appears only when using piping.
		// need to re-do when have time.
		ostringstream convert;
		convert << "state_";
		convert << 1+tree->name_to_id[node->name];
		emitUpdate(out, convert.str(), child);
	    }

	    return;
	}
    }

    if (is_number(node->name)) {
	for (auto it=node->childrenMap.begin();
		it!=node->childrenMap.end();
		it++) {
	    string field = it->first;
	    TreeNode* child = it->second;
	    out << "if (" << field << " == " 
		<< node->name << ") { " << endl;
	    emitUpdate(out, "state", child);
	    out << "}" << endl;
	}
	return;
    }
    
    for (auto it=node->childrenMap.begin();
	    it!=node->childrenMap.end();
	    it++) {
	string field = it->first;
	// fork a branch
	out << "auto it" << tree->name_to_id[node->name] 
	    << " = "
	    << DSname << ".state_map.find(" << field << ");" << endl;
	out << "if (it" << tree->name_to_id[node->name]
	    << " == " << DSname << ".state_map.end()) { " << endl;
	out << "it" << tree->name_to_id[node->name]
	    << " = "
	    << DSname << ".state_map.insert({" 
	    << field << ", " << DSname << ".default_state}).first" 
	    << ";" << endl;
	out << "}" << endl; 
//    }
//
//    for (auto it=node->childrenMap.begin();
//	    it!=node->childrenMap.end();
//	    it++) {
//	string field = it->first;
	TreeNode* child = it->second;

	ostringstream nameConvert;
	nameConvert << "state_" << 1+node->tree->name_to_id[node->name];
	string subMapName = nameConvert.str();

	ostringstream typeConvert;
	// TODO
//	emitDataStructureType(typeConvert, ++freeVariables.begin());
//	string subMapType = typeConvert.str();
	typeConvert << "Node_" << tree->name_to_id[node->name]+1;
	string subMapType = typeConvert.str();
	out << subMapType << "& " << subMapName << " = " 
	    << "it" << tree->name_to_id[node->name]
	    << ".second;" << endl;
	emitUpdate(out, subMapName, child);
    }


//    for (auto it=node->childrenMap.begin();
//	    it!=node->childrenMap.end();
//	    it++) {
//	string field = it->first;
//	TreeNode* child = it->second;
//
//	out << "if (" << field << " in " << DSname << ") { " <<endl;
//
//	// Type and name for submap 
//	ostringstream typeConvert;
//	for (auto it = node->tree->name_to_id.find(node->name); 
//		it != --node->tree->name_to_id.end();
//		it++) {
//	    typeConvert << "map<int, ";
//	}
//	typeConvert << "int";
//	for (auto it = node->tree->name_to_id.find(node->name); 
//		it != --node->tree->name_to_id.end();
//		it++) {
//	    typeConvert << ">";
//	}
//	string subMapType = typeConvert.str();
//
//	ostringstream nameConvert;
//	nameConvert << "state_" << node;
//	string subMapName = nameConvert.str();
//	out << subMapType << "& " << subMapName << " = " << DSname << "[" << field << "];" << endl;
//	emitUpdate(out, subMapName, child);
//	out << "} else {" << endl;
//	// else
//	out << subMapType << " " << subMapName << " = ";
//	out << "default" ;
//        out << ";" << endl;
//	emitUpdate(out, subMapName, child);
//	out << DSname << "["<<field<<"] = "<< subMapName << ";" <<endl;
//	out << "}" << endl;
//    }
}

void RE::emitResetState(ostream& out) {
    if (freeVariables.empty()) {
	out << "state" << ".state_" << this
	    << " = " << fsm->initState << ";" << endl;
	return;
    }
    string lastVar = freeVariables.back();
    int level = tree->name_to_id[lastVar]+1;
    out << "state_" << level << ".state_" << this
	<< " = " << fsm->initState << ";" << endl;
}

FSM* SingleRE::toFSM(Tree *tree) {
    list<Expr*> emptyPred;
    if (pred == NULL) {
	pred = new list<Expr*>();
    }

    set<TreeNode*> predNodes = addPredicateAt(tree->root, *pred);

    fsm = new FSM();
    State *state1 = fsm->addInitState();
    State *state2 = fsm->addFinalState();
    fsm->addTransition(state1, predNodes, state2);

    cout << endl << "single" << endl;
    fsm->print();


    return fsm;
}

void emitPred(list<Expr*>* pred, ostream& out, string indent) {
    if (pred==NULL) {
	out << indent << "new Pred(" ;
	out << "true";
	out << ")";
    } else {
	list<Expr*>::iterator it = pred->begin();
	out << indent << "new SymbolicPred{" << endl;
	out << indent+INDENT << "{";
	(*it)->emitRevPred(out);
	out << "}";
	it++;
	for (; it!=pred->end(); it++) {
	    out << "," << endl;
	    out << indent+INDENT << "{";
	    (*it)->emitRevPred(out);
	    out << "}";
	}
	out << endl;
	out << indent << "}";
    }
}

ConcatRE::ConcatRE(RE* re1, RE* re2) : re1(re1), re2(re2) {}

//void ConcatRE::emitEval(ostream& out, string indent) {
//    out << indent << "sm" << re_id_map[this]
//	<< "->check()";
//}

void ConcatRE::emitInitCode(ostream& out, string indent) {
    out << indent << "new ConcatRE(" << endl;
    re1->emitInitCode(out, indent+INDENT);
    out << "," << endl;
    re2->emitInitCode(out, indent+INDENT);
    out << endl << indent <<  ")";
}

FSM* ConcatRE::toFSM(Tree *tree) {
    cout << endl << "concat" << endl;
    FSM *fsm1 = re1->toFSM(tree);
    FSM *fsm2 = re2->toFSM(tree);

    fsm1->print();
    fsm2->print();

    for (auto state : fsm2->states) {
	fsm1->states.insert(state);
    }

    for (auto state : fsm1->finalStates) {
	fsm1->addEpsilonTransition(state, fsm2->initState);
    }

    fsm1->finalStates.clear();
    for (auto state : fsm2->finalStates) {
	fsm1->finalStates.insert(state);
    }

    cout << "before determinized" << endl;
    fsm1->print();
    fsm = determinize(fsm1);
    cout << "after determinized" << endl;
    fsm->print();
    fsm->minimize();
    cout << "after minimized" << endl;
    fsm->print();
    return fsm;
}

StarRE::StarRE(RE* re) : re(re) {}

//void StarRE::emitEval(ostream& out, string indent) {
//    re->emitInitCode(out);
//    out << indent << "StarRE";
//}

void StarRE::emitInitCode(ostream& out, string indent) {
    out << indent << "new StarRE(" << endl;
    re->emitInitCode(out, indent+INDENT);
    out << endl << indent <<  ")";
}

FSM* StarRE::toFSM(Tree *tree) {
    FSM *fsm1 = re->toFSM(tree);
    for (auto state : fsm1->finalStates) {
	fsm1->addEpsilonTransition(state, fsm1->initState);
    }
    State *state = fsm1->initState;
    State *newState = fsm1->addInitState(true);

    cout << endl << "star" << endl;
    fsm1->addEpsilonTransition(newState, state);
    cout << "Before deter" << endl;
    fsm1->print();

    fsm = determinize(fsm1);
    cout << "After deter" << endl;
    fsm->print();

    fsm->minimize();
    cout << "After minimize" << endl;

    fsm->print();

    return fsm;
}

UnionRE::UnionRE(RE* re1, RE* re2) : re1(re1), re2(re2) {}

//void UnionRE::emitEval(ostream& out, string indent) {
//    out << indent << "UnionRE";
//}

void UnionRE::emitInitCode(ostream& out, string indent) {
    out << "new UnionRE(";
    re1->emitInitCode(out);
    out << ",";
    re2->emitInitCode(out);
    out << ")";
}

/*****************
    Class Methods
*****************/
void IfExpr::getFreeVariables() {
    append(test->freeVariables, freeVariables);
    append(then_block->final_expr->freeVariables, freeVariables);
    if (else_block != NULL) {
	append(else_block->final_expr->freeVariables, freeVariables);
    }

    test->getFreeVariables();
    append(then_block->final_expr->freeVariables, test->freeVariables);
    then_block->final_expr->getFreeVariables();
    if (else_block != NULL) {
	append(else_block->final_expr->freeVariables, test->freeVariables);
	else_block->final_expr->getFreeVariables();
    }
}

list<string> IfExpr::getSortedVariables(const list<string>* variables) {
    return else_block->final_expr->freeVariables;
}

void ChoiceExpr::getFreeVariables() {
    append(test->freeVariables, freeVariables);
    test->getFreeVariables();
    for (auto var : test->freeVariables) {
	bool exist = false;
	for (auto findStr : freeVariables) {
	    if (findStr.compare(var) == 0) {
		exist = true;
		break;
	    }
	}
	if (!exist)
	    freeVariables.push_back(var);
    }
    append(yes_expr->freeVariables, test->freeVariables);
    yes_expr->getFreeVariables();
    for (auto var : yes_expr->freeVariables) {
	bool exist = false;
	for (auto findStr : freeVariables) {
	    if (findStr.compare(var) == 0) {
		exist = true;
		break;
	    }
	}
	if (!exist)
	    freeVariables.push_back(var);

	for (auto findStr : freeVariables) {
	    if (findStr.compare(var) == 0) 
		break;
	    freeVariables.push_back(var);
	}
    }
    if (no_expr!=NULL) {
	append(no_expr->freeVariables, test->freeVariables);
	no_expr->getFreeVariables();
	for (auto var : no_expr->freeVariables) {
	    bool exist = false;
	    for (auto findStr : freeVariables) {
		if (findStr.compare(var) == 0) {
		    exist = true;
		    break;
		}
	    }
	    if (!exist)
		freeVariables.push_back(var);
	}
    }
}

void SFun::getFreeVariables() {
    block->final_expr->getFreeVariables();
}

list<string> SFun::getSortedVariables(const list<string>* variables) {
    return block->final_expr->getSortedVariables(variables);
}

void SingleRE::getFreeVariables() {
    if (pred == NULL)
	return;
    for (auto e : *pred) {
	ostringstream convert;

	BiopExpr* biExpr = (BiopExpr*)(e);
	Expr* right = biExpr->right;
//	if (dynamic_cast<ValExpr*>(right)!=NULL) {
//	    continue;
//	}

	right->emitEval(convert);
	string name = convert.str();

	bool isNew = true;
	for (string s : freeVariables) {
	    if (s.compare(name) == 0) {
		isNew = false;
		break;
	    }
	}

	if (isNew) {
	    cout << "new variable " << name << endl;
	    freeVariables.push_back(name);
	}
    }
}

void ConcatRE::getFreeVariables() {
    re1->getFreeVariables();
    re2->getFreeVariables();

    freeVariables.insert(freeVariables.end(),
	re1->freeVariables.begin(),
	re1->freeVariables.end()
    );

    for (auto s : re2->freeVariables) {
	bool isIn = false;
	for (auto s2 : freeVariables)
	    if (s2.compare(s) == 0) {
		isIn = true;
		break;
	    }
	if (!isIn) {
	    freeVariables.push_back(s);
	}
    }
}

void StarRE::getFreeVariables() {
    re->getFreeVariables();
    freeVariables.insert(freeVariables.end(),
	re->freeVariables.begin(),
	re->freeVariables.end()
    );
}

set<TreeNode*> addPredicateAt(TreeNode* node, list<Expr*> pred) {
    set<TreeNode*> ret;
    if (pred.empty()) {
	ret.insert(node);
	return ret;
    }

    Expr* front = *pred.begin();
    cout << "all expr" << endl;
    for (auto expr : pred) {
	expr->emitEval(cout);
	cout << endl;
    }

    BiopExpr* expr = (BiopExpr*)(front);
    Expr* right = expr->right;

    ostringstream convert;
    right->emitEval(convert);
    string name = convert.str();

    ostringstream convert2;
    expr->left->emitEval(convert2);
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
	expr->emitEval(cout);
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


