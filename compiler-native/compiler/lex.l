/*
inport	return printf("Key word: %s\n", yytext);
srcmac	printf("Key word: %s\n", yytext);
dstmac	printf("Key word: %s\n", yytext);
srcip	printf("Key word: %s\n", yytext);
dstip	printf("Key word: %s\n", yytext);
time	printf("Key word: %s\n", yytext);


ip	yylval.num = 1; return TYPE;
mac	return TYPE;
int	printf("int type\n"); return TYPE;
bool	yylval.num = 1;	return TYPE;
real	return TYPE;

packet 	return TYPE;
action 	yylval.num = 1; return TYPE;


*/
%{
#include "ast.h"
#include <math.h>
#include "y.tab.h"
extern "C" int yylex();
%}

DIGIT	[0-9]
LETTER	[a-zA-Z]

%%
\/\/.*\n	;

true	yylval.boolean = true; return BOOL;
false	yylval.boolean = false; return BOOL;

>>	return PIPE;

==	return EQUAL;
!=	return NOTEQUAL;
&&	return AND;
\|\|	return OR;

sfun	printf("sfun key\n"); return SFUNKEY;
fun	return FUNKEY;

if	printf("IF\n"); return IF;
else	return ELSE;

split	printf("SPLIT\n"); return SPLIT;
iter	printf("ITER\n"); return ITER;

concat	printf("Key word: %s\n", yytext);

[_a-zA-Z]+[_a-zA-Z0-9]*	    yylval.id = (strdup(yytext)); printf("ID: %s\n", yytext); return ID;
{DIGIT}+    yylval.num = atof(yytext); printf("An integer: %s (%d)\n", yytext, atoi(yytext)); return NUM;

[ \t\f\r\n]   ; // ignore white space

.   return yytext[0];

<<EOF>>	yyterminate();
%%

