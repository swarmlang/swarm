%{
#include <string>
#include <limits.h>

/* Get our custom yyFlexScanner subclass */
#include "lang/Scanner.h"
#include "bison/grammar.hh"

#undef YY_DECL
#define YY_DECL int swarmc::Lang::Scanner::yylex(swarmc::Lang::Parser::semantic_type* const lval)

using TokenKind = swarmc::Lang::Parser::token;

/* define yyterminate as returning an EOF token (instead of NULL) */
#define yyterminate() return ( TokenKind::END )

/* exclude unistd.h for Visual Studio compatibility. */
#define YY_NO_UNISTD_H

#define EXIT_ON_ERR 1


%}

%option nodefault
%option yyclass="swarm::Scanner"
%option noyywrap
%option c++

DIGIT [0-9]
WHITESPACE [\040\t]
LETTER [a-zA-Z]
ESCAPEE [nt'"\\]

 /* */ 
BADESC [\\][^nt"\\\n]?
STRELT (([\\][nt"\\])|([^\\\n"]))

%%
%{
	this->yylval = lval;
%}

%%
