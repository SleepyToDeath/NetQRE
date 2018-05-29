#include <string>
#include <list>
#include <set>
#include "predtree.h"

using namespace std;

struct StateInfo {
    string name;
    int mapType = 0;
    list<string> states;
};

class Node {
    public:
	Node* parent;
	//int id;
	list<string> freeVariables;
	list<StateInfo> *stateTree;
	Tree *predTree;
    

    public:
	// Get free variables at this node
	virtual void getFreeVariables() {};

	virtual void emitUpdate(ostream&) {};
	virtual void emitUpdateChange(ostream&, Node*, string, string) {}
	virtual void emitResetState(ostream&) {};
	virtual void emitEval(ostream&) {}

	virtual void emitDataStructure(ostream&) {};

	virtual void genPredTree() {};
	virtual void genStateTree() {};
	virtual void addState(list<StateInfo>&) {};
};

class Expr : public Node {
    public:
	// print debug infomation and other info 
	void print();

	// below is for parser2
	virtual void getFreeVariables() {};

	virtual void emitUpdate(ostream&) {};
	virtual void emitResetState(ostream&) {};

	virtual void emitDataStructure(ostream&) {};
};

// values
class ValExpr : public Expr {
    protected:
	double value;

    public:
	ValExpr(double);
	double getValue() {return value;}
	virtual void emitEval(ostream&);
};

// booleans
class BoolExpr : public Expr {
    protected:
	bool value;

    public:
	BoolExpr(bool);
	double getValue() {return value;}
	virtual void emitEval(ostream&);
};

// id
class IdExpr : public Expr {
    protected:
	string id;

    public:
	IdExpr(string);
	virtual void emitEval(ostream&);
	virtual void emitUpdate(ostream&);
};


// Expr.field
class MemExpr : public Expr {
    protected:
	Expr *main_expr;
	string id;

    public:
	MemExpr(Expr*, string);
};

// id (Expr, .., expr ) 
class FunCallExpr : public Expr {
    protected:
	string id;
	list<Expr*> *args;

    public:
	FunCallExpr(string, list<Expr *> *);
	virtual void getFreeVariables();
	virtual void emitUpdate(ostream&);
};

class BiopExpr : public Expr {
    public:
	Expr *left;
	Expr *right;
	
    public:
	BiopExpr(Expr*, Expr*);
	virtual void getFreeVariables();
	virtual void emitUpdate(ostream&);
};

// constructor for Expr + expr
class PlusExpr : public BiopExpr {
    public:
	PlusExpr(Expr*, Expr*);
};

// constructor for Expr - expr
class MinusExpr : public BiopExpr {
    public:
	MinusExpr(Expr*, Expr*);
};

// constructor for Expr * expr
class TimesExpr : public BiopExpr {
    public:
	TimesExpr(Expr*, Expr*);
};

// constructor for Expr / expr
class DivideExpr : public BiopExpr {
    public:
	DivideExpr(Expr*, Expr*);
};

// constructor for Expr < expr
class LessExpr : public BiopExpr {
    public:
	LessExpr(Expr*, Expr*);
};

// constructor for Expr > expr
class GreaterExpr : public BiopExpr {
    public:
	GreaterExpr(Expr*, Expr*);
};

// constructor for Expr == expr
class EqualExpr : public BiopExpr {
    public:
	EqualExpr(Expr*, Expr*);
	virtual void getFreeVariables();
};

// constructor for Expr != expr
class NotEqualExpr : public BiopExpr {
    public:
	NotEqualExpr(Expr*, Expr*);
	//virtual void emitPred(ostream&, string="");
};

// constructor for Expr & expr
class AndExpr : public BiopExpr {
    public:
	AndExpr(Expr*, Expr*);
};


// constructor for Expr | expr
class OrExpr : public BiopExpr {
    public:
	OrExpr(Expr*, Expr*);
};

// base class for unary Expressions
class UnaryExpr : public Expr {
    protected:
	Expr *sub_expr;

    public:
	UnaryExpr(Expr*);
	virtual void emitUpdate(ostream&);
	virtual void getFreeVariables();
};

// ! Expr
class NegateExpr : public UnaryExpr {
    public:
	NegateExpr(Expr*);
};

// - Expr
class UnaryMinusExpr : public UnaryExpr {
    public:
	UnaryMinusExpr(Expr*);
};

// Expr ? Expr : Expr
class ChoiceExpr : public Expr {
    protected:
	Expr* test;
	Expr* yes_expr;
	Expr* no_expr;

    public:
	ChoiceExpr(Expr*,Expr*,Expr*);

	virtual void getFreeVariables();
	virtual void emitDataStructure(ostream&);
	virtual void emitUpdate(ostream&);
	void emitUpdateChange(ostream&, Node*, string, string);
	virtual void emitResetState(ostream&);
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
//	virtual void emitUpdate(ostream&);
//	void emitUpdateChange(ostream&, Node*, string, string);
//	virtual void emitDataStructureType(ostream&, int);
//	virtual void emitResetState(ostream&);
//	virtual void emitCheck(ostream&, int);
};

// agg{split(e1,e2)}
class SplitExpr : public Expr {
    public:
	string aggop;
	Expr* expr1;
	Expr* expr2;

    public:
	SplitExpr(string,Expr*,Expr*);
	virtual void getFreeVariables();
//	virtual void emitUpdate(ostream&);
//	void emitUpdateChange(ostream&, Node*, string, string);
//	virtual void emitDataStructureType(ostream&, int);
};

// agg{iter(e1)}
class IterExpr : public Expr {
    public:
	string aggop;
	Expr* expr;

    public:
	IterExpr(string, Expr*);
	virtual void getFreeVariables();
	virtual void emitUpdate(ostream&);
	virtual void emitUpdateChange(ostream&, Node*, string, string);
};

class PipeExpr : public Expr {
    public:
	Expr* left;
	Expr* right;

    public:
	PipeExpr(Expr*, Expr*);
	virtual void getFreeVariables();
};


class Decl : public Node {
    public:
	Decl();

    public:
	virtual void getFreeVariables() {};
};

class VarDecl : public Decl {
    private:
	string type;
	string id;
	Expr* expr;

    public:
	VarDecl(string, string, Expr*);
	//virtual void emit();
};


class Arg : public Node {
    public:
	string type;
	string id;

    public:
	Arg(string, string);
};


class Block : public Node {
    public:
	list<Decl*> *decl_list;
	Expr* final_expr;

    public:
	Block(list<Decl*> *, Expr*);
	void emitUpdate(ostream&);
};

// if 
class IfExpr : public Expr {
    public:
	Expr* test;
	Block* then_block;
	Block* else_block;

    public:
	IfExpr(Expr*, Block*, Block*);

	virtual void getFreeVariables();
	virtual void emitDataStructure(ostream&);
	virtual void emitUpdate(ostream&);
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
	virtual void emitDataStructure(ostream&);
	virtual void emitUpdate(ostream&);
};


class Fun : public FunBase {
    public:
	Fun(string, string, list<Arg*>*, Block*);
	//virtual void emit();
};

// regular expression
class RE : public Expr {
    public:
	//FSM* fsm;
	RE() {}

    public:
	// parser2
	virtual void emitDataStructure(ostream&);
	virtual void emitDataStructure(ostream&, list<string>::iterator);
	virtual void emitUpdate(ostream&, string, TreeNode*);
	virtual void emitUpdate(ostream&);
	virtual void emitResetState(ostream&);
	virtual void emitCheck(ostream&, int);

	virtual void genStateTree();
};

class SingleRE : public RE {
    protected:
	list<Expr*> *pred;

	set<TreeNode*> treeNodes;

    public:
	SingleRE(list<Expr*>*);
//	virtual void emit(ostream&, string="");
	virtual void getFreeVariables();
};

class ConcatRE :public  RE {
    protected:
	RE* re1;
	RE* re2;

    public:
	ConcatRE(RE*, RE*);
	//virtual void emit(ostream&, string="");
	virtual void getFreeVariables();
};

class UnionRE :public  RE {
    protected:
	RE* re1;
	RE* re2;

    public:
	UnionRE(RE*, RE*);
//	virtual void emit(ostream&, string="");
	virtual void getFreeVariables();
};

class StarRE : public RE {
    protected:
	RE* re;

    public:
	StarRE(RE*);
	//virtual void emit(ostream&, string="");
	virtual void getFreeVariables();
};

void emitPred(list<Expr*>*, ostream&, string);
set<TreeNode*> addPredicateAt(TreeNode*, list<Expr*>);









