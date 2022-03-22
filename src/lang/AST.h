#ifndef SWARMC_AST_H
#define SWARMC_AST_H

#include <vector>
#include <ostream>
#include <algorithm>
#include <assert.h>
#include "../shared/IStringable.h"
#include "../shared/util/Console.h"
#include "Position.h"
#include "Type.h"
#include "SymbolTable.h"
#include "TypeTable.h"

namespace swarmc {

namespace Runtime {
    class ISymbolValueStore;
    class InterpretWalk;
}

namespace Lang {
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

    using StatementList = std::vector<StatementNode*>;
    using ExpressionList = std::vector<ExpressionNode*>;
    using MapBody = std::vector<MapStatementNode*>;

    /** Base class for all AST nodes. */
    class ASTNode : public IStringable, public IUsesConsole {
    public:
        ASTNode(Position* pos) : IUsesConsole(), _pos(pos) {};

        virtual ~ASTNode() {}

        /** Implements IStringable. */
        virtual std::string toString() const = 0;

        virtual ASTNode* copy() const = 0;

        /** Get the node's Position instance. */
        virtual Position* position() const {
            return _pos;
        };

        virtual std::string getName() const = 0;

        virtual bool isStatement() const {
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

    private:
        Position* _pos = nullptr;
        ScopeTable* _scope = nullptr;
    };


    /** AST node representing the root of the program. */
    class ProgramNode final : public ASTNode {
    public:
        ProgramNode() : ASTNode(new Position(0, 0, 0, 0)) {
            _body = new std::vector<StatementNode*>();
        }

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


    /** AST node representing a statement that can occur in the root of a program. */
    class StatementNode : public ASTNode {
    public:
        StatementNode(Position* pos) : ASTNode(pos) {}
        virtual ~StatementNode() {}

        virtual bool isStatement() const {
            return true;
        }

        virtual StatementNode* copy() const override = 0;
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


    /** AST node representing code that evaluates to a value. */
    class ExpressionNode : public ASTNode {
    public:
        ExpressionNode(Position* pos) : ASTNode(pos) {}
        virtual ~ExpressionNode() {}

        virtual bool isExpression() const override {
            return true;
        }

        virtual ExpressionNode* copy() const override = 0;

        virtual const Type* type() const = 0;

        virtual bool equals(const ExpressionNode* other) const {
            throw Errors::SwarmError("Attempted to determine equality of non-value expressions: " + getName() + " == " + other->getName());
        }
    };


    /** Non-source node representing the UNIT type instance. */
    class UnitNode final : public ExpressionNode {
    public:
        UnitNode(Position* pos) : ExpressionNode(pos) {}

        virtual UnitNode* copy() const override {
            return new UnitNode(position()->copy());
        }

        virtual const Type* type() const {
            return PrimitiveType::of(ValueType::TUNIT);
        }

        bool equals(const ExpressionNode* other) const override {
            return other->getName() == "UnitNode" && other->type()->is(type());
        }

        virtual std::string toString() const {
            return "UnitNode<>";
        }

        virtual std::string getName() const {
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
        virtual ~ExpressionStatementNode() {}

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

        virtual bool isLVal() const {
            return true;
        }

        virtual void setValue(Runtime::ISymbolValueStore* store, ExpressionNode* node) = 0;

        virtual ExpressionNode* getValue(Runtime::ISymbolValueStore* store) = 0;

        virtual bool shared() const = 0;

        virtual SemanticSymbol* lockable() const = 0;

        virtual LValNode* copy() const override = 0;
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

        virtual void setValue(Runtime::ISymbolValueStore* store, ExpressionNode* value) override;

        virtual ExpressionNode* getValue(Runtime::ISymbolValueStore* store) override;

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

        virtual const Type* type() const override {
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
        virtual ~MapAccessNode() {}

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

        virtual void setValue(Runtime::ISymbolValueStore* store, ExpressionNode* value) override;

        virtual ExpressionNode* getValue(Runtime::ISymbolValueStore* store) override;

        SemanticSymbol* lockable() const override;

        virtual bool shared() const override {
            return _path->shared();
        }

        virtual MapAccessNode* copy() const override {
            return new MapAccessNode(position()->copy(), _path->copy(), _end->copy());
        }

        virtual const Type* type() const override {
            auto pathType = _path->type();
            assert(pathType->isGenericType());
            return ((GenericType*) pathType)->concrete();
        }
    private:
        LValNode* _path;
        IdentifierNode* _end;
    };

    /** Base class for AST nodes referencing types. */
    class TypeNode : public ASTNode {
    public:
        static TypeNode* newForType(Type* type);

        TypeNode(Position* pos) : ASTNode(pos) {}
        virtual ~TypeNode() {}

        /** Get the Type instance this node describes. */
        virtual Type* type() const = 0;

        virtual bool isType() const {
            return true;
        }

        virtual void setShared(bool shared) = 0;

        virtual TypeNode* copy() const override = 0;
    };


    /** AST node referencing a primitive type declaration. */
    class PrimitiveTypeNode final : public TypeNode {
    public:
        PrimitiveTypeNode(Position* pos, PrimitiveType* t) : TypeNode(pos), _t(t) {}
        virtual ~PrimitiveTypeNode() {}

        virtual std::string getName() const override {
            return "PrimitiveTypeNode";
        }

        virtual std::string toString() const override {
            std::stringstream s;
            s << "PrimitiveTypeNode<of: " << Type::valueTypeToString(_t->valueType())
              << ", shared: " << (_t->shared() ? "true>" : "false>");
            return s.str();
        }

        virtual Type* type() const override {
            return _t;
        }

        virtual void setShared(bool shared) override {
            _t = PrimitiveType::of(_t->valueType(), shared);
        }

        virtual PrimitiveTypeNode* copy() const override {
            return new PrimitiveTypeNode(position()->copy(), _t);
        }

    protected:
        PrimitiveType* _t;
    };


    /** Base class for TypeNodes that take a type-generic. */
    class GenericTypeNode : public TypeNode {
    public:
        GenericTypeNode(Position* pos, TypeNode* concrete) : TypeNode(pos), _concrete(concrete) {}
        virtual ~GenericTypeNode() {}

        virtual void setShared(bool shared) override {
            _shared = shared;    
            _concrete->setShared(shared);  
        }

        virtual GenericTypeNode* copy() const override = 0;

    protected:
        TypeNode* _concrete;
        bool _shared;
    };


    /** AST node referencing an enumerable type declaration. */
    class EnumerableTypeNode final : public GenericTypeNode {
    public:
        EnumerableTypeNode(Position* pos, TypeNode* concrete) : GenericTypeNode(pos, concrete) {}
        virtual ~EnumerableTypeNode() {}

        virtual std::string getName() const override {
            return "EnumerableTypeNode";
        }

        virtual Type* type() const override {
            return GenericType::of(ValueType::TENUMERABLE, _concrete->type());
        }

        virtual std::string toString() const override {
            std::stringstream s;
            s << "EnumerableTypeNode<of: " << _concrete->toString() 
                << ", shared: " << (type()->shared() ? "true>" : "false>");
            return s.str();
        }

        virtual EnumerableTypeNode* copy() const override {
            return new EnumerableTypeNode(position()->copy(), _concrete->copy());
        }
    };


    /** AST node referencing a map type declaration. */
    class MapTypeNode final : public GenericTypeNode {
    public:
        MapTypeNode(Position* pos, TypeNode* concrete) : GenericTypeNode(pos, concrete) {}
        virtual ~MapTypeNode() {}

        virtual std::string getName() const override {
            return "MapTypeNode";
        }

        virtual Type* type() const override {
            return GenericType::of(ValueType::TMAP, _concrete->type());
        }

        virtual std::string toString() const override {
            std::stringstream s;
            s << "MapTypeNode<of: " << _concrete->toString() 
                << ", shared: " << (type()->shared() ? "true>" : "false>");
            return s.str();
        }

        virtual MapTypeNode* copy() const override {
            return new MapTypeNode(position()->copy(), _concrete->copy());
        }
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

        virtual const Type* type() const override {
            return PrimitiveType::of(ValueType::TBOOL);
        }

        virtual bool equals(const ExpressionNode* other) const {
            return (
                other->getName() == getName()
                && ((BooleanLiteralExpressionNode*) other)->value() == value()
            );
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
         * @param type - the TypeNode declaring the variable
         * @param id - the name of the variable
         * @param value - the initial value of the variable
         */
        VariableDeclarationNode(Position* pos, TypeNode* type, IdentifierNode* id, ExpressionNode* value)
            : DeclarationNode(pos), _type(type), _id(id), _value(value) {}

        virtual std::string getName() const override {
            return "VariableDeclarationNode";
        }

        virtual ~VariableDeclarationNode() {}

        virtual std::string toString() const override {
            return "VariableDeclarationNode<name: " + _id->name() + ">";
        }

        IdentifierNode* id() const {
            return _id;
        }

        ExpressionNode* value() const {
            return _value;
        }

        TypeNode* typeNode() const {
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
        TypeNode* _type;
        IdentifierNode* _id;
        ExpressionNode* _value;
    };


    /** AST node representing an assignment of a value to an lval. */
    class AssignExpressionNode : public StatementExpressionNode {
    public:
        AssignExpressionNode(Position* pos, LValNode* dest, ExpressionNode* value) : StatementExpressionNode(pos), _dest(dest), _value(value) {}
        virtual ~AssignExpressionNode() {}

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

        virtual const Type* type() const override {
            return _value->type();
        }

    protected:
        LValNode* _dest;
        ExpressionNode* _value;
    };


    /** AST node representing a call to a function. */
    class CallExpressionNode final : public StatementExpressionNode {
    public:
        CallExpressionNode(Position* pos, IdentifierNode* id, std::vector<ExpressionNode*>* args) : StatementExpressionNode(pos), _id(id), _args(args) {}
        virtual ~CallExpressionNode() {}

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

        virtual const Type* type() const override {
            auto fnType = _id->type();
            assert(fnType->isFunctionType());
            return ((FunctionType*) fnType)->returnType();
        }

    protected:
        IdentifierNode* _id;
        std::vector<ExpressionNode*>* _args;
    };


    /** Base class for expressions of two operands. */
    class BinaryExpressionNode : public ExpressionNode {
    public:
        BinaryExpressionNode(Position* pos, ExpressionNode* left, ExpressionNode* right) : ExpressionNode(pos), _left(left), _right(right) {}
        virtual ~BinaryExpressionNode() {}

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

        virtual const Type* leftType() const = 0;

        virtual const Type* rightType() const = 0;

        virtual const Type* resultType() const = 0;

        virtual PureBinaryExpressionNode* copy() const override = 0;

        virtual const Type* type() const override {
            return resultType();
        }
    };


    /** Base class for binary nodes that take bool -> bool -> bool */
    class PureBooleanBinaryExpressionNode : public PureBinaryExpressionNode {
    public:
        PureBooleanBinaryExpressionNode(Position* pos, ExpressionNode* left, ExpressionNode* right) : PureBinaryExpressionNode(pos, left, right) {}
        virtual ~PureBooleanBinaryExpressionNode() {}

        virtual const Type* leftType() const override {
            return PrimitiveType::of(ValueType::TBOOL, false);
        }

        virtual const Type* rightType() const override {
            return PrimitiveType::of(ValueType::TBOOL, false);
        }

        virtual const Type* resultType() const override {
            return PrimitiveType::of(ValueType::TBOOL, false);
        }

        virtual PureBooleanBinaryExpressionNode* copy() const override = 0;
    };


    /** Base class for binary nodes that take num -> num -> num */
    class PureNumberBinaryExpressionNode : public PureBinaryExpressionNode {
    public:
        PureNumberBinaryExpressionNode(Position* pos, ExpressionNode* left, ExpressionNode* right) : PureBinaryExpressionNode(pos, left, right) {}
        virtual ~PureNumberBinaryExpressionNode() {}

        virtual const Type* leftType() const override {
            return PrimitiveType::of(ValueType::TNUM, false);
        }

        virtual const Type* rightType() const override {
            return PrimitiveType::of(ValueType::TNUM, false);
        }

        virtual const Type* resultType() const override {
            return PrimitiveType::of(ValueType::TNUM, false);
        }

        virtual PureNumberBinaryExpressionNode* copy() const override = 0;
    };


    /** Base class for binary nodes that take string -> string -> string */
    class PureStringBinaryExpressionNode : public PureBinaryExpressionNode {
    public:
        PureStringBinaryExpressionNode(Position* pos, ExpressionNode* left, ExpressionNode* right) : PureBinaryExpressionNode(pos, left, right) {}
        virtual ~PureStringBinaryExpressionNode() {}

        virtual const Type* leftType() const override {
            return PrimitiveType::of(ValueType::TSTRING, false);
        }

        virtual const Type* rightType() const override {
            return PrimitiveType::of(ValueType::TSTRING, false);
        }

        virtual const Type* resultType() const override {
            return PrimitiveType::of(ValueType::TSTRING, false);
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

        virtual const Type* type() const override {
            return PrimitiveType::of(ValueType::TBOOL);
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

        const Type* leftType() const override {
            return PrimitiveType::of(ValueType::TNUM);
        }

        const Type* rightType() const override {
            return PrimitiveType::of(ValueType::TNUM);
        }

        const Type* resultType() const override {
            return PrimitiveType::of(ValueType::TBOOL);
        }

        PureBinaryExpressionNode* copy() const override {
            return new NumericComparisonExpressionNode(position()->copy(), _comparisonType, _left->copy(), _right->copy());
        }

        std::string getName() const {
            return "NumericComparisonExpressionNode";
        }

        std::string toString() const {
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

        virtual const Type* type() const override {
            return PrimitiveType::of(ValueType::TBOOL);
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

    /** AST node representing the addition of a value to the existing value of a lval. */
    class AddAssignExpressionNode final : public AssignExpressionNode {
    public:
        AddAssignExpressionNode(Position* pos, LValNode* dest, ExpressionNode* value) : 
            AssignExpressionNode(pos, dest, value) {}
        virtual ~AddAssignExpressionNode() {}

        virtual std::string getName() const override {
            return "AddAssignExpressionNode";
        }

        virtual std::string toString() const override {
            return "AddAssignExpressionNode<lval: " + _dest->toString() + ">";
        }

        virtual AddAssignExpressionNode* copy() const override {
            return new AddAssignExpressionNode(position()->copy(), _dest->copy(), _value->copy());
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


    /** AST node representing the multiplication of a value to the existing value of a lval. */
    class MultiplyAssignExpressionNode final : public AssignExpressionNode {
    public:
        MultiplyAssignExpressionNode(Position* pos, LValNode* dest, ExpressionNode* value) :
            AssignExpressionNode(pos, dest, value) {}
        virtual ~MultiplyAssignExpressionNode() {}

        virtual std::string getName() const override {
            return "MultiplyAssignExpressionNode";
        }

        virtual std::string toString() const override {
            return "MultiplyAssignExpressionNode<lval: " + _dest->toString() + ">";
        }

        virtual MultiplyAssignExpressionNode* copy() const override {
            return new MultiplyAssignExpressionNode(position()->copy(), _dest->copy(), _value->copy());
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
        virtual ~UnaryExpressionNode() {}

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

        virtual const Type* type() const override {
            return PrimitiveType::of(ValueType::TNUM);
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

        virtual const Type* type() const override {
            return PrimitiveType::of(ValueType::TBOOL);
        }
    };


    /** AST node representing literal enumerations. */
    class EnumerationLiteralExpressionNode final : public ExpressionNode {
    public:
        EnumerationLiteralExpressionNode(Position* pos, ExpressionList* actuals) : ExpressionNode(pos), _actuals(actuals) {}
        EnumerationLiteralExpressionNode(Position* pos, ExpressionList* actuals, TypeNode* disambiguationType) : ExpressionNode(pos), _actuals(actuals), _disambiguationType(disambiguationType) {}
        virtual ~EnumerationLiteralExpressionNode() {}

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
            auto type = _disambiguationType == nullptr ? nullptr : _disambiguationType->copy();
            auto actuals = new ExpressionList;
            for ( auto actual : *_actuals ) actuals->push_back(actual->copy());

            return new EnumerationLiteralExpressionNode(position()->copy(), actuals, type);
        }

        virtual const Type* type() const override {
            assert(_disambiguationType != nullptr || !_actuals->empty());
            return (_disambiguationType == nullptr) ? _actuals->at(0)->type() : _disambiguationType->type();
        }

        virtual bool equals(const ExpressionNode* other) const override {
            if ( other->getName() != "EnumerationLiteralExpressionNode" ) {
                return false;
            }

            auto otherEnum = (EnumerationLiteralExpressionNode*) other;
            if ( _actuals->size() != otherEnum->_actuals->size() ) {
                return false;
            }

            for ( size_t i = 0; i < _actuals->size(); i += 1 ) {
                auto entry = _actuals->at(i);
                auto otherEntry = otherEnum->_actuals->at(i);
                if ( !entry->equals(otherEntry) ) {
                    return false;
                }
            }

            return true;
        }

    protected:
        ExpressionList* _actuals;
        TypeNode* _disambiguationType = nullptr;

        friend class Walk::TypeAnalysisWalk;
        friend class Runtime::InterpretWalk;
    };


    /** Base class for AST nodes that contain a body of statements. */
    class BlockStatementNode : public StatementNode {
    public:
        BlockStatementNode(Position* pos) : StatementNode(pos) {
            _body = new StatementList();
        }

        virtual ~BlockStatementNode() {
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
            auto other = new StatementList;
            for ( auto stmt : *body() ) other->push_back(stmt->copy());
            return other;
        }

        virtual BlockStatementNode* copy() const override = 0;
    protected:
        StatementList* _body;
    };


    /** Internal node used by the runtime. Represents a single block of statements. */
    class CapturedBlockStatementNode final : public BlockStatementNode {
    public:
        CapturedBlockStatementNode(Position* pos) : BlockStatementNode(pos) {}

        virtual ~CapturedBlockStatementNode() {}

        virtual std::string getName() const override {
            return "CapturedBlockStatementNode";
        }

        virtual std::string toString() const override {
            return "CapturedBlockStatementNode<>";
        }

        virtual CapturedBlockStatementNode* copy() const override {
            auto other = new CapturedBlockStatementNode(position()->copy());
            other->assumeAndReduceStatements(copyBody());
            return other;
        }
    };


    /** AST node representing an enumeration block. */
    class EnumerationStatement final : public BlockStatementNode {
    public:
        EnumerationStatement(Position* pos, IdentifierNode* enumerable, IdentifierNode* local, bool shared)
            : BlockStatementNode(pos), _enumerable(enumerable), _local(local), _shared(shared) {}

        virtual ~EnumerationStatement() {}

        virtual std::string getName() const override {
            return "EnumerationStatement";
        }

        virtual std::string toString() const override {
            return "EnumerationStatement<e: " + _enumerable->name() + ", as: " + _local->name() + ", #body: " + std::to_string(_body->size()) + ">";
        }

        IdentifierNode* enumerable() const {
            return _enumerable;
        }

        IdentifierNode* local() const {
            return _local;
        }

        virtual EnumerationStatement* copy() const override {
            auto other = new EnumerationStatement(position()->copy(), _enumerable->copy(), _local->copy(), _shared);
            other->assumeAndReduceStatements(copyBody());
            return other;
        }
    protected:
        IdentifierNode* _enumerable;
        IdentifierNode* _local;
        bool _shared;

        friend class Walk::NameAnalysisWalk;
        friend class Walk::SymbolScrubWalk;
    };


    /** AST node representing a with-resource block. */
    class WithStatement final : public BlockStatementNode {
    public:
        WithStatement(Position* pos, ExpressionNode* resource, IdentifierNode* local, bool shared)
            : BlockStatementNode(pos), _resource(resource), _local(local), _shared(shared) {}

        virtual ~WithStatement() {}

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

        virtual ~IfStatement() {}

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

        virtual ~WhileStatement() {}

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
        virtual ~MapStatementNode() {}

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
        MapNode(Position* pos, MapBody* body) : ExpressionNode(pos), _body(body) {}
        MapNode(Position* pos, MapBody* body, TypeNode* disambiguationType) : ExpressionNode(pos), _body(body), _disambiguationType(disambiguationType) {}
        virtual ~MapNode() {}

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
            auto type = _disambiguationType == nullptr ? nullptr : _disambiguationType->copy();
            auto body = new MapBody;
            for ( auto entry : *_body ) body->push_back(entry->copy());
            return new MapNode(position()->copy(), body, type);
        }

        virtual const Type* type() const override {
            assert(_disambiguationType != nullptr || !_body->empty());
            return (_disambiguationType == nullptr) ? _body->at(0)->value()->type() : _disambiguationType->type();
        }

        bool equals(const ExpressionNode* other) const override {
            if ( other->getName() != getName() ) return false;
            auto otherMap = (MapNode*) other;

            if ( otherMap->_body->size() != _body->size() ) return false;

            // Make sure both sides have all the same keys
            std::vector<std::string> keys;
            for ( auto entry : *_body ) {
                keys.push_back(entry->id()->name());
            }
            std::sort(keys.begin(), keys.end());

            std::vector<std::string> otherKeys;
            for ( auto entry : *otherMap->_body ) {
                otherKeys.push_back(entry->id()->name());
            }
            std::sort(otherKeys.begin(), otherKeys.end());

            for ( size_t i = 0; i < keys.size(); i += 1 ) {
                if ( keys[i] != otherKeys[i] ) {
                    return false;
                }
            }

            // Make sure all the keys have all the same values
            for ( auto node : *_body ) {
                auto otherNode = otherMap->getBodyNode(node->id());
                if ( !node->value()->equals(otherNode->value()) ) {
                    return false;
                }
            }

            return true;
        }

    protected:
        MapBody* _body;
        TypeNode* _disambiguationType = nullptr;

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

        virtual const Type* type() const override {
            return PrimitiveType::of(ValueType::TSTRING);
        }

        bool equals(const ExpressionNode* other) const override {
            return (
                other->getName() == getName()
                && ((StringLiteralExpressionNode*) other)->value() == _value
            );
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

        virtual const Type* type() const override {
            return PrimitiveType::of(ValueType::TNUM);
        }

        bool equals(const ExpressionNode* other) const override {
            return (
                other->getName() == getName()
                && ((NumberLiteralExpressionNode*) other)->value() == _value
            );
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
        EnumerableAccessNode(Position* pos, LValNode* path, IntegerLiteralExpressionNode* index) : LValNode(pos), _path(path), _index(index) {}
        virtual ~EnumerableAccessNode() {}

        virtual std::string getName() const override {
            return "EnumerableAccessNode";
        }

        virtual std::string toString() const override {
            std::stringstream s;
            s << "EnumerableAccessNode<path: " << _path->toString() << ", index: " << _index->value() << ">";
            return s.str();
        }

        LValNode* path() const {
            return _path;
        }

        IntegerLiteralExpressionNode* index() const {
            return _index;
        }

        ExpressionNode* getValue(Runtime::ISymbolValueStore* store) override;

        void setValue(Runtime::ISymbolValueStore* store, ExpressionNode* node) override;

        SemanticSymbol* lockable() const override;

        virtual bool shared() const override {
            return _path->shared();
        }

        virtual EnumerableAccessNode* copy() const override {
            return new EnumerableAccessNode(position()->copy(), _path->copy(), _index->copy());
        }

        virtual const Type* type() const override {
            auto baseType = _path->type();
            assert(baseType->isGenericType());
            return ((GenericType*) baseType)->concrete();
        }
    private:
        LValNode* _path;
        IntegerLiteralExpressionNode* _index;
    };

}
}


#endif
