#ifndef AST_H
#define AST_H

#include <string>
#include <list>
#include "fsm.h"
//#include "packet.h"

using namespace std;

class Node {
public:
	Node* parent;

public:
	virtual void emitUpdateChange(ostream&, Node*, string, string) {}
	virtual void emit(ostream&, string="") {}
	// Get free variables at this node
	virtual void getFreeVariables() {};
	// Get the ordered list of variables in the ast
	virtual list<string> getSortedVariables(const list<string>*) {};
	virtual void emitDataStructure(ostream&) {};
	virtual void emitDataStructureType(ostream&, int) {}
	virtual void emitUpdate(ostream&) {};
	virtual void emitResetState(ostream&) {};

	virtual void emitProcess(ostream&) {};
	virtual void emitCheck(ostream&, int) {};
};

class Expr : public Node {
public:
	// add this function once we have the class for packet
	//virtual execute(packet);
	
	// print debug infomation and other info 
	void print();
	double getValue();
	virtual void emit(ostream&, string="") {}
	virtual void emitRevPred(ostream&, string="") {}

	virtual void emitUpdateCode(ostream&, string="") {}
	virtual void emitProcessCode(ostream&, string="") {}
	virtual void emitSummaryCode(ostream&, string="") {}

	// below is for parser2
	list<string> freeVariables;
	virtual TreeNode* addToTree(TreeNode*) {};
	virtual void getFreeVariables() {};
	virtual list<string> getSortedVariables(const list<string>*) {};
	virtual void emitDataStructure(ostream&) {};
	virtual void emitUpdate(ostream&) {};
	virtual void emitResetState(ostream&) {};
};

// values
class ValExpr : public Expr {
protected:
	double value;

public:
	ValExpr(double);
	double getValue();
	virtual void emit(ostream&, string="");
	virtual void emitUpdateCode(ostream&, string="");
	virtual void emitProcessCode(ostream&, string="");
};

// booleans
class BoolExpr : public Expr {
protected:
	bool value;

public:
	BoolExpr(bool);
	double getValue();
	virtual void emit(ostream&, string="");
};

// id
class IdExpr : public Expr {
protected:
	string id;

public:
	IdExpr(string);
	virtual void emit(ostream&, string="");
	virtual void emitUpdate(ostream&);
};


// Expr.field
class MemExpr : public Expr {
protected:
	Expr *main_expr;
	string id;

public:
	MemExpr(Expr*, string);
	virtual void emit(ostream&, string="");
};

// id (Expr, .., expr ) 
class FunCallExpr : public Expr {
protected:
	string id;
	list<Expr*> *args;

public:
	FunCallExpr(string, list<Expr *> *);
	virtual void emit(ostream&, string="");
	virtual void getFreeVariables();
	virtual void emitUpdate(ostream&);
	virtual void emitDataStructureType(ostream&, int);
	virtual void emitCheck(ostream&, int);
};

// constructor for 2 operator expr
class BiopExpr : public Expr {
public:
	Expr *left;
	Expr *right;
	
public:
	BiopExpr(Expr*, Expr*);
	virtual void getFreeVariables();
	virtual void emitUpdate(ostream&);
	virtual void emitDataStructureType(ostream&, int);
	virtual void emitCheck(ostream&, int);
};

// constructor for Expr + expr
class PlusExpr : public BiopExpr {
public:
	PlusExpr(Expr*, Expr*);
	virtual void emit(ostream&, string="");
};

// constructor for Expr - expr
class MinusExpr : public BiopExpr {
public:
	MinusExpr(Expr*, Expr*);
	virtual void emit(ostream&, string="");
};

// constructor for Expr * expr
class TimesExpr : public BiopExpr {
public:
	TimesExpr(Expr*, Expr*);
	virtual void emit(ostream&, string="");
	virtual void emitUpdateChange(ostream&, Node*, string, string);
};

// constructor for Expr / expr
class DivideExpr : public BiopExpr {
public:
	DivideExpr(Expr*, Expr*);
	virtual void emit(ostream&, string="");
};

// constructor for Expr < expr
class LessExpr : public BiopExpr {
public:
	LessExpr(Expr*, Expr*);
	virtual void emit(ostream&, string="");
	virtual void emitRevPred(ostream&, string="");
};

// constructor for Expr > expr
class GreaterExpr : public BiopExpr {
public:
	GreaterExpr(Expr*, Expr*);
	virtual void emit(ostream&, string="");
	virtual void emitRevPred(ostream&, string="");
};

// constructor for Expr == expr
class EqualExpr : public BiopExpr {
public:
	EqualExpr(Expr*, Expr*);
	virtual void emit(ostream&, string="");
	virtual void emitRevPred(ostream&, string="");
	virtual TreeNode* addToTree(TreeNode*);

	//virtual void getFreeVariables();
	//	virtual void emitDataStructure();
};

// constructor for Expr != expr
class NotEqualExpr : public BiopExpr {
public:
	NotEqualExpr(Expr*, Expr*);
	virtual void emit(ostream&, string="");
	//virtual void emitPred(ostream&, string="");
};

// constructor for Expr & expr
class AndExpr : public BiopExpr {
public:
	AndExpr(Expr*, Expr*);
	virtual void emit(ostream&, string="");
};


// constructor for Expr | expr
class OrExpr : public BiopExpr {
public:
	OrExpr(Expr*, Expr*);
	virtual void emit(ostream&, string="");
};

// base class for unary Expressions
class UnaryExpr : public Expr {
protected:
	Expr *sub_expr;

public:
	UnaryExpr(Expr*);
	virtual void emitUpdate(ostream&);
	virtual void getFreeVariables();
	virtual void emitDataStructureType(ostream&, int);
	virtual void emitCheck(ostream&, int);
};

// ! Expr
class NegateExpr : public UnaryExpr {
public:
	NegateExpr(Expr*);
	virtual void emit(ostream&, string="");
};

// - Expr
class UnaryMinusExpr : public UnaryExpr {
public:
	UnaryMinusExpr(Expr*);
	virtual void emit(ostream&, string="");
};

// Expr ? Expr : Expr
class ChoiceExpr : public Expr {
protected:
	Expr* test;
	Expr* yes_expr;
	Expr* no_expr;

public:
	ChoiceExpr(Expr*,Expr*,Expr*);
	virtual void emit(ostream&, string="");

	virtual void getFreeVariables();
	virtual void emitDataStructureType(ostream&, int);
	virtual void emitDataStructure(ostream&);
	virtual void emitUpdate(ostream&);
	void emitUpdateChange(ostream&, Node*, string, string);
	virtual void emitResetState(ostream&);
	virtual void emitCheck(ostream&, int);
};


// agg_op { expr | int x}
class AggExpr : public Expr {
protected:
	string aggop;
	Expr* expr;
	string varID;

public:
	AggExpr(string,Expr*,string);
	virtual void emit(ostream&, string="");
	virtual void getFreeVariables();
	virtual void emitUpdate(ostream&);
	void emitUpdateChange(ostream&, Node*, string, string);
	virtual void emitDataStructureType(ostream&, int);
	virtual void emitResetState(ostream&);
	virtual void emitCheck(ostream&, int);
};

// agg{split(e1,e2)}
class SplitExpr : public Expr {
public:
	string aggop;
	Expr* expr1;
	Expr* expr2;

public:
	SplitExpr(string,Expr*,Expr*);
	//	virtual void emit(ostream&, string="");
	virtual void getFreeVariables();
	//	virtual void emitUpdate(ostream&);
	//	void emitUpdateChange(ostream&, Node*, string, string);
	virtual void emitDataStructureType(ostream&, int);
	virtual void emitCheck(ostream&, int);
};

// agg{iter(e1)}
class IterExpr : public Expr {
public:
	string aggop;
	Expr* expr;

public:
	IterExpr(string, Expr*);
	//	virtual void emit(ostream&, string="");
	virtual void getFreeVariables();
	virtual void emitUpdate(ostream&);
	virtual void emitUpdateChange(ostream&, Node*, string, string);
	virtual void emitDataStructureType(ostream&, int);
	virtual void emitCheck(ostream&, int);
	virtual void emit(ostream&, string="");
};

class PipeExpr : public Expr {
public:
	Expr* left;
	Expr* right;

public:
	PipeExpr(Expr*, Expr*);
	virtual void getFreeVariables();
	virtual void emit(ostream&, string="");
	virtual void emitUpdate(ostream&);
	virtual void emitCheck(ostream&, int);
	virtual void emitUpdateChange(ostream&, Node*, string, string);
	virtual void emitDataStructureType(ostream&, int);
};


class Decl : public Node {
public:
	Decl();
	virtual void emit(ostream&, string="") {};

	// parser2
	list<string> freeVariables;
	virtual void getFreeVariables() {};
	virtual list<string> getSortedVariables(const list<string>*) {};
	virtual void emitDataStructure(ostream&) {};
	virtual void emitUpdate(ostream&) {};
};

class VarDecl : public Decl {
private:
	string type;
	string id;
	Expr* expr;

public:
	VarDecl(string, string, Expr*);
	//virtual void emit();
	virtual void emit(ostream&, string="");
};


class Arg : public Node {
public:
	string type;
	string id;

public:
	Arg(string, string);
	virtual void emit(ostream&, string="");
};


class Block : public Node {
public:
	list<Decl*> *decl_list;
	Expr* final_expr;

public:
	Block(list<Decl*> *, Expr*);
	void emit(ostream&, string="");
	void emitUpdate(ostream&);
	//list<string> getSortedVariables(const list<string>&);
};

// if 
class IfExpr : public Expr {
public:
	Expr* test;
	Block* then_block;
	Block* else_block;

public:
	IfExpr(Expr*, Block*, Block*);
	virtual void emit(ostream&, string="");

	virtual void getFreeVariables();
	virtual void emitDataStructure(ostream&);
	virtual list<string> getSortedVariables(const list<string>*);
	virtual void emitDataStructureType(ostream&, int);
	virtual void emitUpdate(ostream&);
	virtual void emitCheck(ostream&, int);
};



class FunBase : public Decl {
public:
	string type;
	string id;
	list<Arg*>* arglist;
	Block* block;

public:
	FunBase(string, string, list<Arg*>*, Block*);
	virtual void emit(ostream&, string="");
};

class SFun : public FunBase {
public:
	SFun(string, string, list<Arg*>*, Block*);
	//virtual void emit();

	
	// parser2
	virtual void getFreeVariables();
	virtual list<string> getSortedVariables(const list<string>*);
	virtual void emitDataStructureType(ostream&, int);
	virtual void emitDataStructure(ostream&);
	virtual void emitUpdate(ostream&);
	virtual void emitCheck(ostream&, int);
};


class Fun : public FunBase {
public:
	Fun(string, string, list<Arg*>*, Block*);
	//virtual void emit();
};

// regular expression
class RE : public Expr {
public:
	FSM* fsm;
	RE() {}
	virtual void emit(ostream&, string=""); 
	virtual void emitInitCode(ostream&, string="") {}
	virtual FSM* toFSM(Tree*) {}


	// parser2
	virtual void emitDataStructure(ostream&);
	virtual void emitDataStructure(ostream&, list<string>::iterator);
	virtual void emitUpdate(ostream&, string, TreeNode*);
	virtual void emitUpdate(ostream&);
	void emitDataStructureType(ostream&, list<string>::iterator);
	virtual void emitDataStructureType(ostream&, int);
	virtual void emitResetState(ostream&);
	virtual void emitCheck(ostream&, int);
};

class SingleRE : public RE {
protected:
	list<Expr*> *pred;

	set<TreeNode*> treeNodes;

public:
	SingleRE(list<Expr*>*);
	//	virtual void emit(ostream&, string="");
	virtual void emitInitCode(ostream&, string="");
	virtual FSM* toFSM(Tree*);
	virtual void getFreeVariables();
};

class ConcatRE :public  RE {
protected:
	RE* re1;
	RE* re2;

public:
	ConcatRE(RE*, RE*);
	//virtual void emit(ostream&, string="");
	virtual void emitInitCode(ostream&, string="");
	virtual FSM* toFSM(Tree*);
	virtual void getFreeVariables();
};

class UnionRE :public  RE {
protected:
	RE* re1;
	RE* re2;

public:
	UnionRE(RE*, RE*);
	//	virtual void emit(ostream&, string="");
	virtual void emitInitCode(ostream&, string="");
};

class StarRE : public RE {
protected:
	RE* re;

public:
	StarRE(RE*);
	//virtual void emit(ostream&, string="");
	virtual void emitInitCode(ostream&, string="");
	virtual FSM* toFSM(Tree*);
	virtual void getFreeVariables();
};

void emitPred(list<Expr*>*, ostream&, string);
set<TreeNode*> addPredicateAt(TreeNode*, list<Expr*>);



#endif





