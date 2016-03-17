%{
#include <math.h>
#include <string>
#include <list>
#include <map>
#include <stdio.h>
#include "ast.h"

using namespace std;
int yylex();
void yyerror(char const * s);

extern map<string, FunBase*> funTable;
extern list<Decl*>* func_list;
extern list<RE*>* re_list;
extern list<list<Expr*>*>* pred_list;
%}


%union {
    double num;
    char * id;
    bool boolean;
    Expr *exp;
    RE *re;
    Block* block;
    list<Decl*>* decllist;
    list<Expr*>* exprlist;
    SFun* sfun;
    Fun*  fun;
    VarDecl* vardecl;
    Arg* arg;
    list<Arg*>* arglist;
}

%token <num> NUM
%token <id> ID
%token <boolean> BOOL
%token FIELD
%token SPLIT
%token ITER
%token IF
%token ELSE
%token FUNKEY
%token SFUNKEY
%token <num> TYPE
%nonassoc '?'
%nonassoc ':'
%left OR '|'
%left AND 
%nonassoc '!'
%left '>' '<' 
%right EQUAL
%right NOTEQUAL
%left '+' '-'
%left PIPE
%left '*' '/' 
%nonassoc NEG
%left '.'
%left '(' ')' '[' ']'
%type <decllist> decl_list
%type <decllist> block_decl_list
%type <decllist> prog
%type <exprlist> expr_list
%type <block> block
%type <exp> expr
%type <exp> agg_expr
%type <re> re
%type <fun> func
%type <sfun> sfunc
%type <vardecl> var_def
%type <arg> arg
%type <arglist> arglist

%start prog
%%
prog	    : decl_list {$$ = $1; func_list = $$;};
decl_list   : /* empty */
	      {printf("delist\n"); $$ = new list<Decl*>();}
	    | decl_list func {$$ = $1; $$->push_back($2); printf("new func\n");}
	    | decl_list sfunc {$$ = $1; $$->push_back($2); printf("new func\n");}
	    | decl_list var_def ';' {$$ = $1; $$->push_back($2); printf("new var\n");}
;

block_decl_list   : func {$$ = new list<Decl*>(); $$->push_back($1); printf("new func\n");}
	    | sfunc {$$ = new list<Decl*>(); $$->push_back($1); printf("new func\n");}
	    | var_def ';' {$$ = new list<Decl*>(); $$->push_back($1); printf("new var\n");}
	    | block_decl_list func {$$ = $1; $$->push_back($2); printf("new func\n");}
	    | block_decl_list sfunc {$$ = $1; $$->push_back($2); printf("new func\n");}
	    | block_decl_list var_def ';' {$$ = $1; $$->push_back($2); printf("new var\n");}
;

block	    : expr ';'	{ printf("block\n"); $$ = new Block(NULL, $1); }
	    | '{' expr '}' { printf("block\n"); $$ = new Block(NULL, $2); }
	    | '{' block_decl_list expr '}' { printf("block: decl_list expr\n"); $$ = new Block($2, $3); }
;

expr_list    : expr {printf("list\n"); $$ = new list<Expr*>(); $$->push_back($1);}
	     | expr_list ',' expr {$$ = $1; $$->push_back($3);}
;

expr	    : NUM {$$ = new ValExpr($1); printf("new num\n");}
	    | BOOL {$$ = new BoolExpr($1); printf("new bool\n");}
	    | ID {$$ = new IdExpr($1);}
	    | ID '(' ')'{$$ = new FunCallExpr($1, NULL);} /* not sure */
	    | ID '(' expr_list ')'{$$ = new FunCallExpr($1, $3);} /* not sure */
	    | expr '.' ID {$$ = new MemExpr($1, $3);} /* not sure */
	    | '(' expr ')' {printf("()"); $$ = $2;}
	    | expr '+' expr {printf("reduce +\n"); $$ = new Expr();}
	    | expr '-' expr {$$ = new MinusExpr($1, $3);}
	    | expr '*' expr {$$ = new TimesExpr($1, $3);}
	    | expr '/' expr {$$ = new DivideExpr($1, $3);}
	    | expr '>' expr {$$ = new GreaterExpr($1, $3);}
	    | expr '<' expr {$$ = new LessExpr($1, $3);}
	    | expr PIPE expr {printf("pipe\n"); $$ = new PipeExpr($1, $3);}
	    | expr AND expr {$$ = new AndExpr($1, $3);}
	    | expr OR expr {printf("OR\n");$$ = new OrExpr($1, $3);}
	    | expr EQUAL expr {$$ = new EqualExpr($1, $3);}
	    | expr NOTEQUAL expr {$$ = new NotEqualExpr($1, $3);}
	    | '-' expr %prec NEG {$$ = new UnaryMinusExpr($2);}
	    | '!' expr {$$ = new NegateExpr($2);}
	    | IF '(' expr ')' block ELSE block
	      {
		printf("if\n"); $$ = new IfExpr($3, $5, $7);
	      }
	    | IF '(' expr ')' block 
	      {
		$$ = new IfExpr($3, $5, NULL);
	      }
	    | expr '?' expr {$$ = new ChoiceExpr($1, $3, NULL);}
	    | expr '?' expr ':' expr {printf("Choice re\n"); $$ = new ChoiceExpr($1, $3, $5);}
	    | '/' re '/' {printf("re expr\n"); $$ = $2; re_list->push_back($2);}
	    | agg_expr {$$ = $1;}
	    | ID '{' SPLIT '(' expr ',' expr ')' '}' 
		{
		    printf("split func\n"); $$ = new SplitExpr($1, $5, $7);
		}
	    | ID '{' ITER '(' expr ')' '}' 
		{
		    printf("iter func\n"); $$ = new IterExpr($1, $5);
		}

;
agg_expr    : ID '{' expr '|' ID ID '}' 
	      {
		printf("new agg expr\n");
		$$ = new AggExpr($1, $3, $6);
	      }
	    | ID '{' expr '}' 
	      {
		$$ = new AggExpr($1, $3, "");
	      }
;

re	    : '[' expr_list ']' { printf("single re\n"); $$ = new SingleRE($2); pred_list->push_back($2);}// expr list
	    | '.' {printf("any\n"); $$ = new SingleRE(NULL);}// expr list
	    | re re %prec '*' {printf("concat\n"); $$ = new ConcatRE($1, $2);}
	    | re '*' %prec NEG {printf("star\n"); $$ = new StarRE($1);}
	    | re '|' re %prec OR {printf("union\n"); $$ = new UnionRE($1,$3);}
	    | '(' re ')' {$$ = $2;}
;


sfunc	    : SFUNKEY ID ID '(' arglist ')' '=' block
	      {
		printf("sfunc\n");
		$$ = new SFun($2, $3, $5, $8);
	      }
	    | SFUNKEY ID ID '(' ')' '=' block
	      {
		$$ = new SFun($2, $3, NULL, $7);
	      }
	    | SFUNKEY ID ID '=' block
	      {
		$$ = new SFun($2, $3, NULL, $5);
	      }
;

func	    : FUNKEY ID ID '(' arglist ')' '=' block
	      {
		$$ = new Fun($2, $3, $5, $8);
	      }
	    | FUNKEY ID ID '(' ')' '=' block
	      {
		$$ = new Fun($2, $3, NULL, $7);
	      }
	    | FUNKEY ID ID '=' block
	      {
		$$ = new Fun($2, $3, NULL, $5);
	      }
;

var_def	    : ID ID '=' expr
	      {
		// should assignment be allowed?
		printf("reduce vardef\n");
		$$ = new VarDecl($1, $2, $4);
	      }
;

arg	    : ID ID {$$ = new Arg($1,$2);}
;

arglist	    : arg {printf("single arg\n"); $$ = new list<Arg*>(); $$->push_back($1);}
	    | arglist ',' arg {printf("arglist\n");$$ = $1; $$->push_back($3);}
;


%%

void yyerror( char const * str ) {
    printf( "error: %s\n", str );
}

