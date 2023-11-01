%skeleton "lalr1.cc"
%require "3.0"
%debug
%defines
%define api.namespace{swarmc::Lang}
%define api.parser.class {Parser}
%define parse.error verbose
%token-table

%code requires{
	#include <list>
    #include <float.h>
    #include "../shared/nslib.h"
	#include "../lang/Token.h"
   	#include "../lang/AST.h"
   	#include "../lang/Type.h"
    #include "../Reporting.h"
    #include "../errors/ParseError.h"

    using namespace nslib;

	namespace swarmc::Lang {
		class Scanner;
	}
    class Shared {
    public:
        Shared(swarmc::Lang::Position* pos, bool shared): _pos(useref(pos)), _shared(shared) {}
        ~Shared() { freeref(_pos); }
        swarmc::Lang::Position* position() const { return _pos; }
        bool shared() const { return _shared; }
    private:
        swarmc::Lang::Position* _pos;
        bool _shared;
    };

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

    #include "../shared/nslib.h"
    #include "../lang/Scanner.h"
    #include "../lang/Token.h"
    #undef yylex
    #define yylex scanner.yylex

    using namespace nslib;
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
    swarmc::Lang::IIFExpressionNode*    transIIFExpression;
    swarmc::Lang::TypeLiteral*          transType;
    swarmc::Lang::DeclarationNode*      transDeclaration;
    swarmc::Lang::MapStatementNode*     transMapStatement;
    swarmc::Lang::FunctionNode*         transFunction;
    std::vector<std::pair<TypeLiteral*, IdentifierNode*>>*  transFormals;
    std::vector<swarmc::Lang::MapStatementNode*>* transMapStatements;
    std::vector<swarmc::Lang::ExpressionNode*>* transExpressions;
    std::vector<swarmc::Lang::DeclarationNode*>* transDeclarations;
    std::vector<swarmc::Lang::IdentifierNode*>* transIdentifiers;
    Shared*                             transShared;
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
%token <transToken>      TYPE
%token <transNumberToken> NUMBERLITERAL
%token <transStringToken> STRINGLITERAL
%token <transToken>      ENUMERABLE
%token <transToken>      MAP
%token <transToken>      IF
%token <transToken>      WHILE
%token <transToken>      TTRUE
%token <transToken>      TFALSE
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
%token <transToken>      ANDASSIGN
%token <transToken>      ORASSIGN
%token <transToken>      MODULUS
%token <transToken>      POWER
%token <transToken>      DOT
%token <transToken>      SHARED
%token <transToken>      FN
%token <transToken>      ARROW
%token <transToken>      FNDEF
%token <transToken>      INCLUDE
%token <transToken>      FROM
%token <transToken>      CONSTRUCTOR
%token <transToken>      SQRT
%token <transToken>      WILDCARD

/*    (attribute type)      (nonterminal)    */
%type <transProgram>        program
%type <transProgram>        includes
%type <transProgram>        statements
%type <transStatement>      statement
%type <transDeclarations>   typeBody
%type <transID>             id
%type <transLVal>           lval
%type <transExpression>     expression
%type <transExpression>     expressionF
%type <transExpression>     term
%type <transExpression>     callable
%type <transAssignExpression> assignment
%type <transCallExpression> callExpression
%type <transIIFExpression>  iifExpression
%type <transExpressions>    actuals
%type <transFormals>        formals
%type <transFunction>       function
%type <transDeclaration>    funcConstr
%type <transType>           type
%type <transType>           typeid
%type <transDeclaration>    declaration
%type <transMapStatement>   mapStatement
%type <transMapStatements>  mapStatements
%type <transShared>         shared
%type <transLVal>           classAccess
%type <transIdentifiers>    includeSyms

%precedence FNDEF
%left OR
%left AND
%nonassoc EQUAL NOTEQUAL LARROW LARROWEQUALS RARROW RARROWEQUALS
%left SUBTRACT ADD
%left MULTIPLY DIVIDE MODULUS
%left POWER
%right ARROW

%%

program :
    includes statements END {
        $$ = $1;
        $$->assumeAndReduceStatements($2->reduceToStatements());
        *root = $$;
    }

includes :
    includes INCLUDE classAccess SEMICOLON {
        $$ = $1;
        $$->pushStatement(new IncludeStatementNode(new Position($2->position(), $4->position()), (ClassAccessNode*)$3, nullptr));
        delete $2; delete $4;
    }

    | includes FROM classAccess INCLUDE LBRACE includeSyms RBRACE SEMICOLON {
        $$ = $1;
        $$->pushStatement(new IncludeStatementNode(new Position($2->position(), $8->position()), (ClassAccessNode*)$3, $6));
        delete $2; delete $4; delete $5; delete $7; delete $8;
    }

    | includes FROM classAccess INCLUDE id SEMICOLON {
        $$ = $1;
        auto ids = new std::vector<IdentifierNode*>();
        ids->push_back(useref($5));
        $$->pushStatement(new IncludeStatementNode(new Position($2->position(), $6->position()), (ClassAccessNode*)$3, ids));
        delete $2; delete $4; delete $6;
    }

    | %empty {
        $$ = new ProgramNode();
    }

includeSyms :
    includeSyms COMMA id {
        $$ = $1;
        $$->push_back(useref($3));
        delete $2;
    }

    | id {
        $$ = new std::vector<IdentifierNode*>();
        $$->push_back(useref($1));
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
    ENUMERATE lval AS shared id LBRACE statements RBRACE {
        Position* pos = new Position($1->position(), $8->position());
        EnumerationStatement* e = new EnumerationStatement(pos, $2, $5, $4->shared());
        e->assumeAndReduceStatements($7->reduceToStatements());
        $$ = e;
        delete $1; delete $3; delete $4; delete $6; delete $8;
    }

    | ENUMERATE lval AS shared id COMMA shared id LBRACE statements RBRACE {
        Position* pos = new Position($1->position(), $11->position());
        EnumerationStatement* e = new EnumerationStatement(pos, $2, $5, $8, $4->shared());
        auto t = Type::Primitive::of(Type::Intrinsic::NUMBER);
        $8->overrideSymbol(new VariableSymbol($8->name(), t, $8->position(), $7->shared()));
        e->assumeAndReduceStatements($10->reduceToStatements());
        $$ = e;
        delete $1; delete $3; delete $4; delete $6; delete $7; delete $9; delete $11;
    }

    | ENUMERATE lval AS WILDCARD LBRACE statements RBRACE {
        Position* pos = new Position($1->position(), $7->position());
        EnumerationStatement* e = new EnumerationStatement(pos, $2, new IdentifierNode($4->position(), "_"), false);
        e->assumeAndReduceStatements($6->reduceToStatements());
        $$ = e;
        delete $1; delete $3; delete $4; delete $5; delete $7;
    }

    | ENUMERATE lval AS WILDCARD COMMA shared id LBRACE statements RBRACE {
        Position* pos = new Position($1->position(), $10->position());
        EnumerationStatement* e = new EnumerationStatement(pos, $2, new IdentifierNode($4->position(), "_"), $7, false);
        auto t = Type::Primitive::of(Type::Intrinsic::NUMBER);
        $7->overrideSymbol(new VariableSymbol($7->name(), t, $7->position(), $6->shared()));
        e->assumeAndReduceStatements($9->reduceToStatements());
        $$ = e;
        delete $1; delete $3; delete $4; delete $5; delete $6; delete $8; delete $10;
    }

    | WITH term AS shared id LBRACE statements RBRACE {
        Position* pos = new Position($1->position(), $8->position());
        WithStatement* w = new WithStatement(pos, $2, $5, $4->shared());
        w->assumeAndReduceStatements($7->reduceToStatements());
        $$ = w;
        delete $1; delete $3; delete $4; delete $6; delete $8;
    }

    | WITH term AS WILDCARD LBRACE statements RBRACE {
        Position* pos = new Position($1->position(), $7->position());
        WithStatement* w = new WithStatement(pos, $2, new IdentifierNode($4->position(), "_"), false);
        w->assumeAndReduceStatements($6->reduceToStatements());
        $$ = w;
        delete $1; delete $3; delete $4; delete $5; delete $7;
    }

    | IF LPAREN expression RPAREN LBRACE statements RBRACE {
        Position* pos = new Position($1->position(), $7->position());
        IfStatement* i = new IfStatement(pos, $3);
        i->assumeAndReduceStatements($6->reduceToStatements());
        $$ = i;
        delete $1; delete $2; delete $4; delete $5; delete $7;
    }

    | WHILE LPAREN expression RPAREN LBRACE statements RBRACE {
        Position* pos = new Position($1->position(), $7->position());
        WhileStatement* w = new WhileStatement(pos, $3);
        w->assumeAndReduceStatements($6->reduceToStatements());
        $$ = w;
        delete $1; delete $2; delete $4; delete $5; delete $7;
    }

    | callExpression SEMICOLON {
        Position* pos = new Position($1->position(), $2->position());
        $$ = new ExpressionStatementNode(pos, $1);
        delete $2;
    }

    | declaration SEMICOLON {
        $$ = $1;
        delete $2;
    }

    | assignment SEMICOLON {
        Position* pos = new Position($1->position(), $2->position());
        $$ = new ExpressionStatementNode(pos, $1);
        delete $2;
    }

    | RETURN SEMICOLON {
        Position* pos = new Position($1->position(), $2->position());
        $$ = new ReturnStatementNode(pos, nullptr);
        delete $1; delete $2;
    }

    | RETURN expression SEMICOLON {
        Position* pos = new Position($1->position(), $3->position());
        $$ = new ReturnStatementNode(pos, $2);
        delete $1; delete $3;
    }

    | CONTINUE SEMICOLON {
        Position* pos = new Position($1->position(), $2->position());
        $$ = new ContinueNode(pos);
        delete $1; delete $2;
    }

    | BREAK SEMICOLON {
        Position* pos = new Position($1->position(), $2->position());
        $$ = new BreakNode(pos);
        delete $1; delete $2;
    }

shared :
    SHARED {
        $$ = new Shared($1->position()->copy(), true);
        delete $1;
    }

    | %empty {
        $$ = new Shared(nullptr, false);
    }

declaration :
    SHARED typeid id ASSIGN expression {
        Position* pos = new Position($1->position(), $5->position());
        VariableDeclarationNode* var = new VariableDeclarationNode(pos, $2, $3, $5, true);
        $$ = var;
        delete $1; delete $4;
    }

    | typeid id ASSIGN expression {
        Position* pos = new Position($1->position(), $4->position());
        VariableDeclarationNode* var = new VariableDeclarationNode(pos, $1, $2, $4, false);
        $$ = var;
        delete $3;
    }

    | shared FN id ASSIGN function {
        auto p1 = $1->position() == nullptr ? $2->position() : $1->position();
        Position* pos = new Position(p1, $5->position());
        $$ = new VariableDeclarationNode(pos, $5->typeNode(), $3, $5, $1->shared());
        delete $1; delete $2; delete $4;
    }

typeBody :
    typeBody declaration SEMICOLON {
        $$ = $1;
        $$->push_back(useref($2));
        delete $3;
    }

    | typeBody typeid id SEMICOLON {
        $$ = $1;
        auto pos = new Position($2->position(), $3->position());
        $$->push_back(useref(new UninitializedVariableDeclarationNode(pos, $2, $3)));
        delete $4;
    }

    | typeBody funcConstr SEMICOLON {
        $$ = $1;
        $$->push_back(useref($2));
        delete $3;
    }

    | declaration SEMICOLON {
        $$ = new std::vector<DeclarationNode*>();
        $$->push_back(useref($1));
        delete $2;
    }

    | typeid id SEMICOLON {
        $$ = new std::vector<DeclarationNode*>();
        auto pos = new Position($1->position(), $2->position());
        $$->push_back(useref(new UninitializedVariableDeclarationNode(pos, $1, $2)));
        delete $3;
    }

    | funcConstr SEMICOLON {
        $$ = new std::vector<DeclarationNode*>();
        $$->push_back(useref($1));
        delete $2;
    }

assignment :
    lval ASSIGN expression {
        Position* pos = new Position($1->position(), $3->position());
        $$ = new AssignExpressionNode(pos, $1, $3);
        delete $2;
    }

    | lval ADDASSIGN expression {
        Position* pos = new Position($1->position(), $3->position());
        auto r = new AddNode(pos, $1, $3);
        $$ = new AssignExpressionNode(pos, $1->copy(), r);
        delete $2;
    }

    | lval MULTIPLYASSIGN expression {
        Position* pos = new Position($1->position(), $3->position());
        auto r = new MultiplyNode(pos, $1, $3);
        $$ = new AssignExpressionNode(pos, $1->copy(), r);
        delete $2;
    }

    | lval SUBTRACTASSIGN expression {
        Position* pos = new Position($1->position(), $3->position());
        auto r = new SubtractNode(pos, $1, $3);
        $$ = new AssignExpressionNode(pos, $1->copy(), r);
        delete $2;
    }

    | lval DIVIDEASSIGN expression {
        Position* pos = new Position($1->position(), $3->position());
        auto r = new DivideNode(pos, $1, $3);
        $$ = new AssignExpressionNode(pos, $1->copy(), r);
        delete $2;
    }

    | lval MODULUSASSIGN expression {
        Position* pos = new Position($1->position(), $3->position());
        auto r = new ModulusNode(pos, $1, $3);
        $$ = new AssignExpressionNode(pos, $1->copy(), r);
        delete $2;
    }

    | lval POWERASSIGN expression {
        Position* pos = new Position($1->position(), $3->position());
        auto r = new PowerNode(pos, $1, $3);
        $$ = new AssignExpressionNode(pos, $1->copy(), r);
        delete $2;
    }

    | lval ANDASSIGN expression {
        Position* pos = new Position($1->position(), $3->position());
        auto r = new AndNode(pos, $1, $3);
        $$ = new AssignExpressionNode(pos, $1->copy(), r);
        delete $2;
    }

    | lval ORASSIGN expression {
        Position* pos = new Position($1->position(), $3->position());
        auto r = new OrNode(pos, $1, $3);
        $$ = new AssignExpressionNode(pos, $1->copy(), r);
        delete $2;
    }


lval :
    id {
        $$ = $1;
    }

    | lval LBRACE id RBRACE {
        Position* pos = new Position($1->position(), $4->position());
        $$ = new MapAccessNode(pos, $1, $3);
        delete $2; delete $4;
    }

    | lval LBRACKET expression RBRACKET {
        Position* pos = new Position($1->position(), $4->position());
        $$ = new EnumerableAccessNode(pos, $1, $3);
        delete $2; delete $4;
    }

    | classAccess {
        $$ = $1;
    }

classAccess :
    lval DOT id {
        $$ = new ClassAccessNode(new Position($1->position(), $3->position()), $1, $3);
        delete $2;
    }

id :
    ID {
        $$ = new IdentifierNode($1->position(), $1->identifier());
        delete $1;
    }

typeid :
    id {
        $$ = new TypeLiteral($1->position(), Type::Ambiguous::partial($1));
    }

    | type { 
        $$ = $1; 
    }

type :
    STRING {
        auto t = Type::Primitive::of(Type::Intrinsic::STRING);
        $$ = new TypeLiteral($1->position(), t);
        delete $1;
    }

    | NUMBER {
        auto t = Type::Primitive::of(Type::Intrinsic::NUMBER);
        $$ = new TypeLiteral($1->position(), t);
        delete $1;
    }

    | BOOL {
        auto t = Type::Primitive::of(Type::Intrinsic::BOOLEAN);
        $$ = new TypeLiteral($1->position(), t);
        delete $1;
    }

    | VOID {
        auto t = Type::Primitive::of(Type::Intrinsic::VOID);
        $$ = new TypeLiteral($1->position(), t);
        delete $1;
    }

    | TYPE {
        auto t = Type::Primitive::of(Type::Intrinsic::TYPE);
        $$ = new TypeLiteral($1->position(), t);
        delete $1;
    }

    | ENUMERABLE LARROW typeid RARROW {
        Position* pos = new Position($1->position(), $4->position());
        $$ = new TypeLiteral(pos, new Type::Enumerable($3->value()));
        delete $1; delete $2; delete $4;
    }

    | MAP LARROW typeid RARROW {
        Position* pos = new Position($1->position(), $4->position());
        $$ = new TypeLiteral(pos, new Type::Map($3->value()));
        delete $1; delete $2; delete $4;
    }

    | typeid ARROW typeid {
        Position* pos = new Position($1->position(), $3->position());
        $$ = new TypeLiteral(pos, new Type::Lambda1($1->value(), $3->value()));
        delete $2;
    }

function :
    LPAREN formals RPAREN COLON typeid FNDEF LBRACE statements RBRACE {
        Position* pos = new Position($1->position(), $9->position());
        Position* typepos = new Position($1->position(), $5->position());

        Type::Type* t = $5->value();

        for ( auto i = $2->rbegin(); i != $2->rend(); ++i ) {
            t = new Type::Lambda1((*i).first->value(), t);
        }

        FunctionNode* fn = new FunctionNode(pos, new TypeLiteral(typepos, t), $2);
        fn->assumeAndReduceStatements($8->reduceToStatements());
        $$ = fn;
        delete $1; delete $3; delete $4; delete $6; delete $7; delete $9;
    }

    | LPAREN RPAREN COLON typeid FNDEF LBRACE statements RBRACE {
        Position* pos = new Position($1->position(), $8->position());

        FunctionNode* fn = new FunctionNode(
            pos, new TypeLiteral($4->position(), new Type::Lambda0($4->value())), new FormalList());
        fn->assumeAndReduceStatements($7->reduceToStatements());
        $$ = fn;
        delete $1; delete $2; delete $3; delete $5; delete $6; delete $8;
    }

    | LPAREN formals RPAREN COLON typeid FNDEF expressionF {
        Position* pos = new Position($1->position(), $7->position());
        Position* typepos = new Position($1->position(), $5->position());

        Type::Type* t = $5->value();

        for ( auto i = $2->rbegin(); i != $2->rend(); ++i ) {
            t = new Type::Lambda1((*i).first->value(), t);
        }

        FunctionNode* fn = new FunctionNode(pos, new TypeLiteral(typepos, t), $2);
        auto retstmt = new StatementList();
        retstmt->push_back(new ReturnStatementNode($7->position()->copy(), $7));
        fn->assumeAndReduceStatements(retstmt);
        $$ = fn;
        delete $1; delete $3; delete $4; delete $6;
    }

    | LPAREN RPAREN COLON typeid FNDEF expressionF {
        Position* pos = new Position($1->position(), $6->position());

        FunctionNode* fn = new FunctionNode(
            pos, new TypeLiteral($4->position()->copy(), new Type::Lambda0($4->value())), new FormalList());
        auto retstmt = new StatementList();
        retstmt->push_back(new ReturnStatementNode($6->position()->copy(), $6));
        fn->assumeAndReduceStatements(retstmt);
        $$ = fn;
        delete $1; delete $2; delete $3; delete $5;
    }

funcConstr :
    CONSTRUCTOR LPAREN RPAREN FNDEF LBRACE statements RBRACE {
        auto pos = new Position($1->position(), $7->position());
        auto pos2 = new Position($2->position(), $7->position());
        auto t = new TypeLiteral($1->position(), new Type::Lambda0(Type::Primitive::of(Type::Intrinsic::VOID)));
        auto func = new FunctionNode(pos2, t, new FormalList());
        func->assumeAndReduceStatements($6->reduceToStatements());
        $$ = new ConstructorNode(pos, func);
        delete $1; delete $2; delete $3; delete $4; delete $5; delete $7;
    }

    | CONSTRUCTOR LPAREN formals RPAREN FNDEF LBRACE statements RBRACE {
        Position* pos = new Position($1->position(), $8->position());
        Position* pos2 = new Position($2->position(), $8->position());
        Position* typepos = new Position($1->position(), $4->position());

        Type::Type* t = Type::Primitive::of(Type::Intrinsic::VOID);

        for ( auto i = $3->rbegin(); i != $3->rend(); ++i ) {
            t = new Type::Lambda1((*i).first->value(), t);
        }

        auto func = new FunctionNode(pos2, new TypeLiteral(typepos, t), $3);
        func->assumeAndReduceStatements($7->reduceToStatements());
        $$ = new ConstructorNode(pos, func);
        delete $1; delete $2; delete $4; delete $5; delete $6; delete $8;
    }

expression :
    LBRACE RBRACE OF typeid {
        Position* pos = new Position($1->position(), $4->position());
        std::vector<MapStatementNode*>* body = new std::vector<MapStatementNode*>();
        $$ = new MapNode(pos, body, new TypeLiteral($4->position(), new Type::Map($4->value())));
        delete $1; delete $2; delete $3; delete $4;
    }

    | LBRACE mapStatements RBRACE {
        Position* pos = new Position($1->position(), $3->position());
        $$ = new MapNode(pos, $2);
        delete $1; delete $3;
    }

    | LBRACE typeBody RBRACE {
        Position* pos = new Position($1->position(), $3->position());
        auto t = new Type::Object();
        for ( auto decl : *$2 ) {
            if (decl->getName() == "VariableDeclarationNode") {
                auto vdecl = (VariableDeclarationNode*)decl;
                t->defineProperty(vdecl->id()->name(), vdecl->typeNode()->value());
            } else if (decl->getName() == "UninitializedVariableDeclarationNode") {
                auto vdecl = (UninitializedVariableDeclarationNode*)decl;
                t->defineProperty(vdecl->id()->name(), vdecl->typeNode()->value());
            }
        }
        $$ = new TypeBodyNode(pos, $2, t->finalize());
        delete $1; delete $3; delete t;
    }

    | expressionF {
        $$ = $1;
    }

expressionF :
    term {
        $$ = $1;
    }

    | LBRACKET RBRACKET OF typeid {
        Position* pos = new Position($1->position(), $4->position());
        std::vector<ExpressionNode*>* actuals = new std::vector<ExpressionNode*>();
        $$ = new EnumerationLiteralExpressionNode(pos, actuals,
            new TypeLiteral($4->position(), new Type::Enumerable($4->value())));
        delete $1; delete $2; delete $3; delete $4;
    }

    | LBRACKET actuals RBRACKET {
        Position* pos = new Position($1->position(), $3->position());
        $$ = new EnumerationLiteralExpressionNode(pos, $2);
        delete $1; delete $3;
    }

    | expressionF EQUAL expressionF {
        Position* pos = new Position($1->position(), $3->position());
        $$ = new EqualsNode(pos, $1, $3);
        delete $2;
    }

    | expressionF NOTEQUAL expressionF {
        Position* pos = new Position($1->position(), $3->position());
        $$ = new NotEqualsNode(pos, $1, $3);
        delete $2;
    }

    | expressionF ADD expressionF {
        Position* pos = new Position($1->position(), $3->position());
        $$ = new AddNode(pos, $1, $3);
        delete $2;
    }

    | expressionF SUBTRACT expressionF {
        Position* pos = new Position($1->position(), $3->position());
        $$ = new SubtractNode(pos, $1, $3);
        delete $2;
    }

    | expressionF MULTIPLY expressionF {
        Position* pos = new Position($1->position(), $3->position());
        $$ = new MultiplyNode(pos, $1, $3);
        delete $2;
    }

    | expressionF DIVIDE expressionF {
        Position* pos = new Position($1->position(), $3->position());
        $$ = new DivideNode(pos, $1, $3);
        delete $2;
    }

    | expressionF MODULUS expressionF {
        Position* pos = new Position($1->position(), $3->position());
        $$ = new ModulusNode(pos, $1, $3);
        delete $2;
    }

    | expressionF POWER expressionF {
        Position* pos = new Position($1->position(), $3->position());
        $$ = new PowerNode(pos, $1, $3);
        delete $2;
    }

    | expressionF OR expressionF {
        Position* pos = new Position($1->position(), $3->position());
        $$ = new OrNode(pos, $1, $3);
        delete $2;
    }

    | expressionF AND expressionF {
        Position* pos = new Position($1->position(), $3->position());
        $$ = new AndNode(pos, $1, $3);
        delete $2;
    }

    | expressionF LARROW expressionF {
        Position* pos = new Position($1->position(), $3->position());
        $$ = new NumericComparisonExpressionNode(pos, NumberComparisonType::LESS_THAN, $1, $3);
        delete $2;
    }

    | expressionF RARROW expressionF {
        Position* pos = new Position($1->position(), $3->position());
        $$ = new NumericComparisonExpressionNode(pos, NumberComparisonType::GREATER_THAN, $1, $3);
        delete $2;
    }

    | expressionF LARROWEQUALS expressionF {
        Position* pos = new Position($1->position(), $3->position());
        $$ = new NumericComparisonExpressionNode(pos, NumberComparisonType::LESS_THAN_OR_EQUAL, $1, $3);
        delete $2;
    }

    | expressionF RARROWEQUALS expressionF {
        Position* pos = new Position($1->position(), $3->position());
        $$ = new NumericComparisonExpressionNode(pos, NumberComparisonType::GREATER_THAN_OR_EQUAL, $1, $3);
        delete $2;
    }

    | SUBTRACT term {
        Position* pos = new Position($1->position(), $2->position());
        $$ = new NegativeExpressionNode(pos,$2);
        delete $1;
    }

    | NOT term {
        Position* pos = new Position($1->position(), $2->position());
        $$ = new NotNode(pos, $2);
        delete $1;
    }

    | SQRT term {
        Position* pos = new Position($1->position(), $2->position());
        $$ = new SqrtNode(pos, $2);
        delete $1;
    }



term :
    callExpression {
        $$ = $1;
    }

    | iifExpression {
        $$ = $1;
    }

    | lval {
        $$ = $1;
    }

    | STRINGLITERAL {
        $$ = new StringLiteralExpressionNode($1->position(), $1->value());
        delete $1;
    }

    | NUMBERLITERAL {
        $$ = new NumberLiteralExpressionNode($1->position(), $1->value());
        delete $1;
    }

    | TTRUE {
        $$ = new BooleanLiteralExpressionNode($1->position(), true);
        delete $1;
    }

    | TFALSE {
        $$ = new BooleanLiteralExpressionNode($1->position(), false);
        delete $1;
    }

    | LPAREN expression RPAREN {
        $$ = $2;
        delete $1; delete $3;
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
        actuals->push_back(useref($1));
        $$ = actuals;
    }

    | actuals COMMA expression {
        $$ = $1;
        $$->push_back(useref($3));
        delete $2;
    }

formals :
    id COLON typeid {
        $$ = new std::vector<std::pair<TypeLiteral*, IdentifierNode*>>();
        auto t = std::pair<TypeLiteral*, IdentifierNode*>(useref($3), useref($1));
        $$->push_back(t);
        delete $2;
    }

    | formals COMMA id COLON typeid {
        $$ = $1;
        auto t = std::pair<TypeLiteral*, IdentifierNode*>(useref($5), useref($3));
        $$->push_back(t);
        delete $2; delete $4;
    }



mapStatements :
    mapStatement {
        std::vector<MapStatementNode*>* stmts = new std::vector<MapStatementNode*>();
        stmts->push_back(useref($1));
        $$ = stmts;
    }

    | mapStatements COMMA mapStatement {
        $1->push_back(useref($3));
        $$ = $1;
        delete $2;
    }



mapStatement :
    id COLON expression {
        Position* pos = new Position($1->position(), $3->position());
        $$ = new MapStatementNode(pos, $1, $3);
        delete $2;
    }



callExpression :
    callable LPAREN RPAREN {
        Position* pos = new Position($1->position(), $3->position());
        $$ = new CallExpressionNode(pos, $1, new std::vector<ExpressionNode*>());
        delete $2; delete $3;
    }

    | callable LPAREN actuals RPAREN {
        Position* pos = new Position($1->position(), $4->position());
        $$ = new CallExpressionNode(pos, $1, $3);
        delete $2; delete $4;
    }

callable :
    callExpression { $$ = $1; }
    | iifExpression { $$ = $1; }
    | lval { $$ = $1; }

iifExpression :
    LPAREN expression RPAREN LPAREN RPAREN {
        Position* pos = new Position($1->position(), $5->position());
        $$ = new IIFExpressionNode(pos, $2, new std::vector<ExpressionNode*>());
        delete $1; delete $3; delete $4; delete $5;
    }

    | LPAREN expression RPAREN LPAREN actuals RPAREN {
        Position* pos = new Position($1->position(), $6->position());
        $$ = new IIFExpressionNode(pos, $2, $5);
        delete $1; delete $3; delete $4; delete $6;
    }

%%

void swarmc::Lang::Parser::error(const std::string& msg){
    Console* console = Console::get();
    console->error(msg);
}
