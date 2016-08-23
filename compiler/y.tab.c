/* A Bison parser, made by GNU Bison 2.5.  */

/* Bison implementation for Yacc-like parsers in C
   
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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.5"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* Using locations.  */
#define YYLSP_NEEDED 0



/* Copy the first part of user declarations.  */

/* Line 268 of yacc.c  */
#line 1 "grammar.y"

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


/* Line 268 of yacc.c  */
#line 90 "y.tab.c"

/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif


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

/* Line 293 of yacc.c  */
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



/* Line 293 of yacc.c  */
#line 182 "y.tab.c"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif


/* Copy the second part of user declarations.  */


/* Line 343 of yacc.c  */
#line 194 "y.tab.c"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int yyi)
#else
static int
YYID (yyi)
    int yyi;
#endif
{
  return yyi;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)				\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack_alloc, Stack, yysize);			\
	Stack = &yyptr->Stack_alloc;					\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  3
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   429

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  40
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  14
/* YYNRULES -- Number of rules.  */
#define YYNRULES  63
/* YYNRULES -- Number of states.  */
#define YYNSTATES  146

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   274

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    19,     2,     2,     2,     2,     2,     2,
      31,    32,    27,    24,    38,    25,    30,    28,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    15,    35,
      21,    39,    20,    14,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    33,     2,    34,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    36,    16,    37,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    17,
      18,    22,    23,    26,    29
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     5,     6,     9,    12,    16,    18,    20,
      23,    26,    29,    33,    36,    40,    45,    47,    51,    53,
      55,    57,    61,    66,    70,    74,    78,    82,    86,    90,
      94,    98,   102,   106,   110,   114,   118,   121,   124,   132,
     138,   142,   148,   152,   154,   164,   172,   180,   185,   189,
     191,   194,   197,   201,   205,   214,   222,   228,   237,   245,
     251,   256,   259,   261
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      41,     0,    -1,    42,    -1,    -1,    42,    50,    -1,    42,
      49,    -1,    42,    51,    35,    -1,    50,    -1,    49,    -1,
      51,    35,    -1,    43,    50,    -1,    43,    49,    -1,    43,
      51,    35,    -1,    46,    35,    -1,    36,    46,    37,    -1,
      36,    43,    46,    37,    -1,    46,    -1,    45,    38,    46,
      -1,     3,    -1,     5,    -1,     4,    -1,     4,    31,    32,
      -1,     4,    31,    45,    32,    -1,    46,    30,     4,    -1,
      31,    46,    32,    -1,    46,    24,    46,    -1,    46,    25,
      46,    -1,    46,    27,    46,    -1,    46,    28,    46,    -1,
      46,    20,    46,    -1,    46,    21,    46,    -1,    46,    26,
      46,    -1,    46,    18,    46,    -1,    46,    17,    46,    -1,
      46,    22,    46,    -1,    46,    23,    46,    -1,    25,    46,
      -1,    19,    46,    -1,     9,    31,    46,    32,    44,    10,
      44,    -1,     9,    31,    46,    32,    44,    -1,    46,    14,
      46,    -1,    46,    14,    46,    15,    46,    -1,    28,    48,
      28,    -1,    47,    -1,     4,    36,     7,    31,    46,    38,
      46,    32,    37,    -1,     4,    36,     8,    31,    46,    32,
      37,    -1,     4,    36,    46,    16,     4,     4,    37,    -1,
       4,    36,    46,    37,    -1,    33,    45,    34,    -1,    30,
      -1,    48,    48,    -1,    48,    27,    -1,    48,    16,    48,
      -1,    31,    48,    32,    -1,    12,     4,     4,    31,    53,
      32,    39,    44,    -1,    12,     4,     4,    31,    32,    39,
      44,    -1,    12,     4,     4,    39,    44,    -1,    11,     4,
       4,    31,    53,    32,    39,    44,    -1,    11,     4,     4,
      31,    32,    39,    44,    -1,    11,     4,     4,    39,    44,
      -1,     4,     4,    39,    46,    -1,     4,     4,    -1,    52,
      -1,    53,    38,    52,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint8 yyrline[] =
{
       0,    77,    77,    79,    80,    81,    82,    85,    86,    87,
      88,    89,    90,    93,    94,    95,    98,    99,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   126,
     130,   131,   132,   133,   134,   138,   144,   149,   155,   156,
     157,   158,   159,   160,   164,   169,   173,   179,   183,   187,
     193,   201,   204,   205
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "NUM", "ID", "BOOL", "FIELD", "SPLIT",
  "ITER", "IF", "ELSE", "FUNKEY", "SFUNKEY", "TYPE", "'?'", "':'", "'|'",
  "OR", "AND", "'!'", "'>'", "'<'", "EQUAL", "NOTEQUAL", "'+'", "'-'",
  "PIPE", "'*'", "'/'", "NEG", "'.'", "'('", "')'", "'['", "']'", "';'",
  "'{'", "'}'", "','", "'='", "$accept", "prog", "decl_list",
  "block_decl_list", "block", "expr_list", "expr", "agg_expr", "re",
  "sfunc", "func", "var_def", "arg", "arglist", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,    63,    58,   124,   269,   270,    33,
      62,    60,   271,   272,    43,    45,   273,    42,    47,   274,
      46,    40,    41,    91,    93,    59,   123,   125,    44,    61
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    40,    41,    42,    42,    42,    42,    43,    43,    43,
      43,    43,    43,    44,    44,    44,    45,    45,    46,    46,
      46,    46,    46,    46,    46,    46,    46,    46,    46,    46,
      46,    46,    46,    46,    46,    46,    46,    46,    46,    46,
      46,    46,    46,    46,    46,    46,    47,    47,    48,    48,
      48,    48,    48,    48,    49,    49,    49,    50,    50,    50,
      51,    52,    53,    53
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     0,     2,     2,     3,     1,     1,     2,
       2,     2,     3,     2,     3,     4,     1,     3,     1,     1,
       1,     3,     4,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     2,     2,     7,     5,
       3,     5,     3,     1,     9,     7,     7,     4,     3,     1,
       2,     2,     3,     3,     8,     7,     5,     8,     7,     5,
       4,     2,     1,     3
};

/* YYDEFACT[STATE-NAME] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       3,     0,     2,     1,     0,     0,     0,     5,     4,     0,
       0,     0,     0,     6,     0,     0,     0,    18,    20,    19,
       0,     0,     0,     0,     0,    60,    43,     0,     0,     0,
       0,     0,     0,     0,    37,    36,    49,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    62,     0,     0,    59,
       0,     0,     0,    56,    21,     0,    16,     0,     0,     0,
       0,     0,     0,     0,    51,    42,    50,    24,    40,    33,
      32,    29,    30,    34,    35,    25,    26,    31,    27,    28,
      23,    61,     0,     0,     0,    20,     0,     0,     8,     7,
       0,    13,     0,     0,    22,     0,     0,     0,     0,    47,
       0,    53,    48,    52,     0,    58,     0,    63,     0,    11,
      10,     0,    14,     9,    55,     0,    17,     0,     0,     0,
      39,    41,    57,    15,    12,    54,     0,     0,     0,     0,
       0,    45,    46,    38,     0,    44
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
      -1,     1,     2,    96,    59,    65,    60,    26,    76,     7,
       8,     9,    56,    57
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -56
static const yytype_int16 yypact[] =
{
     -56,    25,    10,   -56,     5,    22,    46,   -56,   -56,    14,
      19,    55,    58,   -56,   157,   -16,    15,   -56,    38,   -56,
      53,   157,   157,    85,   157,   346,   -56,     7,    52,    16,
      52,    98,   128,   157,   382,    56,   -56,    85,   157,    37,
     261,   157,   157,   157,   157,   157,   157,   157,   157,   157,
     157,   157,   157,    86,    90,    50,   -56,   -26,   139,   -56,
     242,    59,    28,   -56,   -56,    47,   346,    64,    68,   188,
     278,    81,    13,    85,   -56,   -56,    45,   -56,   331,   371,
     382,   391,   391,   391,   399,   145,   145,    97,    56,    56,
     -56,   -56,    52,    65,   101,     9,   139,   206,   -56,   -56,
      71,   -56,    52,    70,   -56,   157,   157,   157,   115,   -56,
      52,   -56,   -56,    45,   157,   -56,    52,   -56,   224,   -56,
     -56,    93,   -56,   -56,   -56,    52,   346,   169,   295,   116,
     111,   360,   -56,   -56,   -56,   -56,   157,   102,   103,    52,
     312,   -56,   -56,   -56,   104,   -56
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -56,   -56,   -56,   -56,   -29,    96,   -14,   -56,   -21,   -55,
     -54,   -53,    44,   117
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const yytype_int16 yytable[] =
{
      25,    63,    39,    98,    99,   100,    93,    34,    35,    10,
      40,    54,    94,    10,     4,    27,    71,    66,    69,    70,
      54,     5,     6,    28,    66,     3,    11,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    55,
      31,   119,   120,   121,    97,    32,    29,   112,    61,    13,
      12,   105,   113,    73,    30,    17,    18,    19,    14,    15,
     103,    20,    16,   115,    74,    75,    94,    36,    37,    31,
      38,    21,    74,   124,    32,    36,    37,    22,    38,   104,
      23,   130,   118,    24,    33,   105,    53,   132,    58,    92,
      90,   126,   127,   128,    91,   106,   135,    73,   102,   107,
     131,    17,    18,    19,   116,    54,   123,    20,    74,   125,
     143,    36,    37,   111,    38,    36,    37,    21,    38,   129,
     138,   139,   140,    22,    51,    52,    23,    53,   134,    24,
      64,    17,    18,    19,    72,    67,    68,    20,   117,   141,
     142,   145,    17,    95,    19,     0,    62,    21,    20,     0,
       5,     6,     0,    22,     0,     0,    23,     0,    21,    24,
      17,    18,    19,     0,    22,     0,    20,    23,     0,     0,
      24,    50,    51,    52,     0,    53,    21,     0,     0,     0,
       0,     0,    22,    41,     0,    23,    42,    43,    24,    44,
      45,    46,    47,    48,    49,    50,    51,    52,     0,    53,
       0,     0,    41,     0,   108,    42,    43,   136,    44,    45,
      46,    47,    48,    49,    50,    51,    52,     0,    53,     0,
      41,     0,     0,    42,    43,   109,    44,    45,    46,    47,
      48,    49,    50,    51,    52,     0,    53,     0,    41,     0,
       0,    42,    43,   122,    44,    45,    46,    47,    48,    49,
      50,    51,    52,     0,    53,     0,    41,     0,     0,    42,
      43,   133,    44,    45,    46,    47,    48,    49,    50,    51,
      52,     0,    53,     0,     0,    41,     0,   101,    42,    43,
       0,    44,    45,    46,    47,    48,    49,    50,    51,    52,
       0,    53,    41,    77,     0,    42,    43,     0,    44,    45,
      46,    47,    48,    49,    50,    51,    52,     0,    53,    41,
     110,     0,    42,    43,     0,    44,    45,    46,    47,    48,
      49,    50,    51,    52,     0,    53,    41,   137,     0,    42,
      43,     0,    44,    45,    46,    47,    48,    49,    50,    51,
      52,     0,    53,     0,   144,    -1,   114,     0,    42,    43,
       0,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      41,    53,     0,    42,    43,     0,    44,    45,    46,    47,
      48,    49,    50,    51,    52,     0,    53,    42,    43,     0,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    43,
      53,    44,    45,    46,    47,    48,    49,    50,    51,    52,
       0,    53,    44,    45,    46,    47,    48,    49,    50,    51,
      52,     0,    53,    46,    47,    48,    49,    50,    51,    52,
       0,    53,    47,    48,    49,    50,    51,    52,     0,    53
};

#define yypact_value_is_default(yystate) \
  ((yystate) == (-56))

#define yytable_value_is_error(yytable_value) \
  ((yytable_value) == (-1))

static const yytype_int16 yycheck[] =
{
      14,    30,    23,    58,    58,    58,    32,    21,    22,     4,
      24,     4,    38,     4,     4,    31,    37,    31,    32,    33,
       4,    11,    12,    39,    38,     0,     4,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    32,
      31,    96,    96,    96,    58,    36,    31,    34,    32,    35,
       4,    38,    73,    16,    39,     3,     4,     5,    39,     4,
      32,     9,     4,    92,    27,    28,    38,    30,    31,    31,
      33,    19,    27,   102,    36,    30,    31,    25,    33,    32,
      28,   110,    96,    31,    31,    38,    30,   116,    36,    39,
       4,   105,   106,   107,     4,    31,   125,    16,    39,    31,
     114,     3,     4,     5,    39,     4,    35,     9,    27,    39,
     139,    30,    31,    32,    33,    30,    31,    19,    33,     4,
       4,    10,   136,    25,    27,    28,    28,    30,    35,    31,
      32,     3,     4,     5,    38,     7,     8,     9,    94,    37,
      37,    37,     3,     4,     5,    -1,    29,    19,     9,    -1,
      11,    12,    -1,    25,    -1,    -1,    28,    -1,    19,    31,
       3,     4,     5,    -1,    25,    -1,     9,    28,    -1,    -1,
      31,    26,    27,    28,    -1,    30,    19,    -1,    -1,    -1,
      -1,    -1,    25,    14,    -1,    28,    17,    18,    31,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    -1,    30,
      -1,    -1,    14,    -1,    16,    17,    18,    38,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    -1,    30,    -1,
      14,    -1,    -1,    17,    18,    37,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    -1,    30,    -1,    14,    -1,
      -1,    17,    18,    37,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    -1,    30,    -1,    14,    -1,    -1,    17,
      18,    37,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    -1,    30,    -1,    -1,    14,    -1,    35,    17,    18,
      -1,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      -1,    30,    14,    32,    -1,    17,    18,    -1,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    -1,    30,    14,
      32,    -1,    17,    18,    -1,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    -1,    30,    14,    32,    -1,    17,
      18,    -1,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    -1,    30,    -1,    32,    14,    15,    -1,    17,    18,
      -1,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      14,    30,    -1,    17,    18,    -1,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    -1,    30,    17,    18,    -1,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    18,
      30,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      -1,    30,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    -1,    30,    22,    23,    24,    25,    26,    27,    28,
      -1,    30,    23,    24,    25,    26,    27,    28,    -1,    30
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    41,    42,     0,     4,    11,    12,    49,    50,    51,
       4,     4,     4,    35,    39,     4,     4,     3,     4,     5,
       9,    19,    25,    28,    31,    46,    47,    31,    39,    31,
      39,    31,    36,    31,    46,    46,    30,    31,    33,    48,
      46,    14,    17,    18,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    30,     4,    32,    52,    53,    36,    44,
      46,    32,    53,    44,    32,    45,    46,     7,     8,    46,
      46,    48,    45,    16,    27,    28,    48,    32,    46,    46,
      46,    46,    46,    46,    46,    46,    46,    46,    46,    46,
       4,     4,    39,    32,    38,     4,    43,    46,    49,    50,
      51,    35,    39,    32,    32,    38,    31,    31,    16,    37,
      32,    32,    34,    48,    15,    44,    39,    52,    46,    49,
      50,    51,    37,    35,    44,    39,    46,    46,    46,     4,
      44,    46,    44,    37,    35,    44,    38,    32,     4,    10,
      46,    37,    37,    44,    32,    37
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  However,
   YYFAIL appears to be in use.  Nevertheless, it is formally deprecated
   in Bison 2.4.2's NEWS entry, where a plan to phase it out is
   discussed.  */

#define YYFAIL		goto yyerrlab
#if defined YYFAIL
  /* This is here to suppress warnings from the GCC cpp's
     -Wunused-macros.  Normally we don't worry about that warning, but
     some users do, and we want to make it easy for users to remove
     YYFAIL uses, which will produce warnings from Bison 2.5.  */
#endif

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* This macro is provided for backward compatibility. */

#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
#else
static void
yy_stack_print (yybottom, yytop)
    yytype_int16 *yybottom;
    yytype_int16 *yytop;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yyrule)
    YYSTYPE *yyvsp;
    int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (0, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  YYSIZE_T yysize1;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = 0;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - Assume YYFAIL is not used.  It's too flawed to consider.  See
       <http://lists.gnu.org/archive/html/bison-patches/2009-12/msg00024.html>
       for details.  YYERROR is fine as it does not invoke this
       function.
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                yysize1 = yysize + yytnamerr (0, yytname[yyx]);
                if (! (yysize <= yysize1
                       && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                  return 2;
                yysize = yysize1;
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  yysize1 = yysize + yystrlen (yyformat);
  if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
    return 2;
  yysize = yysize1;

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  YYUSE (yyvaluep);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
	break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */
#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */


/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;


/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       `yyss': related to states.
       `yyvs': related to semantic values.

       Refer to the stacks thru separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yytoken = 0;
  yyss = yyssa;
  yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */
  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;

	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss_alloc, yyss);
	YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:

/* Line 1806 of yacc.c  */
#line 77 "grammar.y"
    {(yyval.decllist) = (yyvsp[(1) - (1)].decllist); func_list = (yyval.decllist);}
    break;

  case 3:

/* Line 1806 of yacc.c  */
#line 79 "grammar.y"
    {printf("delist\n"); (yyval.decllist) = new list<Decl*>();}
    break;

  case 4:

/* Line 1806 of yacc.c  */
#line 80 "grammar.y"
    {(yyval.decllist) = (yyvsp[(1) - (2)].decllist); (yyval.decllist)->push_back((yyvsp[(2) - (2)].fun)); printf("new func\n");}
    break;

  case 5:

/* Line 1806 of yacc.c  */
#line 81 "grammar.y"
    {(yyval.decllist) = (yyvsp[(1) - (2)].decllist); (yyval.decllist)->push_back((yyvsp[(2) - (2)].sfun)); printf("new func\n");}
    break;

  case 6:

/* Line 1806 of yacc.c  */
#line 82 "grammar.y"
    {(yyval.decllist) = (yyvsp[(1) - (3)].decllist); (yyval.decllist)->push_back((yyvsp[(2) - (3)].vardecl)); printf("new var\n");}
    break;

  case 7:

/* Line 1806 of yacc.c  */
#line 85 "grammar.y"
    {(yyval.decllist) = new list<Decl*>(); (yyval.decllist)->push_back((yyvsp[(1) - (1)].fun)); printf("new func\n");}
    break;

  case 8:

/* Line 1806 of yacc.c  */
#line 86 "grammar.y"
    {(yyval.decllist) = new list<Decl*>(); (yyval.decllist)->push_back((yyvsp[(1) - (1)].sfun)); printf("new func\n");}
    break;

  case 9:

/* Line 1806 of yacc.c  */
#line 87 "grammar.y"
    {(yyval.decllist) = new list<Decl*>(); (yyval.decllist)->push_back((yyvsp[(1) - (2)].vardecl)); printf("new var\n");}
    break;

  case 10:

/* Line 1806 of yacc.c  */
#line 88 "grammar.y"
    {(yyval.decllist) = (yyvsp[(1) - (2)].decllist); (yyval.decllist)->push_back((yyvsp[(2) - (2)].fun)); printf("new func\n");}
    break;

  case 11:

/* Line 1806 of yacc.c  */
#line 89 "grammar.y"
    {(yyval.decllist) = (yyvsp[(1) - (2)].decllist); (yyval.decllist)->push_back((yyvsp[(2) - (2)].sfun)); printf("new func\n");}
    break;

  case 12:

/* Line 1806 of yacc.c  */
#line 90 "grammar.y"
    {(yyval.decllist) = (yyvsp[(1) - (3)].decllist); (yyval.decllist)->push_back((yyvsp[(2) - (3)].vardecl)); printf("new var\n");}
    break;

  case 13:

/* Line 1806 of yacc.c  */
#line 93 "grammar.y"
    { printf("block\n"); (yyval.block) = new Block(NULL, (yyvsp[(1) - (2)].exp)); }
    break;

  case 14:

/* Line 1806 of yacc.c  */
#line 94 "grammar.y"
    { printf("block\n"); (yyval.block) = new Block(NULL, (yyvsp[(2) - (3)].exp)); }
    break;

  case 15:

/* Line 1806 of yacc.c  */
#line 95 "grammar.y"
    { printf("block: decl_list expr\n"); (yyval.block) = new Block((yyvsp[(2) - (4)].decllist), (yyvsp[(3) - (4)].exp)); }
    break;

  case 16:

/* Line 1806 of yacc.c  */
#line 98 "grammar.y"
    {printf("list\n"); (yyval.exprlist) = new list<Expr*>(); (yyval.exprlist)->push_back((yyvsp[(1) - (1)].exp));}
    break;

  case 17:

/* Line 1806 of yacc.c  */
#line 99 "grammar.y"
    {(yyval.exprlist) = (yyvsp[(1) - (3)].exprlist); (yyval.exprlist)->push_back((yyvsp[(3) - (3)].exp));}
    break;

  case 18:

/* Line 1806 of yacc.c  */
#line 102 "grammar.y"
    {(yyval.exp) = new ValExpr((yyvsp[(1) - (1)].num)); printf("new num\n");}
    break;

  case 19:

/* Line 1806 of yacc.c  */
#line 103 "grammar.y"
    {(yyval.exp) = new BoolExpr((yyvsp[(1) - (1)].boolean)); printf("new bool\n");}
    break;

  case 20:

/* Line 1806 of yacc.c  */
#line 104 "grammar.y"
    {(yyval.exp) = new IdExpr((yyvsp[(1) - (1)].id));}
    break;

  case 21:

/* Line 1806 of yacc.c  */
#line 105 "grammar.y"
    {(yyval.exp) = new FunCallExpr((yyvsp[(1) - (3)].id), NULL);}
    break;

  case 22:

/* Line 1806 of yacc.c  */
#line 106 "grammar.y"
    {(yyval.exp) = new FunCallExpr((yyvsp[(1) - (4)].id), (yyvsp[(3) - (4)].exprlist));}
    break;

  case 23:

/* Line 1806 of yacc.c  */
#line 107 "grammar.y"
    {(yyval.exp) = new MemExpr((yyvsp[(1) - (3)].exp), (yyvsp[(3) - (3)].id));}
    break;

  case 24:

/* Line 1806 of yacc.c  */
#line 108 "grammar.y"
    {printf("()\n"); (yyval.exp) = (yyvsp[(2) - (3)].exp);}
    break;

  case 25:

/* Line 1806 of yacc.c  */
#line 109 "grammar.y"
    {printf("reduce +\n"); (yyval.exp) = new PlusExpr((yyvsp[(1) - (3)].exp), (yyvsp[(3) - (3)].exp));}
    break;

  case 26:

/* Line 1806 of yacc.c  */
#line 110 "grammar.y"
    {(yyval.exp) = new MinusExpr((yyvsp[(1) - (3)].exp), (yyvsp[(3) - (3)].exp));}
    break;

  case 27:

/* Line 1806 of yacc.c  */
#line 111 "grammar.y"
    {(yyval.exp) = new TimesExpr((yyvsp[(1) - (3)].exp), (yyvsp[(3) - (3)].exp));}
    break;

  case 28:

/* Line 1806 of yacc.c  */
#line 112 "grammar.y"
    {(yyval.exp) = new DivideExpr((yyvsp[(1) - (3)].exp), (yyvsp[(3) - (3)].exp));}
    break;

  case 29:

/* Line 1806 of yacc.c  */
#line 113 "grammar.y"
    {(yyval.exp) = new GreaterExpr((yyvsp[(1) - (3)].exp), (yyvsp[(3) - (3)].exp));}
    break;

  case 30:

/* Line 1806 of yacc.c  */
#line 114 "grammar.y"
    {(yyval.exp) = new LessExpr((yyvsp[(1) - (3)].exp), (yyvsp[(3) - (3)].exp));}
    break;

  case 31:

/* Line 1806 of yacc.c  */
#line 115 "grammar.y"
    {printf("pipe\n"); (yyval.exp) = new PipeExpr((yyvsp[(1) - (3)].exp), (yyvsp[(3) - (3)].exp));}
    break;

  case 32:

/* Line 1806 of yacc.c  */
#line 116 "grammar.y"
    {(yyval.exp) = new AndExpr((yyvsp[(1) - (3)].exp), (yyvsp[(3) - (3)].exp));}
    break;

  case 33:

/* Line 1806 of yacc.c  */
#line 117 "grammar.y"
    {printf("OR\n");(yyval.exp) = new OrExpr((yyvsp[(1) - (3)].exp), (yyvsp[(3) - (3)].exp));}
    break;

  case 34:

/* Line 1806 of yacc.c  */
#line 118 "grammar.y"
    {(yyval.exp) = new EqualExpr((yyvsp[(1) - (3)].exp), (yyvsp[(3) - (3)].exp));}
    break;

  case 35:

/* Line 1806 of yacc.c  */
#line 119 "grammar.y"
    {(yyval.exp) = new NotEqualExpr((yyvsp[(1) - (3)].exp), (yyvsp[(3) - (3)].exp));}
    break;

  case 36:

/* Line 1806 of yacc.c  */
#line 120 "grammar.y"
    {(yyval.exp) = new UnaryMinusExpr((yyvsp[(2) - (2)].exp));}
    break;

  case 37:

/* Line 1806 of yacc.c  */
#line 121 "grammar.y"
    {(yyval.exp) = new NegateExpr((yyvsp[(2) - (2)].exp));}
    break;

  case 38:

/* Line 1806 of yacc.c  */
#line 123 "grammar.y"
    {
		printf("if\n"); (yyval.exp) = new IfExpr((yyvsp[(3) - (7)].exp), (yyvsp[(5) - (7)].block), (yyvsp[(7) - (7)].block));
	      }
    break;

  case 39:

/* Line 1806 of yacc.c  */
#line 127 "grammar.y"
    {
		(yyval.exp) = new IfExpr((yyvsp[(3) - (5)].exp), (yyvsp[(5) - (5)].block), NULL);
	      }
    break;

  case 40:

/* Line 1806 of yacc.c  */
#line 130 "grammar.y"
    {printf("Choice with 2 exprs\n"); (yyval.exp) = new ChoiceExpr((yyvsp[(1) - (3)].exp), (yyvsp[(3) - (3)].exp), NULL);}
    break;

  case 41:

/* Line 1806 of yacc.c  */
#line 131 "grammar.y"
    {printf("Choice re\n"); (yyval.exp) = new ChoiceExpr((yyvsp[(1) - (5)].exp), (yyvsp[(3) - (5)].exp), (yyvsp[(5) - (5)].exp));}
    break;

  case 42:

/* Line 1806 of yacc.c  */
#line 132 "grammar.y"
    {printf("re expr\n"); (yyval.exp) = (yyvsp[(2) - (3)].re); (yyval.exp)->setName(); re_list->push_back((yyvsp[(2) - (3)].re));}
    break;

  case 43:

/* Line 1806 of yacc.c  */
#line 133 "grammar.y"
    {(yyval.exp) = (yyvsp[(1) - (1)].exp);}
    break;

  case 44:

/* Line 1806 of yacc.c  */
#line 135 "grammar.y"
    {
		    printf("split func\n"); (yyval.exp) = new SplitExpr((yyvsp[(1) - (9)].id), (yyvsp[(5) - (9)].exp), (yyvsp[(7) - (9)].exp));
		}
    break;

  case 45:

/* Line 1806 of yacc.c  */
#line 139 "grammar.y"
    {
		    printf("iter func\n"); (yyval.exp) = new IterExpr((yyvsp[(1) - (7)].id), (yyvsp[(5) - (7)].exp));
		}
    break;

  case 46:

/* Line 1806 of yacc.c  */
#line 145 "grammar.y"
    {
		printf("new agg expr\n");
		(yyval.exp) = new AggExpr((yyvsp[(1) - (7)].id), (yyvsp[(3) - (7)].exp), (yyvsp[(6) - (7)].id), (yyvsp[(5) - (7)].id));
	      }
    break;

  case 47:

/* Line 1806 of yacc.c  */
#line 150 "grammar.y"
    {
		(yyval.exp) = new AggExpr((yyvsp[(1) - (4)].id), (yyvsp[(3) - (4)].exp), "", "");
	      }
    break;

  case 48:

/* Line 1806 of yacc.c  */
#line 155 "grammar.y"
    { printf("single re\n"); (yyval.re) = new SingleRE((yyvsp[(2) - (3)].exprlist)); pred_list->push_back((yyvsp[(2) - (3)].exprlist));}
    break;

  case 49:

/* Line 1806 of yacc.c  */
#line 156 "grammar.y"
    {printf("any\n"); (yyval.re) = new SingleRE(NULL);}
    break;

  case 50:

/* Line 1806 of yacc.c  */
#line 157 "grammar.y"
    {printf("concat\n"); (yyval.re) = new ConcatRE((yyvsp[(1) - (2)].re), (yyvsp[(2) - (2)].re));}
    break;

  case 51:

/* Line 1806 of yacc.c  */
#line 158 "grammar.y"
    {printf("star\n"); (yyval.re) = new StarRE((yyvsp[(1) - (2)].re));}
    break;

  case 52:

/* Line 1806 of yacc.c  */
#line 159 "grammar.y"
    {printf("union\n"); (yyval.re) = new UnionRE((yyvsp[(1) - (3)].re),(yyvsp[(3) - (3)].re));}
    break;

  case 53:

/* Line 1806 of yacc.c  */
#line 160 "grammar.y"
    {(yyval.re) = (yyvsp[(2) - (3)].re);}
    break;

  case 54:

/* Line 1806 of yacc.c  */
#line 165 "grammar.y"
    {
		printf("sfunc\n");
		(yyval.sfun) = new SFun((yyvsp[(2) - (8)].id), (yyvsp[(3) - (8)].id), (yyvsp[(5) - (8)].arglist), (yyvsp[(8) - (8)].block));
	      }
    break;

  case 55:

/* Line 1806 of yacc.c  */
#line 170 "grammar.y"
    {
		(yyval.sfun) = new SFun((yyvsp[(2) - (7)].id), (yyvsp[(3) - (7)].id), NULL, (yyvsp[(7) - (7)].block));
	      }
    break;

  case 56:

/* Line 1806 of yacc.c  */
#line 174 "grammar.y"
    {
		(yyval.sfun) = new SFun((yyvsp[(2) - (5)].id), (yyvsp[(3) - (5)].id), NULL, (yyvsp[(5) - (5)].block));
	      }
    break;

  case 57:

/* Line 1806 of yacc.c  */
#line 180 "grammar.y"
    {
		(yyval.fun) = new Fun((yyvsp[(2) - (8)].id), (yyvsp[(3) - (8)].id), (yyvsp[(5) - (8)].arglist), (yyvsp[(8) - (8)].block));
	      }
    break;

  case 58:

/* Line 1806 of yacc.c  */
#line 184 "grammar.y"
    {
		(yyval.fun) = new Fun((yyvsp[(2) - (7)].id), (yyvsp[(3) - (7)].id), NULL, (yyvsp[(7) - (7)].block));
	      }
    break;

  case 59:

/* Line 1806 of yacc.c  */
#line 188 "grammar.y"
    {
		(yyval.fun) = new Fun((yyvsp[(2) - (5)].id), (yyvsp[(3) - (5)].id), NULL, (yyvsp[(5) - (5)].block));
	      }
    break;

  case 60:

/* Line 1806 of yacc.c  */
#line 194 "grammar.y"
    {
		// should assignment be allowed?
		printf("reduce vardef\n");
		(yyval.vardecl) = new VarDecl((yyvsp[(1) - (4)].id), (yyvsp[(2) - (4)].id), (yyvsp[(4) - (4)].exp));
	      }
    break;

  case 61:

/* Line 1806 of yacc.c  */
#line 201 "grammar.y"
    {(yyval.arg) = new Arg((yyvsp[(1) - (2)].id),(yyvsp[(2) - (2)].id));}
    break;

  case 62:

/* Line 1806 of yacc.c  */
#line 204 "grammar.y"
    {printf("single arg\n"); (yyval.arglist) = new list<Arg*>(); (yyval.arglist)->push_back((yyvsp[(1) - (1)].arg));}
    break;

  case 63:

/* Line 1806 of yacc.c  */
#line 205 "grammar.y"
    {printf("arglist\n");(yyval.arglist) = (yyvsp[(1) - (3)].arglist); (yyval.arglist)->push_back((yyvsp[(3) - (3)].arg));}
    break;



/* Line 1806 of yacc.c  */
#line 2055 "y.tab.c"
      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &yylval);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;


      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  *++yyvsp = yylval;


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined(yyoverflow) || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}



/* Line 2067 of yacc.c  */
#line 209 "grammar.y"


void yyerror( char const * str ) {
    printf( "error: %s\n", str );
}


