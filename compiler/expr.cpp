#include <iostream>
#include <sstream>
#include <stdio.h>
#include <map>
#include "declarations.h"
#include "util.h"
//#include "packet.h"

using namespace std;

void Expr::genStateTree() {
    stateTree = new list<StateInfo>();

    for (string var : freeVariables) {
	StateInfo si;
	si.varName = var;
	si.typeName = "Node_" + name + "_" + var;
	stateTree->push_back(si);
    }
}

void Expr::addLeafToStateTree() {
    if (stateTree == NULL)
	return;

    StateInfo si;
    si.varName = "leaf";
    si.typeName = "Node_" + name + "_" + "leaf";

    stateTree->push_back(si);
}

void Expr::emitStateTree(ostream& out) {
    int level = stateTree->size();
    if (level == 0)
	return;

    auto it = stateTree->rbegin();
    out << "// " << it->varName << endl;
    out << "struct " << it->typeName << "{" << endl;
    for (auto state : it->states) {
	out << state << endl;
    }
    out << "};" << endl;

    auto lastIt = it;
    for (++it, --level;
	    it != stateTree->rend();
	    it++, level--, lastIt++) {
	out << "// " << it->varName << endl;

	out << "struct " << it->typeName << "{" << endl;

	for (auto state : it->states) {
	    out << state << endl;
	}

	out << "unordered_map<int, " << lastIt->typeName << "> "
	    << "stateMap;" << endl;
	out << lastIt->typeName << " default_state;" << endl;
	out << "};" << endl;
    }
}

void Expr::emitDeclInUpdate(ostream& out) {
    if (stateTree == NULL)
	return;

    auto stateIt = stateTree->begin();
    auto nextIt = stateIt;
    nextIt++;
    for (; stateIt != stateTree->end() && nextIt != stateTree->end(); stateIt++, nextIt++) {
	string itName = "it_" + stateIt->varName;
	string nodeName = "node_" + nextIt->varName;

	out << "unordered_map<int, " << nextIt->typeName << ">::iterator " 
	<< itName << ";" << endl;
	out << nextIt->typeName << " *" << nodeName << ";" << endl;
    }
    out << endl;
}

ValExpr::ValExpr(double value) : value(value) {}

double ValExpr::getValue() {
	return value;
}

void ValExpr::emit(ostream & out, string indent) {
	out << indent << value;
}

void ValExpr::emitUpdateCode(ostream & out, string indent) {
	out << indent << value;
}

void ValExpr::emitProcessCode(ostream & out, string indent) {
	out << indent << value;
}

string ValExpr::emitEval(ostream & out) {
    ostringstream convert;
    convert << value;
    return convert.str();
}

BoolExpr::BoolExpr(bool value) : value(value) {}

void BoolExpr::emit(ostream & out, string indent) {
	out << indent << value;
}

IdExpr::IdExpr(string id) : id(id) {}

void IdExpr::emitUpdate(ostream& out) {
	if (funTable.find(id)!=funTable.end()) {
		out << id << "_update(last);" << endl;
	} 
}

void IdExpr::emit(ostream & out, string indent) {
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

void IdExpr::addScopeToVariables(string scope) {
	//rename the id with the parent's scope
	id = id+"_"+scope;
}

MemExpr::MemExpr(Expr* expr, string id) : main_expr(expr), id(id) {
	expr->parent = this;
}

void MemExpr::emit(ostream& out, string indent) {
	out << indent;
	main_expr->emit(out);
	out << "." << id;
}

void MemExpr::addScopeToVariables(string scope) {
	//rename the id with the parent's scope
	id = id+"_"+scope;
}

FunCallExpr::FunCallExpr(string id, list<Expr *> *args) : id(id), args(args) {
	if (args == NULL)
	return;
	for (auto arg : *args) {
		arg->parent = this;
	}
}

void FunCallExpr::emit(ostream & out, string indent) {
	if (compose) {
		funTable[id]->block->final_expr->emit(out);
	} else {
		out << indent << id << "(";
		if (args != NULL) {
			list<Expr*>::iterator it=args->begin();
			(*it)->emit(out);
			for (list<Expr*>::iterator it= ++args->begin(); it!=args->end(); it++) {
				out << ",";
				(*it)->emit(out);
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

void FunCallExpr::addScopeToVariables(string scope) {
	for (auto expr : *args) {
		expr->addScopeToVariables(scope);
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

void BiopExpr::addScopeToVariables(string scope) {
	right->addScopeToVariables(scope);
	left->addScopeToVariables(scope);
}

void BiopExpr::emitDataStructureType(ostream& out, int level) {
	left->emitDataStructureType(out, level);
	right->emitDataStructureType(out, level);
}

void BiopExpr::emitCheck(ostream& out, int level) {
	left->emitCheck(out, level);
	right->emitCheck(out, level);
}

void BiopExpr::genStateTree() {
    left->genStateTree();
    right->genStateTree();
}

void BiopExpr::addState(list<StateInfo>* stateTree) {

    this->stateTree = stateTree;
	left->addState(stateTree);
	right->addState(stateTree);
}

void BiopExpr::emitStateTree(ostream &out) {
    left->emitStateTree(out);
    right->emitStateTree(out);
}

PlusExpr::PlusExpr(Expr* expr1, Expr* expr2) : BiopExpr(expr1, expr2) {}

void PlusExpr::emit(ostream & out, string indent) {
	left->emit(out, indent);
	out << "+";
	right->emit(out);
}

MinusExpr::MinusExpr(Expr* expr1, Expr* expr2) :BiopExpr(expr1, expr2) {}

void MinusExpr::emit(ostream & out, string indent) {
	left->emit(out, indent);
	out << "-";
	right->emit(out);
}

TimesExpr::TimesExpr(Expr* expr1, Expr* expr2) : BiopExpr(expr1, expr2) {}

void TimesExpr::emit(ostream & out, string indent) {
	left->emit(out, indent);
	out << "*";
	right->emit(out);
}

void TimesExpr::emitUpdateChange(ostream& out, Node* child, string oldValue, string newValue) {
	//TODO 123
	//parent->emitUpdateChange(out, this, "", "");
}

DivideExpr::DivideExpr(Expr* expr1, Expr* expr2) : BiopExpr(expr1, expr2) {}

void DivideExpr::emit(ostream & out, string indent) {
	left->emit(out, indent);
	out << "/";
	right->emit(out);
}

LessExpr::LessExpr(Expr* expr1, Expr* expr2) : BiopExpr(expr1, expr2) {}

void LessExpr::emit(ostream & out, string indent) {
	left->emit(out, indent);
	out << "<";
	right->emit(out);
}

void LessExpr::emitRevPred(ostream & out, string indent) {
	out << "{ ";
	right->emit(out);
	out << "," << " {" ;
	out << "Field::";
	left->emit(out, indent);
	out << ", ";
	out << "Field::naf";
	out << "}";
	out << "}";
}

GreaterExpr::GreaterExpr(Expr* expr1, Expr* expr2) : BiopExpr(expr1, expr2) {}

void GreaterExpr::emit(ostream & out, string indent) {
	left->emit(out, indent);
	out << ">";
	right->emit(out);
}

void GreaterExpr::emitRevPred(ostream & out, string indent) {
	out << "{ ";
	right->emit(out);
	out << "," << " {" ;
	out << "Field::naf";
	out << ", ";
	out << "Field::";
	left->emit(out, indent);
	out << "}";
	out << "}";
}


EqualExpr::EqualExpr(Expr* expr1, Expr* expr2) : BiopExpr(expr1, expr2) {}

void EqualExpr::emit(ostream& out, string indent) {
	left->emit(out, indent);
	out << "==";
	right->emit(out);
}

void EqualExpr::emitRevPred(ostream & out, string indent) {
	right->emit(out);
	out << "," << " {" ;
	out << "Field::";
	left->emit(out, indent);
	out << ", ";
	out << "Field::";
	left->emit(out, indent);
	out << "}";
}

//TODO
TreeNode* EqualExpr::addToTree(TreeNode* root) {
	//    ostringstream convert;
	//    right->emit(convert);
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

void NotEqualExpr::emit(ostream& out, string indent) {
	left->emit(out, indent);
	out << " != ";
	right->emit(out);
}

AndExpr::AndExpr(Expr* expr1, Expr* expr2) : BiopExpr(expr1, expr2) {}

void AndExpr::emit(ostream& out, string indent) {
	left->emit(out, indent);
	out << "&&";
	right->emit(out);
}


OrExpr::OrExpr(Expr* expr1, Expr* expr2) : BiopExpr(expr1, expr2) {}

void OrExpr::emit(ostream& out, string indent) {
    left->emit(out, indent);
    out << "||";
    right->emit(out);
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
    mergeTo(right->freeVariables, left->freeVariables);
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

void PipeExpr::genStateTree() {
    Expr::genStateTree();
    addLeafToStateTree();
    addState(stateTree);
}

void PipeExpr::addState(list<StateInfo>* stateTree) {
    this->stateTree = stateTree;
    left->addState(stateTree);
    right->addState(stateTree);
}

//TODO
void PipeExpr::emitEvalAndUpdate(ostream& out, stateIterator stateIt) {
    out << "// In " << endl;

    string lastVar = left->freeVariables.back();
    string itName = "it_" + stateIt->varName;
    string nodeName = "node_" + stateIt->varName;
    string mapName = nodeName + "->state_map";

    out << "for (" << itName << "=" << mapName << ".begin(); "
	<< itName << "!=" << mapName << ".end();" << itName << "++) {" << endl;

    auto nextStateIt = next(stateIt);
    string childNodeName = "node_" + nextStateIt->varName;

    out << childNodeName
	<< " = &(" << itName << "->second);" << endl
	<< endl;

    //right->emit(rightOut, stateIt, true);

    if (stateIt->varName == lastVar) {
	out << "// eval for left" << endl;
	left->emitEval(out, nextStateIt);
	out << "// update for right" << endl;
	right->emitUpdate(out);
    } else 
	emitEvalAndUpdate(out, nextStateIt);

    out << "}" << endl;

    out << childNodeName
	<< " = &(" << nodeName << "->default_state);" << endl
	<< endl;

    //right->emit(rightOut, stateIt, true);

    if (stateIt->varName == lastVar) {
	out << "// eval for left" << endl;
	left->emitEval(out, nextStateIt);
	out << "// update for right" << endl;
	right->emitUpdate(out);
    } else
	emitEvalAndUpdate(out, nextStateIt);
}

void PipeExpr::emitUpdate(ostream& out) {
    cout << "PipeExpr: emitUpdate" << endl;
    // This is an almost right version. ..
    left->emitUpdate(out);

    if (left->freeVariables.empty()) {
    } else {
	string lastVar = left->freeVariables.back();
	list<StateInfo>::iterator stateIt;
	//emitEvalAndUpdate(out, stateTree->begin()); 

	for (stateIt = stateTree->begin(); stateIt != stateTree->end(); stateIt++) {
	    string itName = "it_" + stateIt->varName;
	    string nodeName = "node_" + stateIt->varName;
	    string mapName = nodeName + "->state_map";

	    out << "for (" << itName << "=" << mapName << ".begin(); "
		<< itName << "!=" << mapName << ".end();" << itName << "++) {" << endl;

	    auto nextStateIt = next(stateIt);
	    string childNodeName = "node_" + nextStateIt->varName;

	    out << childNodeName
		<< " = &(" << itName << "->second);" << endl
		<< endl;

	    if (stateIt->varName == lastVar) {
		out << "// eval for left" << endl;
		left->emitEval(out, nextStateIt);
		out << "// update for right" << endl;
		right->emitUpdate(out);
		out << "}" << endl;
		break;
	    }
	}
    }

//    out << "last = ";
//    left->emitEval(out);
//    out << ";" << endl;
//    // if no packet output, then return
//    out << "if (last!=NULL) {" << endl;
//    right->emitUpdate(out);
//    //out << "}" << endl;




    // This implementation assumes left is a single packet filter
    //left->emitUpdate(out);
    //out << "last = ";
    //left->emit(out);
    //out << ";" << endl;
    // if no packet output, then return
    //out << "if (last!=NULL) {" << endl;
    //right->emitUpdate(out);
    //out << "}" << endl;
}

void PipeExpr::emit(ostream& out, string) {
	//left->emit(out);
	right->emit(out);
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

void UnaryMinusExpr::emit(ostream& out, string indent) {
	out << indent <<  "-";
	sub_expr->emit(out);
}


NegateExpr::NegateExpr(Expr* expr) : UnaryExpr(expr) {}

void NegateExpr::emit(ostream& out, string indent) {
	out << indent <<  "!";
	sub_expr->emit(out);
}


IfExpr::IfExpr(Expr* test, Block* block1, Block* block2) : test(test), then_block(block1), else_block(block2) {
	test->parent = this;
	block1->parent = this;
	if (block2!=NULL)
	block2->parent = this;
}

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

void IfExpr::addScopeToVariables(string scope) {
	then_block->final_expr->addScopeToVariables(scope);
	if (else_block != NULL) {
		else_block->final_expr->addScopeToVariables(scope);
	}
}


list<string> IfExpr::getSortedVariables(const list<string>* variables) {
	return else_block->final_expr->freeVariables;
}
void IfExpr::emit(ostream& out, string indent) {
	out << indent << "if " << "(";
	test->emit(out);
	out << ")";
	out << " {" << endl;
	//string newIndent = indent + "  ";
	then_block->emit(out, indent+INDENT);
	out << endl;
	out << indent << "}"; 
	if (else_block != NULL) {
		out << " else {" << endl;
		else_block->emit(out, indent+INDENT);
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
}

void ChoiceExpr::emit(ostream& out, string indent) {
	out << "(";
	test->emit(out, indent);
	out << ")";
	out << " ? ";
	yes_expr->emit(out);
	if (no_expr != NULL) {
		out << " : ";
		no_expr->emit(out);
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


void ChoiceExpr::emitCheck(ostream& out, int level) {
	test->emitCheck(out, level);
	yes_expr->emitCheck(out, level);
	if (no_expr != NULL) 
	no_expr->emitCheck(out, level);
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
    //	append(yes_expr->freeVariables, test->freeVariables);
    //	yes_expr->getFreeVariables();
    //	for (auto var : yes_expr->freeVariables) {
    //		bool exist = false;
    //		for (auto findStr : freeVariables) {
    //			if (findStr.compare(var) == 0) {
    //				exist = true;
    //				break;
    //			}
    //		}
    //		if (!exist)
    //		freeVariables.push_back(var);
    //
    //		for (auto findStr : freeVariables) {
    //			if (findStr.compare(var) == 0) 
    //			break;
    //			freeVariables.push_back(var);
    //		}
    //	}
    //	if (no_expr!=NULL) {
    //		append(no_expr->freeVariables, test->freeVariables);
    //		no_expr->getFreeVariables();
    //		for (auto var : no_expr->freeVariables) {
    //			bool exist = false;
    //			for (auto findStr : freeVariables) {
    //				if (findStr.compare(var) == 0) {
    //					exist = true;
    //					break;
    //				}
    //			}
    //			if (!exist)
    //			freeVariables.push_back(var);
    //		}
    //	}
}

void ChoiceExpr::genStateTree() {
    test->genStateTree();
    yes_expr->genStateTree();
    if (no_expr != NULL)
	no_expr->genStateTree();
}

void ChoiceExpr::addState(list<StateInfo>* stateTree) {
    test->addState(stateTree);
    yes_expr->addState(stateTree);
    if (no_expr != NULL)
	no_expr->addState(stateTree);
}

void ChoiceExpr::emitStateTree(ostream &out) {
    test->emitStateTree(out);
    yes_expr->emitStateTree(out);
}

void ChoiceExpr::emitDeclInUpdate(ostream& out) {
    test->emitDeclInUpdate(out);
    yes_expr->emitDeclInUpdate(out);
    if (no_expr)
	no_expr->emitDeclInUpdate(out);
}

void ChoiceExpr::emitUpdate(ostream& out) {
    test->emitUpdate(out);
    yes_expr->emitUpdate(out);
    if (no_expr != NULL)
	no_expr->emitUpdate(out);
}

void ChoiceExpr::emitUpdateChange(ostream& out, Node* child, string oldValue, string newValue) {
    ostringstream convert1;
    yes_expr->emit(convert1);
    string yes = convert1.str();
    string no;

    if (no_expr == NULL) {
	no = "0";
    } else {
	ostringstream convert2;
	no_expr->emit(convert2);
	no = convert2.str();
    }

    if (oldValue.compare("false")==0) {
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

string ChoiceExpr::emitEval(ostream& out) {
    emitEval(out, this->stateTree->begin());
}

string ChoiceExpr::emitEval(ostream& out, stateIterator startStateIt) {
    string test_ret = test->emitEval(out, startStateIt);

    string retName = "ret_" + name;
    out << "int " << retName << " = 0;" << endl;

    out << "if (" << test_ret << ") {" << endl;
    string ret = yes_expr->emitEval(out, startStateIt);

    out << retName << " == " << ret << ";" << endl;
    out << "}" << endl;

    if (no_expr) {
	ret = no_expr->emitEval(out, startStateIt);
	out << retName << " == " << ret << ";" << endl;
    }

    return ret;
}


int AggExpr::count = 0;

AggExpr::AggExpr(string aggop, Expr* expr, string varID) : aggop(aggop), expr(expr), varID(varID) {
	expr->parent = this;
}

AggExpr::AggExpr(string aggop, Expr* expr, string varID, string varType) : aggop(aggop), expr(expr), varID(varID), varType(varType) {
	expr->parent = this;
	name = "agg" + to_string(count++);
}

//TODO
void AggExpr::emit(ostream& out, string indent) {
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

void AggExpr::addScopeToVariables(string scope) {
	//call the expr's addScopeToVariables with parents scope 
	//along with it's own name
	expr->addScopeToVariables(scope+"_"+name);
}

void AggExpr::emitUpdate(ostream& out) {
    expr->emitUpdate(out);
}

void AggExpr::emitUpdateChange(ostream& out, Node* child, string oldValue, string newValue) {
    string nodeName = "node_";
    string stateName = "sum_" + name;

    // Only handles sum operator
    // TODO: avg, max, min
    if (freeVariables.empty()) {
	nodeName += varID;
    } else {
	string lastVar = freeVariables.back();
	nodeName += varID;
    }

    // All free variables in the freeVariables list should be
    // ordered.
    out << nodeName << "->" << stateName 
	<< " += " << newValue 
	<< " - " << oldValue << ";" << endl;
    parent->emitUpdateChange(out, this, "", "");
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

void AggExpr::genStateTree() {
    Expr::genStateTree();
    StateInfo si;
    si.varName = varID;
    si.typeName = "Node_" + name + "_" + varID;
    stateTree->push_back(si);

    StateInfo si2;
    si2.varName = "leaf";
    si2.typeName = "Node_" + name + "_" + "leaf";
    stateTree->push_back(si2);

    addState(stateTree);

    expr->addState(stateTree);
}

void AggExpr::addState(list<StateInfo>* stateTree) {

    this->stateTree = stateTree;
    auto last = stateTree->rbegin();

    for (auto it = ++stateTree->rbegin();
    it != stateTree->rend();
    it++) {
	if (isIn(it->varName, freeVariables)) 
	    break;
	else
	    last = it;
    }
    
    string state = "int sum_" + name + ";";
    last->states.push_back(state);
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

void IterExpr::emit(ostream& out, string indent) {
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
