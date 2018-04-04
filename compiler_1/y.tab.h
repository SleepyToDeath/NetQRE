/* A Bison parser, made by GNU Bison 2.5.  */

/* Bison interface for Yacc-like parsers in C
   
      Copyright (C) 1984, 1989-1990, 2000-2011 Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     NUM = 258,
     ID = 259,
     BOOL = 260,
     FIELD = 261,
     SPLIT = 262,
     ITER = 263,
     IF = 264,
     ELSE = 265,
     FUNKEY = 266,
     SFUNKEY = 267,
     TYPE = 268,
     OR = 269,
     AND = 270,
     EQUAL = 271,
     NOTEQUAL = 272,
     PIPE = 273,
     NEG = 274
   };
#endif
/* Tokens.  */
#define NUM 258
#define ID 259
#define BOOL 260
#define FIELD 261
#define SPLIT 262
#define ITER 263
#define IF 264
#define ELSE 265
#define FUNKEY 266
#define SFUNKEY 267
#define TYPE 268
#define OR 269
#define AND 270
#define EQUAL 271
#define NOTEQUAL 272
#define PIPE 273
#define NEG 274




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 2068 of yacc.c  */
#line 20 "grammar.y"

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



/* Line 2068 of yacc.c  */
#line 106 "y.tab.h"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

extern YYSTYPE yylval;


