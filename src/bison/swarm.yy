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
    #include "../Reporting.h"
    #include "../errors/ParseError.h"
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
    swarmc::Lang::StringLiteralToken*   transStringToken;
    swarmc::Lang::NumberLiteralToken*   transNumberToken;
    swarmc::Lang::StatementNode*        transStatement;
    swarmc::Lang::IdentifierNode*       transID;
    swarmc::Lang::LValNode*             transLVal;
    swarmc::Lang::ExpressionNode*       transExpression;
    swarmc::Lang::AssignExpressionNode* transAssignExpression;
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
%token <transToken>      OF
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
%token <transNumberToken> NUMBERLITERAL
%token <transStringToken> STRINGLITERAL
%token <transToken>      ENUMERABLE
%token <transToken>      MAP
%token <transToken>      IF
%token <transToken>      WHILE
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
%type <transLVal>           lval
%type <transExpression>     expression
%type <transExpression>     term
%type <transAssignExpression> assignment
%type <transCallExpression> callExpression
%type <transExpressions>    actuals
%type <transType>           type
%type <transDeclaration>    declaration
%type <transMapStatement>   mapStatement
%type <transMapStatements>  mapStatements

%left CAT
%left OR
%left AND
%nonassoc EQUAL NOTEQUAL
%left SUBTRACT ADD
%left MULTIPLY DIVIDE MODULUS
%left POWER

%%

program :
    statements {
        $$ = $1;
        *root = $$;
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

    | IF LPAREN expression RPAREN LBRACE statements RBRACE {
        Position* pos = new Position($1->position(), $7->position());
        IfStatement* i = new IfStatement(pos, $3);
        i->assumeAndReduceStatements($6->reduceToStatements());
        $$ = i;
    } 

    | WHILE LPAREN expression RPAREN LBRACE statements RBRACE {
        Position* pos = new Position($1->position(), $7->position());
        WhileStatement* w = new WhileStatement(pos, $3);
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

    | assignment SEMICOLON {
        Position* pos = new Position($1->position(), $2->position());
        $$ = new ExpressionStatementNode(pos, $1);
    }



declaration :
    type id ASSIGN expression {
        Position* pos = new Position($1->position(), $4->position());
        VariableDeclarationNode* var = new VariableDeclarationNode(pos, $1, $2, $4);
        $$ = var;
    }



assignment :
    lval ASSIGN expression {
        Position* pos = new Position($1->position(), $3->position());
        $$ = new AssignExpressionNode(pos, $1, $3);
    }

    | lval ADDASSIGN expression {
        Position* pos = new Position($1->position(), $3->position());
        $$ = new AddAssignExpressionNode(pos, $1, $3);
    }

    | lval MULTIPLYASSIGN expression {
        Position* pos = new Position($1->position(), $3->position());
        $$ = new MultiplyAssignExpressionNode(pos, $1, $3);
    }


lval :
    id {
        $$ = $1;
    }

    | lval LBRACKET id RBRACKET {
        Position* pos = new Position($1->position(), $4->position());
        $$ = new MapAccessNode(pos, $1, $3);
    }

    | lval LBRACKET NUMBERLITERAL RBRACKET {
        Position* pos = new Position($1->position(), $4->position());
        if ($3->value() != (int)$3->value()) {
            Reporting::parseError(
                $3->position(),
                "Invalid Enumerable index: " + std::to_string($3->value()));
            throw swarmc::Errors::ParseError(1);
        }
        auto index = new IntegerLiteralExpressionNode($3->position(), (int)$3->value());
        $$ = new EnumerableAccessNode(pos, $1, index);
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

    | LBRACKET RBRACKET OF type {
        Position* pos = new Position($1->position(), $4->position());
        std::vector<ExpressionNode*>* actuals = new std::vector<ExpressionNode*>();
        $$ = new EnumerationLiteralExpressionNode(pos, actuals, $4);
    }

    | LBRACKET actuals RBRACKET {
        Position* pos = new Position($1->position(), $3->position());
        $$ = new EnumerationLiteralExpressionNode(pos, $2);
    }

    | LBRACE RBRACE OF type {
        Position* pos = new Position($1->position(), $4->position());
        std::vector<MapStatementNode*>* body = new std::vector<MapStatementNode*>();
        $$ = new MapNode(pos, body, $4);
    }

    | LBRACE mapStatements RBRACE {
        Position* pos = new Position($1->position(), $3->position());
        $$ = new MapNode(pos, $2);
    }

    | expression EQUAL expression {
        Position* pos = new Position($1->position(), $3->position());
        $$ = new EqualsNode(pos, $1, $3);
    }

    | expression NOTEQUAL expression {
        Position* pos = new Position($1->position(), $3->position());
        $$ = new NotEqualsNode(pos, $1, $3);
    }

    | expression ADD expression {
        Position* pos = new Position($1->position(), $3->position());
        $$ = new AddNode(pos, $1, $3);
    }

    | expression SUBTRACT expression {
        Position* pos = new Position($1->position(), $3->position());
        $$ = new SubtractNode(pos, $1, $3);
    }

    | expression MULTIPLY expression {
        Position* pos = new Position($1->position(), $3->position());
        $$ = new MultiplyNode(pos, $1, $3);
    }

    | expression DIVIDE expression {
        Position* pos = new Position($1->position(), $3->position());
        $$ = new DivideNode(pos, $1, $3);
    }

    | expression MODULUS expression {
        Position* pos = new Position($1->position(), $3->position());
        $$ = new ModulusNode(pos, $1, $3);
    }

    | expression POWER expression {
        Position* pos = new Position($1->position(), $3->position());
        $$ = new PowerNode(pos, $1, $3);
    }

    | expression CAT expression {
        Position* pos = new Position($1->position(), $3->position());
        $$ = new ConcatenateNode(pos, $1, $3);
    }

    | expression OR expression {
        Position* pos = new Position($1->position(), $3->position());
        $$ = new OrNode(pos, $1, $3);
    }

    | expression AND expression {
        Position* pos = new Position($1->position(), $3->position());
        $$ = new AndNode(pos, $1, $3);
    }

    | SUBTRACT term {
        Position* pos = new Position($1->position(), $2->position());
        $$ = new NegativeExpressionNode(pos,$2);
    }

    | NOT term {
        Position* pos = new Position($1->position(), $2->position());
        $$ = new NotNode(pos, $2);
    }



term :
    callExpression {
        $$ = $1;
    }

    | lval {
        $$ = $1;
    }

    | STRINGLITERAL {
        $$ = new StringLiteralExpressionNode($1->position(), $1->value());
    }

    | NUMBERLITERAL {
        $$ = new NumberLiteralExpressionNode($1->position(), $1->value());
    }

    | TRUE {
        $$ = new BooleanLiteralExpressionNode($1->position(), true);
    }

    | FALSE {
        $$ = new BooleanLiteralExpressionNode($1->position(), false);
    }

    | LPAREN expression RPAREN {
        $$ = $2;
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

%%

void swarmc::Lang::Parser::error(const std::string& msg){
    Console* console = Console::get();
    console->error(msg);
}
