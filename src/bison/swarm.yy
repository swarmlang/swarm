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
    #include <float.h>
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
    swarmc::Lang::TypeLiteral*          transType;
    swarmc::Lang::DeclarationNode*      transDeclaration;
    swarmc::Lang::MapStatementNode*     transMapStatement;
    swarmc::Lang::FunctionNode*         transFunction;
    std::vector<std::pair<TypeLiteral*, IdentifierNode*>>*  transFormals;
    std::vector<swarmc::Lang::MapStatementNode*>* transMapStatements;
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
%token <transToken>      LARROWEQUALS
%token <transToken>      RARROWEQUALS
%token <transToken>      SEMICOLON
%token <transToken>      COLON
%token <transToken>      COMMA
%token <transToken>      ASSIGN
%token <transToken>      STRING
%token <transToken>      NUMBER
%token <transToken>      BOOL
%token <transToken>      VOID
%token <transNumberToken> NUMBERLITERAL
%token <transStringToken> STRINGLITERAL
%token <transToken>      ENUMERABLE
%token <transToken>      MAP
%token <transToken>      IF
%token <transToken>      WHILE
%token <transToken>      TRUE
%token <transToken>      FALSE
%token <transToken>      CONTINUE
%token <transToken>      BREAK
%token <transToken>      RETURN
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
%token <transToken>      SUBTRACTASSIGN
%token <transToken>      DIVIDEASSIGN
%token <transToken>      MODULUSASSIGN
%token <transToken>      POWERASSIGN
%token <transToken>      CATASSIGN
%token <transToken>      ANDASSIGN
%token <transToken>      ORASSIGN
%token <transToken>      MODULUS
%token <transToken>      POWER
%token <transToken>      CAT
%token <transToken>      SHARED
%token <transToken>      FN
%token <transToken>      ARROW
%token <transToken>      FNDEF

/*    (attribute type)      (nonterminal)    */
%type <transProgram>        program
%type <transProgram>        statements
%type <transStatement>      statement
%type <transID>             id
%type <transLVal>           lval
%type <transExpression>     expression
%type <transExpression>     expressionF
%type <transExpression>     term
%type <transAssignExpression> assignment
%type <transCallExpression> callExpression
%type <transExpressions>    actuals
%type <transFormals>        formals
%type <transFunction>       function
%type <transType>           type
%type <transDeclaration>    declaration
%type <transMapStatement>   mapStatement
%type <transMapStatements>  mapStatements

%precedence FNDEF
%left CAT
%left OR
%left AND
%nonassoc EQUAL NOTEQUAL LARROW LARROWEQUALS RARROW RARROWEQUALS
%left SUBTRACT ADD
%left MULTIPLY DIVIDE MODULUS
%left POWER
%right ARROW

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
        EnumerationStatement* e = new EnumerationStatement(pos, $2, $4, false);
        e->assumeAndReduceStatements($6->reduceToStatements());
        $$ = e;
    }

    | ENUMERATE id AS SHARED id LBRACE statements RBRACE {
        Position* pos = new Position($1->position(), $8->position());
        EnumerationStatement* e = new EnumerationStatement(pos, $2, $5, true);
        e->assumeAndReduceStatements($7->reduceToStatements());
        $$ = e;
    }

    | WITH term AS id LBRACE statements RBRACE {
        Position* pos = new Position($1->position(), $7->position());
        WithStatement* w = new WithStatement(pos, $2, $4, false);
        w->assumeAndReduceStatements($6->reduceToStatements());
        $$ = w;
    }

    | WITH term AS SHARED id LBRACE statements RBRACE {
        Position* pos = new Position($1->position(), $8->position());
        WithStatement* w = new WithStatement(pos, $2, $5, true);
        w->assumeAndReduceStatements($7->reduceToStatements());
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

    | RETURN SEMICOLON {
        Position* pos = new Position($1->position(), $2->position());
        $$ = new ReturnStatementNode(pos, nullptr);
    }

    | RETURN expression SEMICOLON {
        Position* pos = new Position($1->position(), $3->position());
        $$ = new ReturnStatementNode(pos, $2);
    }



declaration :
    type id ASSIGN expression {
        Position* pos = new Position($1->position(), $4->position());
        VariableDeclarationNode* var = new VariableDeclarationNode(pos, $1, $2, $4);
        $$ = var;
    }

    | SHARED type id ASSIGN expression {
        Position* pos = new Position($1->position(), $5->position());
        $2->setShared(true);
        VariableDeclarationNode* var = new VariableDeclarationNode(pos, $2, $3, $5);
        $$ = var;
    }

    | FN id ASSIGN function {
        Position* pos = new Position($1->position(), $4->position());

        $$ = new VariableDeclarationNode(pos, $4->typeNode(), $2, $4);
    }



assignment :
    lval ASSIGN expression {
        Position* pos = new Position($1->position(), $3->position());
        $$ = new AssignExpressionNode(pos, $1, $3);
    }

    | lval ADDASSIGN expression {
        Position* pos = new Position($1->position(), $3->position());
        auto r = new AddNode(pos, $1, $3);
        $$ = new AssignExpressionNode(pos, $1, r);
    }

    | lval MULTIPLYASSIGN expression {
        Position* pos = new Position($1->position(), $3->position());
        auto r = new MultiplyNode(pos, $1, $3);
        $$ = new AssignExpressionNode(pos, $1, r);
    }

    | lval SUBTRACTASSIGN expression {
        Position* pos = new Position($1->position(), $3->position());
        auto r = new SubtractNode(pos, $1, $3);
        $$ = new AssignExpressionNode(pos, $1, r);
    }

    | lval DIVIDEASSIGN expression {
        Position* pos = new Position($1->position(), $3->position());
        auto r = new DivideNode(pos, $1, $3);
        $$ = new AssignExpressionNode(pos, $1, r);
    }

    | lval MODULUSASSIGN expression {
        Position* pos = new Position($1->position(), $3->position());
        auto r = new ModulusNode(pos, $1, $3);
        $$ = new AssignExpressionNode(pos, $1, r);
    }

    | lval POWERASSIGN expression {
        Position* pos = new Position($1->position(), $3->position());
        auto r = new PowerNode(pos, $1, $3);
        $$ = new AssignExpressionNode(pos, $1, r);
    }

    | lval CATASSIGN expression {
        Position* pos = new Position($1->position(), $3->position());
        auto r = new ConcatenateNode(pos, $1, $3);
        $$ = new AssignExpressionNode(pos, $1, r);
    }

    | lval ANDASSIGN expression {
        Position* pos = new Position($1->position(), $3->position());
        auto r = new AndNode(pos, $1, $3);
        $$ = new AssignExpressionNode(pos, $1, r);
    }

    | lval ORASSIGN expression {
        Position* pos = new Position($1->position(), $3->position());
        auto r = new OrNode(pos, $1, $3);
        $$ = new AssignExpressionNode(pos, $1, r);
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
        if ($3->value() != (int)$3->value() || $3->value() < 0 || $3->value() > DBL_MAX) {
            Reporting::parseError(
                $3->position(),
                "Invalid Enumerable index: " + std::to_string($3->value()));
            throw swarmc::Errors::ParseError(1);
        }
        auto index = new IntegerLiteralExpressionNode($3->position(), (size_t)$3->value());
        $$ = new EnumerableAccessNode(pos, $1, index);
    }

id :
    ID {
        $$ = new IdentifierNode($1->position(), $1->identifier());
    }



type :
    STRING {
        auto t = Type::Primitive::of(Type::Intrinsic::STRING);
        $$ = new TypeLiteral($1->position(), t);
    }

    | NUMBER {
        auto t = Type::Primitive::of(Type::Intrinsic::NUMBER);
        $$ = new TypeLiteral($1->position(), t);
    }

    | BOOL {
        auto t = Type::Primitive::of(Type::Intrinsic::BOOLEAN);
        $$ = new TypeLiteral($1->position(), t);
    }

    | VOID {
        auto t = Type::Primitive::of(Type::Intrinsic::VOID);
        $$ = new TypeLiteral($1->position(), t);
    }

    | ENUMERABLE LARROW type RARROW {
        Position* pos = new Position($1->position(), $4->position());
        $$ = new TypeLiteral(pos, $3->value());
    }

    | MAP LARROW type RARROW {
        Position* pos = new Position($1->position(), $4->position());
        $$ = new TypeLiteral(pos, $3->value());
    }

    | type ARROW type {
        // might not want this because it allows for using these kind of types with
        // regular variable declarations (although maybe thats desired?)
        
        Position* pos = new Position($1->position(), $3->position());
        $$ = new TypeLiteral(pos, new Type::Lambda1($1->value(), $3->value()));
    }


function :
    LPAREN formals RPAREN COLON type FNDEF LBRACE statements RBRACE {
        Position* pos = new Position($1->position(), $9->position());
        Position* typepos = new Position($1->position(), $5->position());

        Type::Type* t = $5->value();

        for ( auto i = $2->rbegin(); i != $2->rend(); ++i ) {
            t = new Type::Lambda1((*i).first->value(), t);
        }

        FunctionNode* fn = new FunctionNode(pos, new TypeLiteral(typepos, t), $2);
        fn->assumeAndReduceStatements($8->reduceToStatements());
        $$ = fn;
    }

    | LPAREN RPAREN COLON type FNDEF LBRACE statements RBRACE {
        Position* pos = new Position($1->position(), $8->position());

        FunctionNode* fn = new FunctionNode(
            pos, new TypeLiteral($4->position(), new Type::Lambda0($4->value())), new FormalList());
        fn->assumeAndReduceStatements($7->reduceToStatements());
        $$ = fn;
    }

    | LPAREN formals RPAREN COLON type FNDEF expressionF {
        Position* pos = new Position($1->position(), $7->position());
        Position* typepos = new Position($1->position(), $5->position());

        Type::Type* t = $5->value();

        for ( auto i = $2->rbegin(); i != $2->rend(); ++i ) {
            t = new Type::Lambda1((*i).first->value(), t);
        }

        FunctionNode* fn = new FunctionNode(pos, new TypeLiteral(typepos, t), $2);
        auto retstmt = new StatementList();
        retstmt->push_back(new ReturnStatementNode($7->position(), $7));
        fn->assumeAndReduceStatements(retstmt);
        $$ = fn;
    }

    | LPAREN RPAREN COLON type FNDEF expressionF {
        Position* pos = new Position($1->position(), $6->position());

        FunctionNode* fn = new FunctionNode(
            pos, new TypeLiteral($4->position(), new Type::Lambda0($4->value())), new FormalList());
        auto retstmt = new StatementList();
        retstmt->push_back(new ReturnStatementNode($6->position(), $6));
        fn->assumeAndReduceStatements(retstmt);
        $$ = fn;
    }



expression :
    LBRACE RBRACE OF type {
        Position* pos = new Position($1->position(), $4->position());
        std::vector<MapStatementNode*>* body = new std::vector<MapStatementNode*>();
        $$ = new MapNode(pos, body, $4);
    }

    | LBRACE mapStatements RBRACE {
        Position* pos = new Position($1->position(), $3->position());
        $$ = new MapNode(pos, $2);
    }

    | expressionF {
        $$ = $1;
    }

expressionF :
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

    | expressionF EQUAL expressionF {
        Position* pos = new Position($1->position(), $3->position());
        $$ = new EqualsNode(pos, $1, $3);
    }

    | expressionF NOTEQUAL expressionF {
        Position* pos = new Position($1->position(), $3->position());
        $$ = new NotEqualsNode(pos, $1, $3);
    }

    | expressionF ADD expressionF {
        Position* pos = new Position($1->position(), $3->position());
        $$ = new AddNode(pos, $1, $3);
    }

    | expressionF SUBTRACT expressionF {
        Position* pos = new Position($1->position(), $3->position());
        $$ = new SubtractNode(pos, $1, $3);
    }

    | expressionF MULTIPLY expressionF {
        Position* pos = new Position($1->position(), $3->position());
        $$ = new MultiplyNode(pos, $1, $3);
    }

    | expressionF DIVIDE expressionF {
        Position* pos = new Position($1->position(), $3->position());
        $$ = new DivideNode(pos, $1, $3);
    }

    | expressionF MODULUS expressionF {
        Position* pos = new Position($1->position(), $3->position());
        $$ = new ModulusNode(pos, $1, $3);
    }

    | expressionF POWER expressionF {
        Position* pos = new Position($1->position(), $3->position());
        $$ = new PowerNode(pos, $1, $3);
    }

    | expressionF CAT expressionF {
        Position* pos = new Position($1->position(), $3->position());
        $$ = new ConcatenateNode(pos, $1, $3);
    }

    | expressionF OR expressionF {
        Position* pos = new Position($1->position(), $3->position());
        $$ = new OrNode(pos, $1, $3);
    }

    | expressionF AND expressionF {
        Position* pos = new Position($1->position(), $3->position());
        $$ = new AndNode(pos, $1, $3);
    }

    | expressionF LARROW expressionF {
        Position* pos = new Position($1->position(), $3->position());
        $$ = new NumericComparisonExpressionNode(pos, NumberComparisonType::LESS_THAN, $1, $3);
    }

    | expressionF RARROW expressionF {
        Position* pos = new Position($1->position(), $3->position());
        $$ = new NumericComparisonExpressionNode(pos, NumberComparisonType::GREATER_THAN, $1, $3);
    }

    | expressionF LARROWEQUALS expressionF {
        Position* pos = new Position($1->position(), $3->position());
        $$ = new NumericComparisonExpressionNode(pos, NumberComparisonType::LESS_THAN_OR_EQUAL, $1, $3);
    }

    | expressionF RARROWEQUALS expressionF {
        Position* pos = new Position($1->position(), $3->position());
        $$ = new NumericComparisonExpressionNode(pos, NumberComparisonType::GREATER_THAN_OR_EQUAL, $1, $3);
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

    | function {
        $$ = $1;
    }

    | type {
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

formals :
    id COLON type {
        $$ = new std::vector<std::pair<TypeLiteral*, IdentifierNode*>>();
        auto t = std::pair<TypeLiteral*, IdentifierNode*>($3, $1);
        $$->push_back(t);
    }

    | formals COMMA id COLON type {
        $$ = $1;
        auto t = std::pair<TypeLiteral*, IdentifierNode*>($5, $3);
        $$->push_back(t);
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
