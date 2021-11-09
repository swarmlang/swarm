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
   	#include "../lang/Type.h"
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
    swarmc::Lang::TypeNode*             transType;
    swarmc::Lang::DeclarationNode*      transDeclaration;
    swarmc::Lang::MapStatementNode*     transMapStatement;
    std::vector<MapStatementNode*>*     transMapStatements;
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
%token <transToken>      COLON
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
%type <transExpression>     term
%type <transCallExpression> callExpression
%type <transExpressions>    actuals
%type <transType>           type
%type <transDeclaration>    declaration
%type <transMapStatement>   mapStatement
%type <transMapStatements>  mapStatements

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

    | WITH term AS id LBRACE statements RBRACE {
        Position* pos = new Position($1->position(), $7->position());
        WithStatement* w = new WithStatement(pos, $2, $4);
        w->assumeAndReduceStatements($6->reduceToStatements());
        $$ = w;
    }

    | callExpression SEMICOLON {
        Position* pos = new Position($1->position(), $2->position());
        $$ = new ExpressionStatementNode(pos, $1);
    }

    | declaration SEMICOLON {
        $$ = $1;
    }

    | id ADDASSIGN SEMICOLON {
        $$ = $1;
    }

    | id MULTIPLYASSIGN SEMICOLON {
        $$ = $1;
    }



declaration :
    type id ASSIGN expression {
        Position* pos = new Position($1->position(), $4->position());
        VariableDeclarationNode* var = new VariableDeclarationNode(pos, $1, $2, $4);
        $$ = var;
    }



id :
    ID {
        $$ = new IdentifierNode($1->position(), $1->identifier());
    }



type :
    STRING {
        PrimitiveType* t = PrimitiveType::of(ValueType::TSTRING);
        $$ = new PrimitiveTypeNode($1->position(), t);
    }

    | NUMBER {
        PrimitiveType* t = PrimitiveType::of(ValueType::TNUM);
        $$ = new PrimitiveTypeNode($1->position(), t);
    }

    | BOOL {
        PrimitiveType* t = PrimitiveType::of(ValueType::TBOOL);
        $$ = new PrimitiveTypeNode($1->position(), t);
    }

    | NUMBERLITERAL {
        PrimitiveType* t = PrimitiveType::of(ValueType::TNUMBERLITERAL);
        $$ = new PrimitiveTypeNode($1->position(), t);
    }

    | STRINGLITERAL {
        PrimitiveType* t = PrimitiveType::of(ValueType::TSTRINGLITERAL);
        $$ = new PrimitiveTypeNode($1->position(), t);
    }

    | ENUMERABLE LARROW type RARROW {
        Position* pos = new Position($1->position(), $4->position());
        $$ = new EnumerableTypeNode(pos, $3);
    }

    | MAP LARROW type RARROW {
        Position* pos = new Position($1->position(), $4->position());
        $$ = new MapTypeNode(pos, $3);
    }



expression :
    term {
        $$ = $1;
    }

<<<<<<< HEAD
    | type operation type { # not sure if this goes here
        # should filter only number and string types?
        $$ = $1;
=======
    | LBRACKET RBRACKET {
        Position* pos = new Position($1->position(), $2->position());
        std::vector<ExpressionNode*>* actuals = new std::vector<ExpressionNode*>();
        $$ = new EnumerationLiteralExpressionNode(pos, actuals);
    }

    | LBRACKET actuals RBRACKET {
        Position* pos = new Position($1->position(), $3->position());
        $$ = new EnumerationLiteralExpressionNode(pos, $2);
    }

    | LBRACE RBRACE {
        Position* pos = new Position($1->position(), $2->position());
        std::vector<MapStatementNode*>* body = new std::vector<MapStatementNode*>();
        $$ = new MapNode(pos, body);
    }

    | LBRACE mapStatements RBRACE {
        Position* pos = new Position($1->position(), $3->position());
        $$ = new MapNode(pos, $2);
>>>>>>> f62a614347bc94973933f369129fed9723ace32f
    }



term :
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



mapStatements :
    mapStatement {
        std::vector<MapStatementNode*>* stmts = new std::vector<MapStatementNode*>();
        stmts->push_back($1);
        $$ = stmts;
    }

    | mapStatements COMMA mapStatement {
        $1->push_back($3);
        $$ = $1;
    }



mapStatement :
    id COLON expression {
        Position* pos = new Position($1->position(), $3->position());
        $$ = new MapStatementNode(pos, $1, $3);
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

operation :
    EQUAL {
        $$ = $1; # tbf
    }

    | NOTEQUAL {
        $$ = $1; # tbf
    }

    | ADD {
        $$ = $1; # tbf
    }
    
    | SUBTRACT {
        $$ = $1; # tbf
    }

    | MULTIPLY {
        $$ = $1; # tbf
    }

    | DIVIDE {
        $$ = $1; # tbf
    }

    | MODULUS {
        $$ = $1; # tbf
    }

    | POWER {
        $$ = $1; # tbf
    }

    | CAT 
        $$ = $1; # tbf
    }

%%

void swarmc::Lang::Parser::error(const std::string& msg){
    Console* console = Console::get();
    console->error(msg);
}
