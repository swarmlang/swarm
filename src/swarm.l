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

enumerate {
	return makeBareToken(TokenKind::ENUMERATE);
}

as {
	return makeBareToken(TokenKind::AS);
}

with {
	return makeBareToken(TokenKind::WITH);
}

"{" {
	return makeBareToken(TokenKind::LBRACE);
}

"}" {
	return makeBareToken(TokenKind::RBRACE);
}

"(" {
	return makeBareToken(TokenKind::LPAREN);
}

")" {
	return makeBareToken(TokenKind::RPAREN);
}

"<" {
	return makeBareToken(TokenKind::LARROW);
}

">" {
	return makeBareToken(TokenKind::RARROW);
}

"[" {
	return makeBareToken(TokenKind::LBRACKET);
}

"]" {
	return makeBareToken(TokenKind::RBRACKET);
}

";" {
	return makeBareToken(TokenKind::SEMICOLON);
}

"," {
	return makeBareToken(TokenKind::COMMA);
}

"=" {
	return makeBareToken(TokenKind::ASSIGN);
}

string {
	return makeBareToken(TokenKind::STRING);
}

number {
	return makeBareToken(TokenKind::NUMBER);
}

bool {
	return makeBareToken(TokenKind::BOOL);
}

[0-9]+([.][0-9]+)? {
	return makeBareToken(TokenKind::NUMBERLITERAL);
}

enumerable {
	return makeBareToken(TokenKind::ENUMERABLE);
}

map {
	return makeBareToken(TokenKind::MAP);
}

if {
	return makeBareToken(TokenKind::IF);
}

true {
	return makeBareToken(TokenKind::TRUE);
}

false {
	return makeBareToken(TokenKind::FALSE);
}

"&&" {
	return makeBareToken(TokenKind::AND);
}

"||" {
	return makeBareToken(TokenKind::OR);
}

"!" {
	return makeBareToken(TokenKind::NOT);
}

"==" {
	return makeBareToken(TokenKind::EQUAL);
}

"!=" {
	return makeBareToken(TokenKind::NOTEQUAL);
}

"+" {
	return makeBareToken(TokenKind::ADD);
}

"-" {
	return makeBareToken(TokenKind::SUBTRACT);
}

"*" {
	return makeBareToken(TokenKind::MULTIPLY);
}

"/" {
	return makeBareToken(TokenKind::DIVIDE);
}

"+=" {
	return makeBareToken(TokenKind::ADDASSIGN);
}

"*=" {
	return makeBareToken(TokenKind::MULTIPLYASSIGN);
}

"%" {
	return makeBareToken(TokenKind::MODULUS);
}

"^" {
	return makeBareToken(TokenKind::POWER);
}

[.] {
	return makeBareToken(TokenKind::CAT);
}

({LETTER})({LETTER}|{DIGIT})* {
	return makeIDToken();
}

(\"{STRELT}*\") {
	return makeBareToken(TokenKind::STRINGLITERAL);
}

\n|(\r\n) {
	lineNum++;
	colNum = 1;
}

[ \t]+ {
	colNum += yyleng;
}

. {
	Console::get()->error("Parse error, unexpected character");
	exit(1);
}
%%
