#ifndef DECLARATIONS_H
#define DECLARATIONS_H
#include "ast.h"

const string INDENT("  ");
// settings
extern bool compose;
extern bool online;

extern map<Expr*, int> var_map;
extern Tree* tree;
extern bool is_number(const string&);
//extern map<string, Var*> re_var_map;
extern map<string, FunBase*> funTable;

#endif