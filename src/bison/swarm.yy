%skeleton "lalr1.cc"
%require "3.8"
%debug
%defines
%define api.namespace{swarmc::Lang}
%define api.parser.class {Parser}
%define parse.error custom

%code requires {
    #include <vector>
    #include "../lang/Token.h"
    #include "../lang/AST.h"

    namespace swarmc::Lang {
        class Scanner;
    };

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

%parse-param { swarmc::Lang::Scanner &scanner } { swarmc::Lang::ProgramNode** root } { std::string file }
%code{
    #include <iostream>
    #include <cstdlib>
    #include <fstream>
    #include <float.h>

   	#include "../lang/Type.h"
    #include "../errors/ParseError.h"
    #include "../lang/Scanner.h"
    #undef yylex
    #define yylex scanner.yylex

    auto parseLog = nslib::Logging::get()->get("Bison");
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
%token <transToken>      SOCKET
%token <transToken>      SOCKETCONNECTION
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
%token <transToken>      ROOT
%token <transToken>      WILDCARD
%token <transToken>      DEFER
%token <transToken>      USE

/*    (attribute type)      (nonterminal)    */
%type <transProgram>        program
%type <transProgram>        includes
%type <transProgram>        statements
%type <transStatement>      statement
%type <transDeclarations>   typeBody
%type <transID>             id
%type <transLVal>           lval
%type <transLVal>           assignable
%type <transExpression>     expression
%type <transExpression>     expressionF
%type <transExpression>     term
%type <transExpression>     callable
%type <transAssignExpression> assignment
%type <transCallExpression> callExpression
%type <transCallExpression>  iifExpression
%type <transExpressions>    actuals
%type <transExpressions>    callExpressions
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
%type <transIdentifiers>    idList

%precedence FNDEF
%left OR
%left AND
%nonassoc EQUAL NOTEQUAL LARROW LARROWEQUALS RARROW RARROWEQUALS
%left SUBTRACT ADD
%left MULTIPLY DIVIDE MODULUS
%left POWER ROOT
%right ARROW

%%

program :
    includes statements END {
        $$ = $1;
        $$->assumeAndReduceStatements($2->reduceToStatements());
        if ( yynerrs_ > 0 ) {
            delete $$;
            throw Errors::ParseError();
        }
        *root = $$;
    }

includes :
    includes INCLUDE classAccess SEMICOLON {
        $$ = $1;
        auto pos = new Position($2->position(), $4->position());
        $$->pushStatement(new IncludeStatementNode(pos, (ClassAccessNode*)$3, nullptr));
        parseLog->debug(s(pos) + " Finished: " + s($$->body()->back()));
    }

    | includes FROM classAccess INCLUDE LBRACE idList RBRACE SEMICOLON {
        $$ = $1;
        auto pos = new Position($2->position(), $8->position());
        $$->pushStatement(new IncludeStatementNode(pos, (ClassAccessNode*)$3, $6));
        parseLog->debug(s(pos) + " Finished: " + s($$->body()->back()));
    }

    | includes FROM classAccess INCLUDE id SEMICOLON {
        $$ = $1;
        auto ids = new std::vector<IdentifierNode*>();
        ids->push_back(useref($5));
        auto pos = new Position($2->position(), $6->position());
        $$->pushStatement(new IncludeStatementNode(pos, (ClassAccessNode*)$3, ids));
        parseLog->debug(s(pos) + " Finished: " + s($$->body()->back()));
    }

    | %empty {
        $$ = new ProgramNode();
    }

idList :
    idList COMMA id {
        $$ = $1;
        $$->push_back(useref($3));
    }

    | id {
        $$ = new std::vector<IdentifierNode*>();
        $$->push_back(useref($1));
    }

statements :
    statements statement {
        $1->pushStatement($2);
        $$ = $1;
        parseLog->debug(s($2->position()) + " Finished: " + s($$->body()->back()));
    }

    | %empty {
        $$ = new ProgramNode();
    }



statement :
    ENUMERATE expression AS shared id LBRACE statements RBRACE {
        Position* pos = new Position($1->position(), $8->position());
        auto e = new EnumerationStatement(pos, $2, $5, $4->shared());
        e->assumeAndReduceStatements($7->reduceToStatements());
        $$ = e;
        delete $4;
    }

    | ENUMERATE expression AS shared id COMMA shared id LBRACE statements RBRACE {
        Position* pos = new Position($1->position(), $11->position());
        auto e = new EnumerationStatement(pos, $2, $5, $8, $4->shared());
        auto t = Type::Primitive::of(Type::Intrinsic::NUMBER);
        $8->overrideSymbol(new VariableSymbol($8->name(), t, $8->position(), $7->shared(), false));
        e->assumeAndReduceStatements($10->reduceToStatements());
        $$ = e;
        delete $4; delete $7;
    }

    | ENUMERATE expression AS WILDCARD LBRACE statements RBRACE {
        Position* pos = new Position($1->position(), $7->position());
        auto e = new EnumerationStatement(pos, $2, new IdentifierNode($4->position(), "_"), false);
        e->assumeAndReduceStatements($6->reduceToStatements());
        $$ = e;
    }

    | ENUMERATE expression AS WILDCARD COMMA shared id LBRACE statements RBRACE {
        Position* pos = new Position($1->position(), $10->position());
        auto e = new EnumerationStatement(pos, $2, new IdentifierNode($4->position(), "_"), $7, false);
        auto t = Type::Primitive::of(Type::Intrinsic::NUMBER);
        $7->overrideSymbol(new VariableSymbol($7->name(), t, $7->position(), $6->shared(), false));
        e->assumeAndReduceStatements($9->reduceToStatements());
        $$ = e;
        delete $6;
    }

    | WITH term AS shared id LBRACE statements RBRACE {
        Position* pos = new Position($1->position(), $8->position());
        auto w = new WithStatement(pos, $2, $5, $4->shared());
        w->assumeAndReduceStatements($7->reduceToStatements());
        $$ = w;
        delete $4;
    }

    | WITH term AS WILDCARD LBRACE statements RBRACE {
        Position* pos = new Position($1->position(), $7->position());
        auto w = new WithStatement(pos, $2, new IdentifierNode($4->position(), "_"), false);
        w->assumeAndReduceStatements($6->reduceToStatements());
        $$ = w;
    }

    | IF LPAREN expression RPAREN LBRACE statements RBRACE {
        Position* pos = new Position($1->position(), $7->position());
        auto i = new IfStatement(pos, $3);
        i->assumeAndReduceStatements($6->reduceToStatements());
        $$ = i;
    }

    | WHILE LPAREN expression RPAREN LBRACE statements RBRACE {
        Position* pos = new Position($1->position(), $7->position());
        auto w = new WhileStatement(pos, $3);
        w->assumeAndReduceStatements($6->reduceToStatements());
        $$ = w;
    }

    | callExpression SEMICOLON {
        Position* pos = new Position($1->position(), $2->position());
        $$ = new ExpressionStatementNode(pos, $1);
    }

    | DEFER callExpression SEMICOLON {
        Position* pos = new Position($1->position(), $3->position());
        auto dc = new DeferCallExpressionNode(pos, $2);
        $$ = new ExpressionStatementNode(pos, dc);
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

    | CONTINUE SEMICOLON {
        Position* pos = new Position($1->position(), $2->position());
        $$ = new ContinueNode(pos);
    }

    | BREAK SEMICOLON {
        Position* pos = new Position($1->position(), $2->position());
        $$ = new BreakNode(pos);
    }

shared :
    SHARED {
        $$ = new Shared($1->position()->copy(), true);
    }

    | %empty {
        $$ = new Shared(nullptr, false);
    }

declaration :
    SHARED typeid id ASSIGN expression {
        Position* pos = new Position($1->position(), $5->position());
        auto assign = new AssignExpressionNode(
            new Position($3->position(), $5->position()),
            $3,
            $5
        );
        $$ = new VariableDeclarationNode(pos, $2, assign, true);
    }

    | typeid id ASSIGN expression {
        Position* pos = new Position($1->position(), $4->position());
        auto assign = new AssignExpressionNode(
            new Position($2->position(), $4->position()),
            $2,
            $4
        );
        $$ = new VariableDeclarationNode(pos, $1, assign, false);
    }

    | shared FN id ASSIGN function {
        auto p1 = $1->position() == nullptr ? $2->position() : $1->position();
        Position* pos = new Position(p1, $5->position());
        auto assign = new AssignExpressionNode(
            new Position($3->position(), $5->position()),
            $3,
            $5
        );
        $$ = new VariableDeclarationNode(pos, $5->typeNode(), assign, $1->shared());
        delete $1;
    }

    | typeid id ASSIGN DEFER callExpression {
        auto pos = new Position($1->position(), $5->position());
        auto assign = new AssignExpressionNode(
            new Position($2->position(), $5->position()),
            $2,
            new DeferCallExpressionNode($5->position(), $5)
        );
        $$ = new VariableDeclarationNode(pos, $1, assign, false);
    }

    | SHARED typeid id ASSIGN DEFER callExpression {
        auto pos = new Position($1->position(), $6->position());
        auto assign = new AssignExpressionNode(
            new Position($3->position(), $6->position()),
            $3,
            $6
        );
        $$ = new VariableDeclarationNode(pos, $2, assign, true);
    }

typeBody :
    typeBody declaration SEMICOLON {
        $$ = $1;
        $$->push_back(useref($2));
    }

    | typeBody typeid id SEMICOLON {
        $$ = $1;
        auto pos = new Position($2->position(), $3->position());
        $$->push_back(useref(new UninitializedVariableDeclarationNode(pos, $2, $3)));
    }

    | typeBody funcConstr SEMICOLON {
        $$ = $1;
        $$->push_back(useref($2));
    }

    | typeBody USE idList SEMICOLON {
        $$ = $1;
        $$->push_back(useref(new UseNode(
            new Position($2->position(), $4->position()), $3
        )));
    }

    | declaration SEMICOLON {
        $$ = new std::vector<DeclarationNode*>();
        $$->push_back(useref($1));
    }

    | typeid id SEMICOLON {
        $$ = new std::vector<DeclarationNode*>();
        auto pos = new Position($1->position(), $2->position());
        $$->push_back(useref(new UninitializedVariableDeclarationNode(pos, $1, $2)));
    }

    | funcConstr SEMICOLON {
        $$ = new std::vector<DeclarationNode*>();
        $$->push_back(useref($1));
    }

    | USE idList SEMICOLON {
        $$ = new std::vector<DeclarationNode*>();
        $$->push_back(useref(new UseNode(
            new Position($1->position(), $3->position()), $2
        )));
    }

assignment :
    assignable ASSIGN expression {
        Position* pos = new Position($1->position(), $3->position());
        $$ = new AssignExpressionNode(pos, $1, $3);
    }

    | assignable ADDASSIGN expression {
        Position* pos = new Position($1->position(), $3->position());
        auto r = new AddNode(pos, $1, $3);
        $$ = new AssignExpressionNode(pos, $1->copy(), r);
    }

    | assignable MULTIPLYASSIGN expression {
        Position* pos = new Position($1->position(), $3->position());
        auto r = new MultiplyNode(pos, $1, $3);
        $$ = new AssignExpressionNode(pos, $1->copy(), r);
    }

    | assignable SUBTRACTASSIGN expression {
        Position* pos = new Position($1->position(), $3->position());
        auto r = new SubtractNode(pos, $1, $3);
        $$ = new AssignExpressionNode(pos, $1->copy(), r);
    }

    | assignable DIVIDEASSIGN expression {
        Position* pos = new Position($1->position(), $3->position());
        auto r = new DivideNode(pos, $1, $3);
        $$ = new AssignExpressionNode(pos, $1->copy(), r);
    }

    | assignable MODULUSASSIGN expression {
        Position* pos = new Position($1->position(), $3->position());
        auto r = new ModulusNode(pos, $1, $3);
        $$ = new AssignExpressionNode(pos, $1->copy(), r);
    }

    | assignable POWERASSIGN expression {
        Position* pos = new Position($1->position(), $3->position());
        auto r = new PowerNode(pos, $1, $3);
        $$ = new AssignExpressionNode(pos, $1->copy(), r);
    }

    | assignable ANDASSIGN expression {
        Position* pos = new Position($1->position(), $3->position());
        auto r = new AndNode(pos, $1, $3);
        $$ = new AssignExpressionNode(pos, $1->copy(), r);
    }

    | assignable ORASSIGN expression {
        Position* pos = new Position($1->position(), $3->position());
        auto r = new OrNode(pos, $1, $3);
        $$ = new AssignExpressionNode(pos, $1->copy(), r);
    }

assignable
    : lval { $$ = $1; }
    | lval LBRACKET RBRACKET {
        $$ = new EnumerableAppendNode(new Position($1->position(), $2->position()), $1);
    }

lval :
    id {
        $$ = $1;
    }

    | lval LBRACE id RBRACE {
        Position* pos = new Position($1->position(), $4->position());
        $$ = new MapAccessNode(pos, $1, $3);
    }

    | callExpression LBRACE id RBRACE {
        Position* pos = new Position($1->position(), $4->position());
        $$ = new MapAccessNode(pos, $1, $3);
    }

    | lval LBRACKET expression RBRACKET {
        Position* pos = new Position($1->position(), $4->position());
        $$ = new EnumerableAccessNode(pos, $1, $3);
    }

    | callExpression LBRACKET expression RBRACKET {
        Position* pos = new Position($1->position(), $4->position());
        $$ = new EnumerableAccessNode(pos, $1, $3);
    }

    | classAccess {
        $$ = $1;
    }

classAccess :
    lval DOT id {
        $$ = new ClassAccessNode(new Position($1->position(), $3->position()), $1, $3);
    }

    | callExpression DOT id {
        $$ = new ClassAccessNode(new Position($1->position(), $3->position()), $1, $3);
    }

id :
    ID {
        $$ = new IdentifierNode($1->position(), $1->identifier());
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
    }

    | NUMBER {
        auto t = Type::Primitive::of(Type::Intrinsic::NUMBER);
        $$ = new TypeLiteral($1->position(), t);
    }

    | SOCKET {
        auto t = new Type::Resource(Type::Opaque::of("PROLOGUE::SOCKET"));
        $$ = new TypeLiteral($1->position(), t);
    }

    | SOCKETCONNECTION {
        auto t = new Type::Resource(Type::Opaque::of("PROLOGUE::SOCKET::CONNECTION"));
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

    | TYPE {
        auto t = Type::Primitive::of(Type::Intrinsic::TYPE);
        $$ = new TypeLiteral($1->position(), t);
    }

    | ENUMERABLE LARROW typeid RARROW {
        Position* pos = new Position($1->position(), $4->position());
        $$ = new TypeLiteral(pos, new Type::Enumerable($3->value()));
        delete $3;
    }

    | MAP LARROW typeid RARROW {
        Position* pos = new Position($1->position(), $4->position());
        $$ = new TypeLiteral(pos, new Type::Map($3->value()));
        delete $3;
    }

    | typeid ARROW typeid {
        Position* pos = new Position($1->position(), $3->position());
        $$ = new TypeLiteral(pos, new Type::Lambda1($1->value(), $3->value()));
        delete $1; delete $3;
    }

    | ARROW typeid {
        Position* pos = new Position($1->position(), $2->position());
        $$ = new TypeLiteral(pos, new Type::Lambda0($2->value()));
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

        auto fn = new FunctionNode(pos, new TypeLiteral(typepos, t), $2);
        fn->assumeAndReduceStatements($8->reduceToStatements());
        $$ = fn;
        delete $5;
    }

    | LPAREN RPAREN COLON typeid FNDEF LBRACE statements RBRACE {
        Position* pos = new Position($1->position(), $8->position());

        auto fn = new FunctionNode(
            pos, new TypeLiteral($4->position(), new Type::Lambda0($4->value())), new FormalList());
        fn->assumeAndReduceStatements($7->reduceToStatements());
        $$ = fn;
        delete $4;
    }

    | LPAREN formals RPAREN COLON typeid FNDEF expressionF {
        Position* pos = new Position($1->position(), $7->position());
        Position* typepos = new Position($1->position(), $5->position());

        Type::Type* t = $5->value();

        for ( auto i = $2->rbegin(); i != $2->rend(); ++i ) {
            t = new Type::Lambda1((*i).first->value(), t);
        }

        auto fn = new FunctionNode(pos, new TypeLiteral(typepos, t), $2);
        auto retstmt = new StatementList();
        retstmt->push_back(useref(new ReturnStatementNode($7->position()->copy(), $7)));
        fn->assumeAndReduceStatements(retstmt);
        $$ = fn;
        delete $5;
    }

    | LPAREN RPAREN COLON typeid FNDEF expressionF {
        Position* pos = new Position($1->position(), $6->position());

        auto fn = new FunctionNode(
            pos, new TypeLiteral($4->position()->copy(), new Type::Lambda0($4->value())), new FormalList());
        auto retstmt = new StatementList();
        retstmt->push_back(useref(new ReturnStatementNode($6->position()->copy(), $6)));
        fn->assumeAndReduceStatements(retstmt);
        $$ = fn;
        delete $4;
    }

funcConstr :
    CONSTRUCTOR LPAREN RPAREN FNDEF LBRACE statements RBRACE {
        auto pos = new Position($1->position(), $7->position());
        auto pos2 = new Position($2->position(), $7->position());
        auto t = new TypeLiteral($1->position(), new Type::Lambda0(Type::Primitive::of(Type::Intrinsic::VOID)));
        auto func = new FunctionNode(pos2, t, new FormalList());
        func->assumeAndReduceStatements($6->reduceToStatements());
        $$ = new ConstructorNode(pos, func, new std::vector<ExpressionNode*>());
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
        $$ = new ConstructorNode(pos, func, new std::vector<ExpressionNode*>());
    }

    | CONSTRUCTOR LPAREN RPAREN COLON callExpressions FNDEF LBRACE statements RBRACE {
        auto pos = new Position($1->position(), $9->position());
        auto pos2 = new Position($2->position(), $9->position());
        auto t = new TypeLiteral($1->position(), new Type::Lambda0(Type::Primitive::of(Type::Intrinsic::VOID)));
        auto func = new FunctionNode(pos2, t, new FormalList());
        func->assumeAndReduceStatements($8->reduceToStatements());
        $$ = new ConstructorNode(pos, func, $5);
    }

    | CONSTRUCTOR LPAREN formals RPAREN COLON callExpressions FNDEF LBRACE statements RBRACE {
        Position* pos = new Position($1->position(), $10->position());
        Position* pos2 = new Position($2->position(), $10->position());
        Position* typepos = new Position($1->position(), $4->position());

        Type::Type* t = Type::Primitive::of(Type::Intrinsic::VOID);

        for ( auto i = $3->rbegin(); i != $3->rend(); ++i ) {
            t = new Type::Lambda1((*i).first->value(), t);
        }

        auto func = new FunctionNode(pos2, new TypeLiteral(typepos, t), $3);
        func->assumeAndReduceStatements($9->reduceToStatements());
        $$ = new ConstructorNode(pos, func, $6);
    }

callExpressions
    : callExpression {
        $$ = new std::vector<ExpressionNode*>();
        $$->push_back(useref($1));
    }

    | callExpressions COMMA callExpression {
        $$ = $1;
        $$->push_back(useref($3));
    }

expression :
    LBRACE RBRACE OF typeid {
        Position* pos = new Position($1->position(), $4->position());
        std::vector<MapStatementNode*>* body = new std::vector<MapStatementNode*>();
        $$ = new MapNode(pos, body, new TypeLiteral($4->position(), new Type::Map($4->value())));
        delete $4;
    }

    | LBRACE mapStatements RBRACE {
        Position* pos = new Position($1->position(), $3->position());
        $$ = new MapNode(pos, $2);
    }

    | LBRACE typeBody RBRACE {
        Position* pos = new Position($1->position(), $3->position());
        $$ = new TypeBodyNode(pos, $2);
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
        delete $4;
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

    | expressionF ROOT expressionF {
        Position* pos = new Position($1->position(), $3->position());
        $$ = new NthRootNode(pos, $1, $3);
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

    | ROOT term {
        Position* pos = new Position($1->position(), $2->position());
        $$ = new NthRootNode(pos, new NumberLiteralExpressionNode($1->position(), 2), $2);
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
    }

    | NUMBERLITERAL {
        $$ = new NumberLiteralExpressionNode($1->position(), $1->value());
    }

    | TTRUE {
        $$ = new BooleanLiteralExpressionNode($1->position(), true);
    }

    | TFALSE {
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
        actuals->push_back(useref($1));
        $$ = actuals;
    }

    | actuals COMMA expression {
        $$ = $1;
        $$->push_back(useref($3));
    }

formals :
    id COLON typeid {
        $$ = new std::vector<std::pair<TypeLiteral*, IdentifierNode*>>();
        auto t = std::pair<TypeLiteral*, IdentifierNode*>(useref($3), useref($1));
        $$->push_back(t);
    }

    | formals COMMA id COLON typeid {
        $$ = $1;
        auto t = std::pair<TypeLiteral*, IdentifierNode*>(useref($5), useref($3));
        $$->push_back(t);
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
    }



mapStatement :
    id COLON expression {
        Position* pos = new Position($1->position(), $3->position());
        $$ = new MapStatementNode(pos, $1, $3);
    }



callExpression :
    callable LPAREN RPAREN {
        Position* pos = new Position($1->position(), $3->position());
        $$ = new CallExpressionNode(pos, $1, new std::vector<ExpressionNode*>());
    }

    | callable LPAREN actuals RPAREN {
        Position* pos = new Position($1->position(), $4->position());
        $$ = new CallExpressionNode(pos, $1, $3);
    }

callable :
    callExpression { $$ = $1; }
    | iifExpression { $$ = $1; }
    | lval { $$ = $1; }

iifExpression :
    LPAREN expression RPAREN LPAREN RPAREN {
        Position* pos = new Position($1->position(), $5->position());
        $$ = new CallExpressionNode(pos, $2, new std::vector<ExpressionNode*>());
    }

    | LPAREN expression RPAREN LPAREN actuals RPAREN {
        Position* pos = new Position($1->position(), $6->position());
        $$ = new CallExpressionNode(pos, $2, $5);
    }

%%

void swarmc::Lang::Parser::error(const std::string& msg){
    parseLog->error(msg);
}

void swarmc::Lang::Parser::report_syntax_error(const swarmc::Lang::Parser::context& ctx) const {
    auto found = symbol_name(ctx.token());
    swarmc::Lang::Parser::symbol_kind_type yyarg[3];
    int amt = ctx.expected_tokens(yyarg, 3);

    std::string sum = "";
    for ( auto i = 0; i < amt; i++ ) {
        std::string expected(symbol_name(yyarg[i]));
        sum += ", " + expected;
    }

    auto tkPos = scanner.currentPos();
    GC_LOCAL_REF(tkPos)

    if ( amt ) {
        parseLog->error(s(tkPos) + " Unexpected " + s(found) + ", expected: {" + sum.substr(2) + "}");
    } else {
        parseLog->error(s(tkPos) + " Unexpected " + s(found));
    }
}