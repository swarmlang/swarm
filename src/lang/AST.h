#ifndef SWARMC_AST_H
#define SWARMC_AST_H

#include <vector>
#include <ostream>
#include <algorithm>
#include <cassert>
#include "../shared/nslib.h"
#include "Position.h"
#include "Type.h"
#include "SymbolTable.h"
#include "TypeTable.h"

using namespace nslib;

namespace swarmc::Lang {
namespace Walk {
    class NameAnalysisWalk;
    class TypeAnalysisWalk;
    class DeSerializeWalk;
    class SymbolScrubWalk;
}

    class StatementNode;
    class ExpressionNode;
    class MapStatementNode;
    class IntegerLiteralExpressionNode;
    class TypeLiteral;
    class IdentifierNode;

    using StatementList = std::vector<StatementNode*>;
    using ExpressionList = std::vector<ExpressionNode*>;
    using MapBody = std::vector<MapStatementNode*>;
    using FormalList = std::vector<std::pair<TypeLiteral*, IdentifierNode*>>;

    /** Base class for all AST nodes. */
    class ASTNode : public IStringable, public IUsesConsole {
    public:
        explicit ASTNode(Position* pos) : IUsesConsole(), _pos(pos) {};

        ~ASTNode() override {
            delete _pos;
        };

        /** Implements IStringable. */
        std::string toString() const override = 0;

        virtual ASTNode* copy() const = 0;

        /** Get the node's Position instance. */
        virtual Position* position() const {
            return _pos;
        };

        virtual std::string getName() const = 0;

        virtual bool isStatement() const {
            return false;
        }

        virtual bool isResource() const {
            return false;
        }

        virtual bool isExpression() const {
            return false;
        }

        virtual bool isValue() const {
            return false;
        }

        virtual bool isLVal() const {
            return false;
        }

        virtual bool isType() const {
            return false;
        }

        virtual bool isBlock() const {
            return false;
        }

    private:
        Position* _pos = nullptr;
    };

    /** AST node representing a statement that can occur in the root of a program. */
    class StatementNode : public ASTNode {
    public:
        StatementNode(Position* pos) : ASTNode(pos) {}
        virtual ~StatementNode() {}

        virtual bool isStatement() const override {
            return true;
        }

        virtual StatementNode* copy() const override = 0;
    };

    /** AST node representing the root of the program. */
    class ProgramNode final : public ASTNode {
    public:
        ProgramNode() : ASTNode(new Position(0, 0, 0, 0)) {
            _body = new std::vector<StatementNode*>();
        }

        virtual ~ProgramNode() { /*cannot implement because of reduceToStatements */ }

        virtual std::string getName() const override {
            return "ProgramNode";
        }

        const StatementList* body() const {
            return _body;
        }

        /**
         * Demote this program to a list of statements.
         * This destructs the current instance, so only the
         * returned StatementList is valid.
         */
        StatementList* reduceToStatements() {
            StatementList* list = _body;
            delete this;
            return list;
        }

        /** Append a statement to the program. */
        virtual void pushStatement(StatementNode* statement) {
            _body->push_back(statement);
        }

        virtual std::string toString() const override {
            return "ProgramNode<#body: " + std::to_string(_body->size()) + ">";
        }

        virtual ProgramNode* copy() const override;

    protected:
        /** The statements that comprise the program. */
        StatementList* _body;
    };

    class StatementListWrapper {
    public:
        StatementListWrapper() {
            _body = new StatementList();
        }

        virtual ~StatementListWrapper() {
            for ( auto stmt : *_body ) {
                delete stmt;
            }

            delete _body;
        }

        /** Push a new statement to the end of the body. */
        void pushStatement(StatementNode* statement) {
            _body->push_back(statement);
        }

        /**
         * Given a list of statements, concatentate them onto the
         * body and delete the original list container.
         */
        void assumeAndReduceStatements(StatementList* body) {
            for ( auto statement : *body ) {
                pushStatement(statement);
            }

            delete body;
        }

        StatementList* body() const {
            return _body;
        }

        StatementList* copyBody() const {
            auto other = new StatementList();
            for ( auto stmt : *body() ) other->push_back(stmt->copy());
            return other;
        }

    protected:
        StatementList* _body;
    };

    class ContinueNode final : public StatementNode {
    public:
        ContinueNode(Position* pos) : StatementNode(pos) {}
        virtual ~ContinueNode() {}

        std::string toString() const override {
            return "ContinueNode<>";
        }

        std::string getName() const override {
            return "ContinueNode";
        }

        virtual ContinueNode* copy() const override {
            return new ContinueNode(position()->copy());
        }
    };

    class BreakNode : public StatementNode {
    public:
        BreakNode(Position* pos) : StatementNode(pos) {}
        virtual ~BreakNode() {}

        std::string toString() const override {
            return "BreakNode<>";
        }

        std::string getName() const override {
            return "BreakNode";
        }

        virtual BreakNode* copy() const override {
            return new BreakNode(position()->copy());
        }
    };


    /** AST node representing code that evaluates to a value. */
    class ExpressionNode : public ASTNode {
    public:
        ExpressionNode(Position* pos) : ASTNode(pos) {}
        virtual ~ExpressionNode() {}

        virtual bool isExpression() const override {
            return true;
        }

        virtual ExpressionNode* copy() const override = 0;

        virtual const Type::Type* type() const = 0;
    };


    /** Non-source node representing the UNIT type instance. */
    class UnitNode final : public ExpressionNode {
    public:
        UnitNode(Position* pos) : ExpressionNode(pos) {}

        virtual UnitNode* copy() const override {
            return new UnitNode(position()->copy());
        }

        const Type::Type* type() const override {
            return Type::Primitive::of(Type::Intrinsic::UNIT);
        }

        std::string toString() const override {
            return "UnitNode<>";
        }

        std::string getName() const override {
            return "UnitNode";
        }
    };


    /**
     * Base class for ExpressionNodes that can ALSO appear as top-level statements.
     * For example, call expressions can also be statements. So, rather than have
     * a separate call statement and call expression, we combine them using this
     * class.
     */
    class StatementExpressionNode : public ExpressionNode {
    public:
        StatementExpressionNode(Position* pos) : ExpressionNode(pos) {}
        virtual ~StatementExpressionNode() {}
        virtual StatementExpressionNode* copy() const override = 0;
    };


    /**
     * AST node representing a statement of an expression. Only works for expressions
     * that derive StatementExpressionNode.
     */
    class ExpressionStatementNode final : public StatementNode {
    public:
        ExpressionStatementNode(Position* pos, StatementExpressionNode* exp) : StatementNode(pos), _exp(exp) {}
        virtual ~ExpressionStatementNode() {
            delete _exp;
        }

        virtual std::string getName() const override {
            return "ExpressionStatementNode";
        }

        virtual std::string toString() const override {
            return "ExpressionStatementNode<" + _exp->toString() + ">";
        }

        StatementExpressionNode* expression() const {
            return _exp;
        }

        virtual ExpressionStatementNode* copy() const override {
            return new ExpressionStatementNode(position()->copy(), _exp->copy());
        }

    protected:
        StatementExpressionNode* _exp;
    };


    /** Base class for AST nodes that can have a value assigned to them. */
    class LValNode : public ExpressionNode {
    public:
        LValNode(Position* pos) : ExpressionNode(pos) {}
        virtual ~LValNode() {}

        bool isLVal() const override {
            return true;
        }

        virtual bool shared() const = 0;

        virtual SemanticSymbol* lockable() const = 0;

        LValNode* copy() const override = 0;

        const Type::Type* type() const override = 0;
    };


    /** AST node representing an identifier. */
    class IdentifierNode final : public LValNode {
    public:
        IdentifierNode(Position* pos, std::string name) : LValNode(pos), _name(name) {}
        const std::string name() { return _name; }

        virtual std::string getName() const override {
            return "IdentifierNode";
        }

        virtual std::string toString() const override {
            return "IdentifierNode<name: " + _name + ">";
        }

        /** Get the semantic symbol associated with this identifier in its scope. */
        SemanticSymbol* symbol() const {
            return _symbol;
        }

        SemanticSymbol* lockable() const override {
            return symbol();
        }

        void overrideSymbol(SemanticSymbol* sym) {
            _symbol = sym;
        }

        virtual bool shared() const override {
            if (_symbol == nullptr) {
                throw Errors::SwarmError("Attempt to get sharedness of symbol-less identifier: " + _name);
            }

            return _symbol->type()->shared();
        }

        virtual IdentifierNode* copy() const override {
            auto other = new IdentifierNode(position()->copy(), _name);
            other->_symbol = _symbol;
            return other;
        }

        virtual const Type::Type* type() const override {
            return _symbol->type();
        }

    protected:
        std::string _name;
        SemanticSymbol* _symbol = nullptr;

        friend class Walk::NameAnalysisWalk;
        friend class Walk::TypeAnalysisWalk;
        friend class Walk::DeSerializeWalk;
        friend class Walk::SymbolScrubWalk;
    };

    /** Node for accessing data from a map */
    class MapAccessNode final : public LValNode {
    public:
        MapAccessNode(Position* pos, LValNode* path, IdentifierNode* end) : LValNode(pos), _path(path), _end(end) {}
        virtual ~MapAccessNode() {
            delete _path;
            delete _end;
        }

        virtual std::string getName() const override {
            return "MapAccessNode";
        }

        virtual std::string toString() const override {
            return "MapAccessNode<path: " + _path->toString() + " id: " + _end->name() + ">";
        }

        LValNode* path() const {
            return _path;
        }

        IdentifierNode* end() const {
            return _end;
        }

        SemanticSymbol* lockable() const override;

        virtual bool shared() const override {
            return _path->shared();
        }

        virtual MapAccessNode* copy() const override {
            return new MapAccessNode(position()->copy(), _path->copy(), _end->copy());
        }

        virtual const Type::Type* type() const override {
            auto pathType = _path->type();
            assert(pathType->intrinsic() == Type::Intrinsic::MAP);
            return ((Type::Map*) pathType)->values();
        }
    private:
        LValNode* _path;
        IdentifierNode* _end;
    };

    class TypeLiteral final : public ExpressionNode {
    public:
        TypeLiteral(Position* pos, swarmc::Type::Type* type) : ExpressionNode(pos), _type(type) {}

        TypeLiteral* copy() const override {
            return new TypeLiteral(position()->copy(), _type);
        }

        std::string getName() const override {
            return "TypeLiteral";
        }

        const Type::Type* type() const override {
            return Type::Primitive::of(Type::Intrinsic::TYPE);
        }

        Type::Type* value() const {
            return _type;
        }

        std::string toString() const override {
            if ( _type->shared() ) {
                return "Type<SHARED "+ _type->toString() + ">";
            }
            return "Type<" + _type->toString() + ">";
        }

        void setShared(bool shared) const {
            _type->_shared = shared;
        }
    protected:
        swarmc::Type::Type* _type;
    };

    /** AST node referencing a literal boolean value. */
    class BooleanLiteralExpressionNode final : public ExpressionNode {
    public:
        BooleanLiteralExpressionNode(Position* pos, const bool val) : ExpressionNode(pos), _val(val) {}

        virtual std::string getName() const override {
            return "BooleanLiteralExpressionNode";
        }

        virtual std::string toString() const override {
            return "BoolLiteralNode<of: " + std::to_string(_val) + ">";
        }

        /** Get the value of the literal expression. */
        virtual bool value() const {
            return _val;
        }

        virtual bool isValue() const override {
            return true;
        }

        virtual BooleanLiteralExpressionNode* copy() const override {
            return new BooleanLiteralExpressionNode(position()->copy(), _val);
        }

        virtual const Type::Type* type() const override {
            return Type::Primitive::of(Type::Intrinsic::BOOLEAN);
        }

    private:
        const bool _val;
    };


    /** Base class for AST nodes declaring things in scope. */
    class DeclarationNode : public StatementNode {
    public:
        DeclarationNode(Position* pos) : StatementNode(pos) {}
        virtual ~DeclarationNode() {}
        virtual DeclarationNode* copy() const override = 0;
    };


    /** AST node that declares a variable and sets its value to some expression's result. */
    class VariableDeclarationNode final : public DeclarationNode {
    public:
        /**
         * @param pos - the position of the code
         * @param type - the TypeLiteral declaring the variable
         * @param id - the name of the variable
         * @param value - the initial value of the variable
         */
        VariableDeclarationNode(Position* pos, TypeLiteral* type, IdentifierNode* id, ExpressionNode* value)
            : DeclarationNode(pos), _type(type), _id(id), _value(value) {}

        virtual ~VariableDeclarationNode() {
            delete _type;
            delete _id;
            delete _value;
        }

        virtual std::string getName() const override {
            return "VariableDeclarationNode";
        }

        virtual std::string toString() const override {
            return "VariableDeclarationNode<name: " + _id->name() + ">";
        }

        IdentifierNode* id() const {
            return _id;
        }

        ExpressionNode* value() const {
            return _value;
        }

        TypeLiteral* typeNode() const {
            return _type;
        }

        virtual VariableDeclarationNode* copy() const override {
            return new VariableDeclarationNode(
                position()->copy(),
                _type->copy(),
                _id->copy(),
                _value->copy()
            );
        }

    protected:
        TypeLiteral* _type;
        IdentifierNode* _id;
        ExpressionNode* _value;
    };

    /** AST node representing an assignment of a value to an lval. */
    class AssignExpressionNode : public StatementExpressionNode {
    public:
        AssignExpressionNode(Position* pos, LValNode* dest, ExpressionNode* value) : StatementExpressionNode(pos), _dest(dest), _value(value) {}
        virtual ~AssignExpressionNode() {
            delete _dest;
            delete _value;
        }

        virtual std::string toString() const override {
            return "AssignExpressionNode<lval: " + _dest->toString() + ">";
        }

        virtual std::string getName() const override {
            return "AssignExpressionNode";
        }

        virtual LValNode* dest() const {
            return _dest;
        }

        virtual ExpressionNode* value() const {
            return _value;
        }

        virtual AssignExpressionNode* copy() const override {
            return new AssignExpressionNode(position()->copy(), _dest->copy(), _value->copy());
        }

        virtual const Type::Type* type() const override {
            return _value->type();
        }

    protected:
        LValNode* _dest;
        ExpressionNode* _value;
    };

    class ReturnStatementNode : public StatementNode {
    public:
        ReturnStatementNode(Position* pos, ExpressionNode* value) : StatementNode(pos), _value(value) {}
        virtual ~ReturnStatementNode() {
            if ( _value != nullptr ) delete _value;
        }

        virtual std::string toString() const override {
            if (_value == nullptr) {
                return "ReturnStatementNode<>";
            }
            return "ReturnStatementNode<lval: " + _value->toString() + ">";
        }

        virtual std::string getName() const override {
            return "ReturnStatementNode";
        }

        virtual ReturnStatementNode* copy() const override {
            return new ReturnStatementNode(position()->copy(), _value->copy());
        }

        ExpressionNode* value() const {
            return _value;
        }
    private:
        ExpressionNode* _value;
    };

    class FunctionNode : public ExpressionNode, public StatementListWrapper {
    public:
        FunctionNode(Position* pos, TypeLiteral* type, FormalList* formals)
            : ExpressionNode(pos), _type(type), _formals(formals) {
        }

        virtual ~FunctionNode() {
            for ( auto f : *_formals ) {
                delete f.first;
                delete f.second;
            }
            delete _formals;
        }

        virtual std::string getName() const override {
            return "FunctionNode";
        }

        virtual std::string toString() const override {
            return "FunctionNode<type: " + _type->toString() + ">";
        }

        TypeLiteral* typeNode() const {
            return _type;
        }

        FormalList* formals() const {
            return _formals;
        }

        virtual const Type::Type* type() const override {
            return _type->value();
        }

        virtual FunctionNode* copy() const override {
            auto formals = new FormalList();
            for ( auto f : *_formals ) {
                formals->push_back(std::pair<TypeLiteral*, IdentifierNode*>(f.first->copy(), f.second->copy()));
            }

            auto fn = new FunctionNode(position()->copy(), _type->copy(), formals);
            fn->_body = copyBody();
            return fn;
        }
    protected:
        TypeLiteral* _type;
        FormalList* _formals;
    };

    /** AST node representing a call to a function. */
    class CallExpressionNode final : public StatementExpressionNode {
    public:
        CallExpressionNode(Position* pos, IdentifierNode* id, std::vector<ExpressionNode*>* args) : StatementExpressionNode(pos), _id(id), _args(args) {}
        virtual ~CallExpressionNode() {
            delete _id;
            for ( auto arg : *_args ) delete arg;
            delete _args;
        }

        virtual std::string getName() const override {
            return "CallExpressionNode";
        }

        virtual std::string toString() const override {
            return "CallExpressionNode<id: " + _id->name() + ", #args: " + std::to_string(_args->size()) + ">";
        }

        IdentifierNode* id() const {
            return _id;
        }

        std::vector<ExpressionNode*>* args() const {
            return _args;
        }

        virtual CallExpressionNode* copy() const override {
            auto args = new std::vector<ExpressionNode*>;
            for ( auto arg : *_args ) {
                args->push_back(arg->copy());
            }

            return new CallExpressionNode(position()->copy(), _id->copy(), args);
        }

        virtual const Type::Type* type() const override {
            auto fnType = _id->type();
            for ( size_t i = 0; i < _args->size(); i++ ) {
                assert(fnType->isCallable());
                fnType = ((Type::Lambda*) fnType)->returns();
            }
            return fnType;
        }

    protected:
        IdentifierNode* _id;
        std::vector<ExpressionNode*>* _args;
    };

    /** AST node representing a call to a function. */
    class IIFExpressionNode final : public StatementExpressionNode {
    public:
        IIFExpressionNode(Position* pos, ExpressionNode* exp, std::vector<ExpressionNode*>* args) : StatementExpressionNode(pos), _expression(exp), _args(args) {}
        virtual ~IIFExpressionNode() {
            delete _expression;
            for ( auto arg : *_args ) delete arg;
            delete _args;
        }

        virtual std::string getName() const override {
            return "IIFExpressionNode";
        }

        virtual std::string toString() const override {
            return "IIFExpressionNode<func: " + _expression->toString() + ", #args: " + std::to_string(_args->size()) + ">";
        }

        ExpressionNode* expression() const {
            return _expression;
        }

        std::vector<ExpressionNode*>* args() const {
            return _args;
        }

        virtual IIFExpressionNode* copy() const override {
            auto args = new std::vector<ExpressionNode*>;
            for ( auto arg : *_args ) {
                args->push_back(arg->copy());
            }

            return new IIFExpressionNode(position()->copy(), _expression->copy(), args);
        }

        virtual const Type::Type* type() const override {
            auto fnType = _expression->type();
            for ( size_t i = 0; i < _args->size(); i++ ) {
                assert(fnType->isCallable());
                fnType = ((Type::Lambda*) fnType)->returns();
            }
            return fnType;
        }

    protected:
        ExpressionNode* _expression;
        std::vector<ExpressionNode*>* _args;
    };


    /** Base class for expressions of two operands. */
    class BinaryExpressionNode : public ExpressionNode {
    public:
        BinaryExpressionNode(Position* pos, ExpressionNode* left, ExpressionNode* right) : ExpressionNode(pos), _left(left), _right(right) {}
        virtual ~BinaryExpressionNode() {
            delete _left;
            delete _right;
        }

        ExpressionNode* left() const {
            return _left;
        }

        ExpressionNode* right() const {
            return _right;
        }

        virtual BinaryExpressionNode* copy() const override = 0;
    protected:
        ExpressionNode* _left;
        ExpressionNode* _right;
    };


    /** Base class for expression nodes over static types. */
    class PureBinaryExpressionNode : public BinaryExpressionNode {
    public:
        PureBinaryExpressionNode(Position* pos, ExpressionNode* left, ExpressionNode* right) : BinaryExpressionNode(pos, left, right) {}
        virtual ~PureBinaryExpressionNode() {}

        virtual const Type::Type* leftType() const = 0;

        virtual const Type::Type* rightType() const = 0;

        virtual const Type::Type* resultType() const = 0;

        virtual PureBinaryExpressionNode* copy() const override = 0;

        virtual const Type::Type* type() const override {
            return resultType();
        }
    };


    /** Base class for binary nodes that take bool -> bool -> bool */
    class PureBooleanBinaryExpressionNode : public PureBinaryExpressionNode {
    public:
        PureBooleanBinaryExpressionNode(Position* pos, ExpressionNode* left, ExpressionNode* right) : PureBinaryExpressionNode(pos, left, right) {}
        virtual ~PureBooleanBinaryExpressionNode() {}

        virtual const Type::Type* leftType() const override {
            return Type::Primitive::of(Type::Intrinsic::BOOLEAN);
        }

        virtual const Type::Type* rightType() const override {
            return Type::Primitive::of(Type::Intrinsic::BOOLEAN);
        }

        virtual const Type::Type* resultType() const override {
            return Type::Primitive::of(Type::Intrinsic::BOOLEAN);
        }

        virtual PureBooleanBinaryExpressionNode* copy() const override = 0;
    };


    /** Base class for binary nodes that take num -> num -> num */
    class PureNumberBinaryExpressionNode : public PureBinaryExpressionNode {
    public:
        PureNumberBinaryExpressionNode(Position* pos, ExpressionNode* left, ExpressionNode* right) : PureBinaryExpressionNode(pos, left, right) {}
        virtual ~PureNumberBinaryExpressionNode() {}

        virtual const Type::Type* leftType() const override {
            return Type::Primitive::of(Type::Intrinsic::NUMBER);
        }

        virtual const Type::Type* rightType() const override {
            return Type::Primitive::of(Type::Intrinsic::NUMBER);
        }

        virtual const Type::Type* resultType() const override {
            return Type::Primitive::of(Type::Intrinsic::NUMBER);
        }

        virtual PureNumberBinaryExpressionNode* copy() const override = 0;
    };


    /** Base class for binary nodes that take string -> string -> string */
    class PureStringBinaryExpressionNode : public PureBinaryExpressionNode {
    public:
        PureStringBinaryExpressionNode(Position* pos, ExpressionNode* left, ExpressionNode* right) : PureBinaryExpressionNode(pos, left, right) {}
        virtual ~PureStringBinaryExpressionNode() {}

        virtual const Type::Type* leftType() const override {
            return Type::Primitive::of(Type::Intrinsic::STRING);
        }

        virtual const Type::Type* rightType() const override {
            return Type::Primitive::of(Type::Intrinsic::STRING);
        }

        virtual const Type::Type* resultType() const override {
            return Type::Primitive::of(Type::Intrinsic::STRING);
        }

        virtual PureStringBinaryExpressionNode* copy() const override = 0;
    };


    /** AST node referencing boolean AND of two expressions. */
    class AndNode final : public PureBooleanBinaryExpressionNode {
    public:
        AndNode(Position* pos, ExpressionNode* left, ExpressionNode* right): PureBooleanBinaryExpressionNode(pos, left, right) {}

        virtual std::string getName() const override {
            return "AndNode";
        }

        virtual std::string toString() const override {
            return "AndNode<>";
        }

        virtual AndNode* copy() const override {
            return new AndNode(position()->copy(), _left->copy(), _right->copy());
        }
    };


    /** AST node referencing boolean OR of two expressions. */
    class OrNode final : public PureBooleanBinaryExpressionNode {
    public:
        OrNode(Position* pos, ExpressionNode* left, ExpressionNode* right): PureBooleanBinaryExpressionNode(pos, left, right) {}

        virtual std::string getName() const override {
            return "OrNode";
        }

        virtual std::string toString() const override {
            return "OrNode<>";
        }

        virtual OrNode* copy() const override {
            return new OrNode(position()->copy(), _left->copy(), _right->copy());
        }
    };


    /** AST node referencing equality comparison of two expressions. */
    class EqualsNode final : public BinaryExpressionNode {
    public:
        EqualsNode(Position* pos, ExpressionNode* left, ExpressionNode* right) : BinaryExpressionNode(pos, left, right) {}

        virtual std::string getName() const override {
            return "EqualsNode";
        }

        virtual std::string toString() const override {
            return "EqualsNode<>";
        }

        virtual EqualsNode* copy() const override {
            return new EqualsNode(position()->copy(), _left->copy(), _right->copy());
        }

        virtual const Type::Type* type() const override {
            return Type::Primitive::of(Type::Intrinsic::BOOLEAN);
        }
    };


    enum class NumberComparisonType {
        LESS_THAN,
        LESS_THAN_OR_EQUAL,
        GREATER_THAN,
        GREATER_THAN_OR_EQUAL,
    };


    /** Node type for >, >=, <, <= numeric comparisons. */
    class NumericComparisonExpressionNode final : public PureBinaryExpressionNode {
    public:
        NumericComparisonExpressionNode(Position* pos, NumberComparisonType comparisonType, ExpressionNode* left, ExpressionNode* right) : PureBinaryExpressionNode(pos, left, right), _comparisonType(comparisonType) {}

        const Type::Type* leftType() const override {
            return Type::Primitive::of(Type::Intrinsic::NUMBER);
        }

        const Type::Type* rightType() const override {
            return Type::Primitive::of(Type::Intrinsic::NUMBER);
        }

        const Type::Type* resultType() const override {
            return Type::Primitive::of(Type::Intrinsic::BOOLEAN);
        }

        PureBinaryExpressionNode* copy() const override {
            return new NumericComparisonExpressionNode(position()->copy(), _comparisonType, _left->copy(), _right->copy());
        }

        std::string getName() const override {
            return "NumericComparisonExpressionNode";
        }

        std::string toString() const override {
            return "NumericComparisonExpressionNode<type: " + comparisonTypeToString() + ">";
        }

        NumberComparisonType comparisonType() const {
            return _comparisonType;
        }

        std::string comparisonTypeToString() const {
            if ( _comparisonType == NumberComparisonType::LESS_THAN ) return "LESS_THAN";
            if ( _comparisonType == NumberComparisonType::LESS_THAN_OR_EQUAL ) return "LESS_THAN_OR_EQUAL";
            if ( _comparisonType == NumberComparisonType::GREATER_THAN ) return "GREATER_THAN";
            return "GREATER_THAN_OR_EQUAL";
        }
    protected:
        NumberComparisonType _comparisonType;
    };


    /** AST node referencing inequality comparison of two expressions. */
    class NotEqualsNode final : public BinaryExpressionNode {
    public:
        NotEqualsNode(Position* pos, ExpressionNode* left, ExpressionNode* right) : BinaryExpressionNode(pos, left, right) {}

        virtual std::string getName() const override {
            return "NotEqualsNode";
        }

        virtual std::string toString() const override {
            return "NotEqualsNode<>";
        }

        virtual NotEqualsNode* copy() const override {
            return new NotEqualsNode(position()->copy(), _left->copy(), _right->copy());
        }

        virtual const Type::Type* type() const override {
            return Type::Primitive::of(Type::Intrinsic::BOOLEAN);
        }
    };

    /** AST node referencing addition of two values. */
    class AddNode final : public PureNumberBinaryExpressionNode {
    public:
        AddNode(Position* pos, ExpressionNode* left, ExpressionNode* right) : PureNumberBinaryExpressionNode(pos, left, right) {}

        virtual std::string getName() const override {
            return "AddNode";
        }

        virtual std::string toString() const override {
            return "AddNode<>";
        }

        virtual AddNode* copy() const override {
            return new AddNode(position()->copy(), _left->copy(), _right->copy());
        }
    };

    /** AST node referencing subtraction of two values. */
    class SubtractNode final : public PureNumberBinaryExpressionNode {
    public:
        SubtractNode(Position* pos, ExpressionNode* left, ExpressionNode* right) : PureNumberBinaryExpressionNode(pos, left, right) {}

        virtual std::string getName() const override {
            return "SubtractNode";
        }

        virtual std::string toString() const override {
            return "SubtractNode<>";
        }

        virtual SubtractNode* copy() const override {
            return new SubtractNode(position()->copy(), _left->copy(), _right->copy());
        }
    };


    /** AST node referencing multiplication of two values. */
    class MultiplyNode final : public PureNumberBinaryExpressionNode {
    public:
        MultiplyNode(Position* pos, ExpressionNode* left, ExpressionNode* right) : PureNumberBinaryExpressionNode(pos, left, right) {}

        virtual std::string getName() const override {
            return "MultiplyNode";
        }

        virtual std::string toString() const override {
            return "MultiplyNode<>";
        }

        virtual MultiplyNode* copy() const override {
            return new MultiplyNode(position()->copy(), _left->copy(), _right->copy());
        }
    };

    /** AST node referencing division of two values. */
    class DivideNode final : public PureNumberBinaryExpressionNode {
    public:
        DivideNode(Position* pos, ExpressionNode* left, ExpressionNode* right) : PureNumberBinaryExpressionNode(pos, left, right) {}

        virtual std::string getName() const override {
            return "DivideNode";
        }

        virtual std::string toString() const override {
            return "DivideNode<>";
        }

        virtual DivideNode* copy() const override {
            return new DivideNode(position()->copy(), _left->copy(), _right->copy());
        }
    };


    /** AST node referencing the modulus of two values. */
    class ModulusNode final : public PureNumberBinaryExpressionNode {
    public:
        ModulusNode(Position* pos, ExpressionNode* left, ExpressionNode* right) : PureNumberBinaryExpressionNode(pos, left, right) {}

        virtual std::string getName() const override {
            return "ModulusNode";
        }

        virtual std::string toString() const override {
            return "ModulusNode<>";
        }

        virtual ModulusNode* copy() const override {
            return new ModulusNode(position()->copy(), _left->copy(), _right->copy());
        }
    };


    /** AST node referencing the exponential of two values. */
    class PowerNode final : public PureNumberBinaryExpressionNode {
    public:
        PowerNode(Position* pos, ExpressionNode* left, ExpressionNode* right) : PureNumberBinaryExpressionNode(pos, left, right) {}

        virtual std::string getName() const override {
            return "PowerNode";
        }

        virtual std::string toString() const override {
            return "PowerNode<>";
        }

        virtual PowerNode* copy() const override {
            return new PowerNode(position()->copy(), _left->copy(), _right->copy());
        }
    };


    /** AST node referencing concatenation of two strings. */
    class ConcatenateNode final : public PureStringBinaryExpressionNode {
    public:
        ConcatenateNode(Position* pos, ExpressionNode* left, ExpressionNode* right) : PureStringBinaryExpressionNode(pos, left, right) {}

        virtual std::string getName() const override {
            return "ConcatenateNode";
        }

        virtual std::string toString() const override {
            return "ConcatenateNode<>";
        }

        virtual ConcatenateNode* copy() const override {
            return new ConcatenateNode(position()->copy(), _left->copy(), _right->copy());
        }
    };

    /** Base class for expressions of one operand. */
    class UnaryExpressionNode : public ExpressionNode {
    public:
        UnaryExpressionNode(Position* pos, ExpressionNode* exp) : ExpressionNode(pos), _exp(exp) {}
        virtual ~UnaryExpressionNode() {
            delete _exp;
        }

        ExpressionNode* exp() const {
            return _exp;
        }

        virtual UnaryExpressionNode* copy() const override = 0;
    protected:
        ExpressionNode* _exp;
    };

    /** AST node referencing numeric negation of an expression */
    class NegativeExpressionNode final : public UnaryExpressionNode {
    public:
        NegativeExpressionNode(Position* pos, ExpressionNode* exp) : UnaryExpressionNode(pos, exp) {}
    
        virtual std::string getName() const override {
            return "NegativeExpressionNode";
        }

        virtual std::string toString() const override {
            return "NegativeExpressionNode<>";
        }

        virtual NegativeExpressionNode* copy() const override {
            return new NegativeExpressionNode(position()->copy(), _exp->copy());
        }

        virtual const Type::Type* type() const override {
            return Type::Primitive::of(Type::Intrinsic::NUMBER);
        }
    };

    /** AST node referencing boolean negation of an expression. */
    class NotNode final : public UnaryExpressionNode {
    public:
        NotNode(Position* pos, ExpressionNode* exp) : UnaryExpressionNode(pos, exp) {}

        virtual std::string getName() const override {
            return "NotNode";
        }

        virtual std::string toString() const override {
            return "NotNode<>";
        }

        virtual NotNode* copy() const override {
            return new NotNode(position()->copy(), _exp->copy());
        }

        virtual const Type::Type* type() const override {
            return Type::Primitive::of(Type::Intrinsic::BOOLEAN);
        }
    };


    /** AST node representing literal enumerations. */
    class EnumerationLiteralExpressionNode final : public ExpressionNode {
    public:
        EnumerationLiteralExpressionNode(Position* pos, ExpressionList* actuals) : ExpressionNode(pos), _actuals(actuals), _type(nullptr) {}
        EnumerationLiteralExpressionNode(Position* pos, ExpressionList* actuals, TypeLiteral* type) : ExpressionNode(pos), _actuals(actuals), _type(type) {}
        virtual ~EnumerationLiteralExpressionNode() {
            if ( _type != nullptr ) delete _type;
            for ( auto a : *_actuals ) delete a;
            delete _actuals;
        }

        std::string toString() const override {
            return "EnumerationLiteralExpressionNode<#actuals: " + std::to_string(_actuals->size()) + ">";
        }

        virtual std::string getName() const override {
            return "EnumerationLiteralExpressionNode";
        }

        ExpressionList* actuals() const {
            return _actuals;
        }

        virtual bool isValue() const override {
            return true;
        }

        virtual bool isEmpty() const {
            return _actuals->empty();
        }

        virtual bool hasIndex(size_t idx) const {
            return idx < _actuals->size();
        }

        virtual ExpressionNode* getIndex(size_t idx) const {
            if ( !hasIndex(idx) ) {
                // todo raise exception
            }

            return _actuals->at(idx);
        }

        virtual void setIndex(size_t idx, ExpressionNode* value) {
            if ( !hasIndex(idx) ) {
                // todo raise exception
            }

            assert(value->isValue());
            _actuals->at(idx) = value;
        }

        virtual void push(ExpressionNode* value) {
            assert(value->isValue());
            _actuals->push_back(value);
        }

        virtual void pop() {
            if ( isEmpty() ) {
                // todo raise exception
            }

            _actuals->pop_back();
        }

        virtual EnumerationLiteralExpressionNode* copy() const override {
            auto actuals = new ExpressionList;
            for ( auto actual : *_actuals ) actuals->push_back(actual->copy());

            return new EnumerationLiteralExpressionNode(position()->copy(), actuals, _type->copy());
        }

        virtual const Type::Type* type() const override {
            assert(_type != nullptr);
            return _type->value();
        }

    protected:
        ExpressionList* _actuals;
        TypeLiteral* _type;

        friend class Walk::TypeAnalysisWalk;
    };


    /** Base class for AST nodes that contain a body of statements. */
    class BlockStatementNode : public StatementNode, public StatementListWrapper {
    public:
        BlockStatementNode(Position* pos) : StatementNode(pos), StatementListWrapper() {}

        virtual ~BlockStatementNode() {}

        virtual BlockStatementNode* copy() const override = 0;

        bool isBlock() const override { return true; }
    };

    /** AST node representing an enumeration block. */
    class EnumerationStatement final : public BlockStatementNode {
    public:
        EnumerationStatement(Position* pos, LValNode* enumerable, IdentifierNode* local, IdentifierNode* index, bool shared)
            : BlockStatementNode(pos), _enumerable(enumerable), _local(local), _index(index), _shared(shared) {}

        virtual ~EnumerationStatement() {
            delete _enumerable;
            delete _local;
            if ( _index != nullptr ) delete _index;
        }

        virtual std::string getName() const override {
            return "EnumerationStatement";
        }

        virtual std::string toString() const override {
            return "EnumerationStatement<e: " + _enumerable->toString() + ", as: " + _local->name() + ", #body: " + std::to_string(_body->size()) + ">";
        }

        LValNode* enumerable() const {
            return _enumerable;
        }

        IdentifierNode* local() const {
            return _local;
        }

        IdentifierNode* index() const {
            return _index;
        }

        virtual EnumerationStatement* copy() const override {
            auto other = new EnumerationStatement(position()->copy(), _enumerable->copy(), 
                _local->copy(), _index->copy(), _shared);
            other->assumeAndReduceStatements(copyBody());
            return other;
        }
    protected:
        LValNode* _enumerable;
        IdentifierNode* _local, * _index;
        bool _shared;

        friend class Walk::NameAnalysisWalk;
        friend class Walk::SymbolScrubWalk;
    };


    /** AST node representing a with-resource block. */
    class WithStatement final : public BlockStatementNode {
    public:
        WithStatement(Position* pos, ExpressionNode* resource, IdentifierNode* local, bool shared)
            : BlockStatementNode(pos), _resource(resource), _local(local), _shared(shared) {}

        virtual ~WithStatement() {
            delete _resource;
            delete _local;
        }

        virtual std::string getName() const override {
            return "WithStatement";
        }

        virtual std::string toString() const override {
            return "WithStatement<r: " + _resource->toString() + ", as: " + _local->name() + ">";
        }

        ExpressionNode* resource() const {
            return _resource;
        }

        IdentifierNode* local() const {
            return _local;
        }

        virtual WithStatement* copy() const override {
            auto other = new WithStatement(position()->copy(), _resource->copy(), _local->copy(), _shared);
            other->assumeAndReduceStatements(copyBody());
            return other;
        }
    protected:
        ExpressionNode* _resource;
        IdentifierNode* _local;
        bool _shared;

        friend class Walk::TypeAnalysisWalk;
    };


    /** AST node referencing a single-clause conditional statement. */
    class IfStatement final : public BlockStatementNode {
    public:
        IfStatement(Position* pos, ExpressionNode* condition)
            : BlockStatementNode(pos), _condition(condition) {}

        virtual ~IfStatement() {
            delete _condition;
        }

        virtual std::string getName() const override {
            return "IfStatement";
        }

        virtual std::string toString() const override {
            return "IfStatement<f: if " + _condition->toString() + " then, #body: " + std::to_string(_body->size()) + ">";
        }

        ExpressionNode* condition() const {
            return _condition;
        }

        virtual IfStatement* copy() const override {
            auto other = new IfStatement(position()->copy(), _condition->copy());
            other->assumeAndReduceStatements(copyBody());
            return other;
        }
    protected:
        ExpressionNode* _condition;
    };


    /** AST node referencing a conditional loop. */
    class WhileStatement final : public BlockStatementNode {
    public:
        WhileStatement(Position* pos, ExpressionNode* condition)
            : BlockStatementNode(pos), _condition(condition) {}

        virtual ~WhileStatement() {
            delete _condition;
        }

        virtual std::string getName() const override {
            return "WhileStatement";
        }

        virtual std::string toString() const override {
            return "WhileStatement<w: while " + _condition->toString() + " then, #body: " + std::to_string(_body->size()) + ">";
        }

        ExpressionNode* condition() const {
            return _condition;
        }

        virtual WhileStatement* copy() const override {
            auto other = new WhileStatement(position()->copy(), _condition->copy());
            other->assumeAndReduceStatements(copyBody());
            return other;
        }
    protected:
        ExpressionNode* _condition;
    };


    /** AST node referencing one entry in a map. */
    class MapStatementNode final : public ASTNode {
    public:
        MapStatementNode(Position* pos, IdentifierNode* id, ExpressionNode* value) : ASTNode(pos), _id(id), _value(value) {}
        virtual ~MapStatementNode() {
            delete _id;
            delete _value;
        }

        virtual std::string getName() const override {
            return "MapStatementNode";
        }

        virtual std::string toString() const override {
            return "MapStatementNode<id: " + _id->name() + ">";
        }

        /**
         * Get the identifier for the key of this entry.
         * NOTE: This identifier is *NOT* a "real" identifier in the sense that it
         *       has no semantic symbol attached to it and, therefore, no type directly.
         *       Instead, its type is inferred from the type of the map.
         */
        virtual IdentifierNode* id() const {
            return _id;
        }

        ExpressionNode* value() const {
            return _value;
        }

        void setValue(ExpressionNode* value) {
            assert(value->isValue());
            _value = value;
        }

        virtual MapStatementNode* copy() const override {
            return new MapStatementNode(position()->copy(), _id->copy(), _value->copy());
        }

    protected:
        IdentifierNode* _id;
        ExpressionNode* _value;
    };


    /** AST node referencing a map literal. */
    class MapNode final : public ExpressionNode {
    public:
        MapNode(Position* pos, MapBody* body) : ExpressionNode(pos), _body(body), _type(nullptr) {}
        MapNode(Position* pos, MapBody* body, TypeLiteral* type) : ExpressionNode(pos), _body(body), _type(type) {}
        virtual ~MapNode() {
            delete _type;
            for ( auto stmt : *_body ) delete stmt;
            delete _body;
        }

        virtual std::string getName() const override {
            return "MapNode";
        }

        std::string toString() const override {
            return "MapNode<#body: " + std::to_string(_body->size()) + ">";
        }

        MapBody* body() const {
            return _body;
        }

        virtual bool isValue() const override {
            return true;
        }

        virtual bool hasKey(IdentifierNode* name) const {
            return getBodyNode(name) != nullptr;
        }

        virtual ExpressionNode* getKey(IdentifierNode* name) const {
            auto node = getBodyNode(name);
            if ( node != nullptr ) {
                return node->value();
            }

            return nullptr;
        }

        virtual void setKey(IdentifierNode* name, ExpressionNode* value) {
            assert(value->isValue());

            auto node = getBodyNode(name);
            if ( node == nullptr ) {
                node = new MapStatementNode(nullptr, name, value);
                _body->push_back(node);
            }

            node->setValue(value);
        }

        virtual MapNode* copy() const override {
            auto body = new MapBody;
            for ( auto entry : *_body ) body->push_back(entry->copy());
            return new MapNode(position()->copy(), body, _type->copy());
        }

        virtual const Type::Type* type() const override {
            assert(_type != nullptr);
            return _type->value();
        }

    protected:
        MapBody* _body;
        TypeLiteral* _type;

        virtual MapStatementNode* getBodyNode(IdentifierNode* name) const {
            for ( auto stmt : *_body ) {
                if ( stmt->id()->name() == name->name() ) {
                    return stmt;
                }
            }

            return nullptr;
        }

        friend class Walk::TypeAnalysisWalk;
    };


    /** Value containing a prologue resource reference. */
    class PrologueResourceNode : public ExpressionNode {
    public:
        PrologueResourceNode(Position* pos) : ExpressionNode(pos) {}
        virtual ~PrologueResourceNode() {}

        virtual ExpressionNode* value() = 0;

        virtual bool isOpened() const {
            return _opened;
        }

        virtual bool isValue() const override {
            return true;
        }

        virtual bool isResource() const override {
            return true;
        }

    protected:
        bool _opened = false;
    };

    /** AST node representing literal strings. */
    class StringLiteralExpressionNode final : public ExpressionNode {
    public:
        StringLiteralExpressionNode(Position* pos, std::string value) : ExpressionNode(pos), _value(value) {}
        virtual ~StringLiteralExpressionNode() {}

        virtual std::string getName() const override {
            return "StringLiteralExpressionNode";
        }

        std::string toString() const override {
            return "StringLiteralExpressionNode<#value: '" + _value + "'>";
        }

        std::string value() {
            return _value;
        }

        virtual bool isValue() const override {
            return true;
        }

        virtual StringLiteralExpressionNode* copy() const override {
            return new StringLiteralExpressionNode(position()->copy(), _value);
        }

        virtual const Type::Type* type() const override {
            return Type::Primitive::of(Type::Intrinsic::STRING);
        }
    protected:
        std::string _value;
    };

    /** AST node representing literal numbers. */
    class NumberLiteralExpressionNode : public ExpressionNode {
    public:
        NumberLiteralExpressionNode(Position* pos, double value) : ExpressionNode(pos), _value(value) {}
        virtual ~NumberLiteralExpressionNode() {}

        virtual std::string getName() const override {
            return "NumberLiteralExpressionNode";
        }

        std::string toString() const override {
            return "NumberLiteralExpressionNode<#value: " + std::to_string(_value) + ">";
        }

        double value() const {
            return _value;
        }

        virtual bool isValue() const override {
            return true;
        }

        virtual NumberLiteralExpressionNode* copy() const override {
            return new NumberLiteralExpressionNode(position()->copy(), _value);
        }

        virtual const Type::Type* type() const override {
            return Type::Primitive::of(Type::Intrinsic::NUMBER);
        }
    protected:
        double _value;
    };


    /** AST node representing integer literals */
    class IntegerLiteralExpressionNode final : public NumberLiteralExpressionNode {
        public:
        IntegerLiteralExpressionNode(Position* pos, size_t value) : NumberLiteralExpressionNode(pos, value) {}
        virtual ~IntegerLiteralExpressionNode() {}

        virtual std::string getName() const override {
            return "IntegerLiteralExpressionNode";
        }

        std::string toString() const override {
            return "IntegerLiteralExpressionNode<#value: " + std::to_string((size_t)_value) + ">";
        }

        int value() const {
            return (size_t) _value;
        }

        virtual IntegerLiteralExpressionNode* copy() const override {
            return new IntegerLiteralExpressionNode(position()->copy(), _value);
        }
    };

    /** Node for accessing data from an array */
    class EnumerableAccessNode final : public LValNode {
    public:
        EnumerableAccessNode(Position* pos, LValNode* path, ExpressionNode* index) : LValNode(pos), _path(path), _index(index) {}
        virtual ~EnumerableAccessNode() {
            delete _path;
            delete _index;
        }

        virtual std::string getName() const override {
            return "EnumerableAccessNode";
        }

        virtual std::string toString() const override {
            std::stringstream s;
            s << "EnumerableAccessNode<path: " << _path->toString() << ", index: " << _index->toString() << ">";
            return s.str();
        }

        LValNode* path() const {
            return _path;
        }

        ExpressionNode* index() const {
            return _index;
        }

        SemanticSymbol* lockable() const override;

        virtual bool shared() const override {
            return _path->shared();
        }

        virtual EnumerableAccessNode* copy() const override {
            return new EnumerableAccessNode(position()->copy(), _path->copy(), _index->copy());
        }

        virtual const Type::Type* type() const override {
            auto baseType = _path->type();
            assert(baseType->intrinsic() == Type::Intrinsic::ENUMERABLE);
            return ((Type::Enumerable*) baseType)->values();
        }
    private:
        LValNode* _path;
        ExpressionNode* _index;
    };

}


#endif
