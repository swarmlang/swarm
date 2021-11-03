%skeleton "lalr1.cc"
%require "3.0"
%debug
%defines
%define api.namespace{swarmc::Lang}
%define api.parser.class {Parser}
%define parse.error verbose
%output "parser.cc"
%token-table

%code requires{
	#include <list>
    #include "../shared/util/Console.h"
	#include "../lang/Token.h"
   	#include "../lang/AST.h"
	namespace swarmc::Lang {
		class Scanner;
	}

# ifndef YY_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULLPTR nullptr
#  else
#   define YY_NULLPTR 0
#  endif
# endif
}

%parse-param { swarmc::Lang::Scanner &scanner }
%parse-param { swarmc::Lang::ProgramNode** root }
%code{
    #include <iostream>
    #include <cstdlib>
    #include <fstream>

    #include "../shared/util/Console.h"
    #include "../lang/Scanner.h"
    #include "../lang/Token.h"
    #undef yylex
    #define yylex scanner.yylex
}

%union {
    swarmc::Lang::ASTNode*              transPlaceholder;
    swarmc::Lang::Token*                transToken;
    swarmc::Lang::Token*                lexeme;
    swarmc::Lang::ProgramNode*          transProgram;
    swarmc::Lang::IDToken*              transIDToken;
    swarmc::Lang::StatementNode*        transStatement;
    swarmc::Lang::IdentifierNode*       transID;
    swarmc::Lang::ExpressionNode*       transExpression;
    swarmc::Lang::CallExpressionNode*   transCallExpression;
    std::vector<swarmc::Lang::ExpressionNode*>* transExpressions;
}

%define parse.assert

%token                   END	    0 "end file"
%token <transToken>      ENUMERATE
%token <transIDToken>    ID
%token <transToken>      AS
%token <transToken>      WITH
%token <transToken>      LBRACE
%token <transToken>      RBRACE
%token <transToken>      LPAREN
%token <transToken>      RPAREN
%token <transToken>      LBRACKET
%token <transToken>      RBRACKET
%token <transToken>      LARROW
%token <transToken>      RARROW
%token <transToken>      SEMICOLON
%token <transToken>      COMMA
%token <transToken>      ASSIGN
%token <transToken>      STRING
%token <transToken>      NUMBER
%token <transToken>      BOOL
%token <transToken>      NUMBERLITERAL
%token <transToken>      STRINGLITERAL
%token <transToken>      ENUMERABLE
%token <transToken>      MAP
%token <transToken>      IF
%token <transToken>      TRUE
%token <transToken>      FALSE
%token <transToken>      AND
%token <transToken>      OR
%token <transToken>      NOT
%token <transToken>      EQUAL
%token <transToken>      NOTEQUAL
%token <transToken>      ADD
%token <transToken>      SUBTRACT
%token <transToken>      MULTIPLY
%token <transToken>      DIVIDE
%token <transToken>      ADDASSIGN
%token <transToken>      MULTIPLYASSIGN
%token <transToken>      MODULUS
%token <transToken>      POWER
%token <transToken>      CAT

/*    (attribute type)      (nonterminal)    */
%type <transProgram>        program
%type <transProgram>        statements
%type <transStatement>      statement
%type <transID>             id
%type <transExpression>     expression
%type <transCallExpression> callExpression
%type <transExpressions>    actuals

%%

program :
    statements {
        $$ = $1;
    }



statements :
    statements statement {
        $1->pushStatement($2);
        $$ = $1;
    }

    | %empty {
        $$ = new ProgramNode();
    }



statement :
    ENUMERATE id AS id LBRACE statements RBRACE {
        Position* pos = new Position($1->position(), $7->position());
        EnumerationStatement* e = new EnumerationStatement(pos, $2, $4);
        e->assumeAndReduceStatements($6->reduceToStatements());
        $$ = e;
    }

    | WITH expression AS id LBRACE statements RBRACE {
        Position* pos = new Position($1->position(), $7->position());
        WithStatement* w = new WithStatement(pos, $2, $4);
        w->assumeAndReduceStatements($6->reduceToStatements());
        $$ = w;
    }

    | callExpression SEMICOLON {
        Position* pos = new Position($1->position(), $2->position());
        $$ = new ExpressionStatementNode(pos, $1);
    }



id :
    ID {
        $$ = new IdentifierNode($1->position(), $1->identifier());
    }



expression :
    callExpression {
        $$ = $1;
    }

    | id {
        $$ = $1;
    }



actuals :
    expression {
        std::vector<ExpressionNode*>* actuals = new std::vector<ExpressionNode*>();
        actuals->push_back($1);
        $$ = actuals;
    }

    | actuals COMMA expression {
        $$ = $1;
        $$->push_back($3);
    }



callExpression :
    id LPAREN RPAREN {
        Position* pos = new Position($1->position(), $3->position());
        $$ = new CallExpressionNode(pos, $1, new std::vector<ExpressionNode*>());
    }

    | id LPAREN actuals RPAREN {
        Position* pos = new Position($1->position(), $4->position());
        $$ = new CallExpressionNode(pos, $1, $3);
    }

%%

void swarmc::Lang::Parser::error(const std::string& msg){
    Console* console = Console::get();
    console->error(msg);
}
