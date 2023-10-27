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
LETTER [a-zA-Z]
ESCAPEE [ntr'"\\]
STRELT (([\\]{ESCAPEE})|([^\\\n"]))
/*
WHITESPACE [\040\t]
BADESC [\\][^nt"\\\n]?
*/

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

of {
	return makeBareToken(TokenKind::OF);
}

with {
	return makeBareToken(TokenKind::WITH);
}

shared {
	return makeBareToken(TokenKind::SHARED);
}

continue {
	return makeBareToken(TokenKind::CONTINUE);
}

break {
	return makeBareToken(TokenKind::BREAK);
}

return {
	return makeBareToken(TokenKind::RETURN);
}

include {
	return makeBareToken(TokenKind::INCLUDE);
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

"<=" {
	return makeBareToken(TokenKind::LARROWEQUALS);
}

">=" {
	return makeBareToken(TokenKind::RARROWEQUALS);
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

":" {
	return makeBareToken(TokenKind::COLON);
}

"," {
	return makeBareToken(TokenKind::COMMA);
}

"=" {
	return makeBareToken(TokenKind::ASSIGN);
}

"->" {
	return makeBareToken(TokenKind::ARROW);
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

void {
	return makeBareToken(TokenKind::VOID);
}

type {
	return makeBareToken(TokenKind::TYPE);
}

[0-9]+([.][0-9]+)? {
	return makeNumberLiteralToken();
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

while {
	return makeBareToken(TokenKind::WHILE);
}

true {
	return makeBareToken(TokenKind::TTRUE);
}

false {
	return makeBareToken(TokenKind::TFALSE);
}

fn {
	return makeBareToken(TokenKind::FN);
}

constructor {
	return makeBareToken(TokenKind::CONSTRUCTOR);
}

from {
	return makeBareToken(TokenKind::FROM);
}

"=>" {
	return makeBareToken(TokenKind::FNDEF);
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

"-=" {
	return makeBareToken(TokenKind::SUBTRACTASSIGN);
}

"/=" {
	return makeBareToken(TokenKind::DIVIDEASSIGN);
}

"%=" {
	return makeBareToken(TokenKind::MODULUSASSIGN);
}

"^=" {
	return makeBareToken(TokenKind::POWERASSIGN);
}

"&&=" {
	return makeBareToken(TokenKind::ANDASSIGN);
}

"||=" {
	return makeBareToken(TokenKind::ORASSIGN);
}

"%" {
	return makeBareToken(TokenKind::MODULUS);
}

"^" {
	return makeBareToken(TokenKind::POWER);
}

"_/" {
	return makeBareToken(TokenKind::SQRT);
}

"_" {
	return makeBareToken(TokenKind::WILDCARD);
}

\. {
	return makeBareToken(TokenKind::DOT);
}

({LETTER}({LETTER}|{DIGIT}|_)*)|(_*({LETTER}|{DIGIT})({LETTER}|{DIGIT}|_)*) {
	return makeIDToken();
}

(\"{STRELT}*\") {
	return makeStringLiteralToken();
}

\n|(\r\n) {
	lineNum++;
	colNum = 1;
}

[ \t]+ {
	colNum += yyleng;
}

--.*\n {
	lineNum++;
}

--\*([^*]|(\*[^-])|(\*-[^-]))*\*-- {
	std::string s(yytext);
	for (char c : s) {
		if (c == '\n') {
			lineNum++;
		}
	}
}

. {
	std::string s(yytext), pos = "[" + std::to_string(lineNum) + ", " + std::to_string(colNum) + "]";
	nslib::Fatal::error("Lexing error, unexpected character " + s + " at position " + pos);
}
%%
