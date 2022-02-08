#ifndef SWARMC_AST_H
#define SWARMC_AST_H

#include <vector>
#include <ostream>
#include "../shared/IStringable.h"
#include "../shared/util/Console.h"
#include "Position.h"
#include "Type.h"
#include "SymbolTable.h"
#include "TypeTable.h"

namespace swarmc {
namespace Lang {

    class StatementNode;
    class ExpressionNode;
    class MapStatementNode;

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

        /** Print this node and its subtree. */
        virtual void printTree(std::ostream& out, std::string prefix = "") const = 0;

        /**
         * Perform name analysis on this node and its subtree.
         * @return false if name analysis failed
         */
        virtual bool nameAnalysis(SymbolTable* symbols) = 0;

        /**
         * Perform type analysis on this node and its subtree.
         * @return false if type analysis failed
         */
        virtual bool typeAnalysis(TypeTable* types) = 0;

        /** Get the node's Position instance. */
        virtual Position* position() const {
            return _pos;
        };

        virtual std::string getName() const = 0;

        virtual bool isExpression() const {
            return false;
        }

        virtual bool isValue() const {
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

        virtual void printTree(std::ostream& out, std::string prefix = "") const override;

        /**
         * Helper that instantiates a new SymbolTable and starts name analysis.
         * @returns true if all names were valid
         */
        virtual bool nameAnalysis();

        virtual bool nameAnalysis(SymbolTable* symbols) override;

        virtual bool typeAnalysis(TypeTable* types) override;
        
        virtual bool typeAnalysis() {
            TypeTable* types = new TypeTable();
            return typeAnalysis(types);
        }

    protected:
        /** The statements that comprise the program. */
        StatementList* _body;
    };


    /** AST node representing a statement that can occur in the root of a program. */
    class StatementNode : public ASTNode {
    public:
        StatementNode(Position* pos) : ASTNode(pos) {}
        virtual ~StatementNode() {}
    };


    /** AST node representing code that evaluates to a value. */
    class ExpressionNode : public ASTNode {
    public:
        ExpressionNode(Position* pos) : ASTNode(pos) {}
        virtual ~ExpressionNode() {}

        virtual void printTree(std::ostream& out, std::string prefix = "") const override;

        virtual bool typeAnalysis(TypeTable* types) override = 0;

        virtual bool isExpression() const override {
            return true;
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

        virtual void printTree(std::ostream& out, std::string prefix = "") const override;

        virtual bool nameAnalysis(SymbolTable* symbols) override;

        virtual bool typeAnalysis(TypeTable* types) override;

    protected:
        StatementExpressionNode* _exp;
    };


    /** Base class for AST nodes that can have a value assigned to them. */
    class LValNode : public ExpressionNode {
    public:
        LValNode(Position* pos) : ExpressionNode(pos) {}
        virtual ~LValNode() {}
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

        virtual bool nameAnalysis(SymbolTable* symbols) override;

        virtual bool typeAnalysis(TypeTable* types) override;

        /** Get the semantic symbol associated with this identifier in its scope. */
        SemanticSymbol* symbol() const {
            return _symbol;
        }

    protected:
        std::string _name;
        SemanticSymbol* _symbol = nullptr;
    };


    /** Base class for AST nodes referencing types. */
    class TypeNode : public ASTNode {
    public:
        TypeNode(Position* pos) : ASTNode(pos) {}
        virtual ~TypeNode() {}

        /** Get the Type instance this node describes. */
        virtual Type* type() const = 0;

        virtual void printTree(std::ostream& out, std::string prefix = "") const override;

        virtual bool nameAnalysis(SymbolTable* symbols) override;

        virtual bool typeAnalysis(TypeTable* types) override;
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
            return "PrimitiveTypeNode<of: " + Type::valueTypeToString(_t->valueType()) + ">";
        }

        virtual Type* type() const override {
            return _t;
        }

    protected:
        PrimitiveType* _t;
    };


    /** Base class for TypeNodes that take a type-generic. */
    class GenericTypeNode : public TypeNode {
    public:
        GenericTypeNode(Position* pos, TypeNode* concrete) : TypeNode(pos), _concrete(concrete) {}
        virtual ~GenericTypeNode() {}

    protected:
        TypeNode* _concrete;
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
            return GenericType::of(TENUMERABLE, _concrete->type());
        }

        virtual std::string toString() const override {
            return "EnumerableTypeNode<of: " + _concrete->toString() + ">";
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
            return GenericType::of(TMAP, _concrete->type());
        }

        virtual std::string toString() const override {
            return "MapTypeNode<of: " + _concrete->toString() + ">";
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

        virtual bool nameAnalysis(SymbolTable* symbols) override;

        virtual bool typeAnalysis(TypeTable* types) override;

        /** Get the value of the literal expression. */
        virtual bool value() const {
            return _val;
        }

        virtual bool isValue() const override {
            return true;
        }

    private:
        const bool _val;
    };


    /** Base class for AST nodes declaring things in scope. */
    class DeclarationNode : public StatementNode {
    public:
        DeclarationNode(Position* pos) : StatementNode(pos) {}
        virtual ~DeclarationNode() {}
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

        virtual void printTree(std::ostream& out, std::string prefix = "") const override;

        virtual bool nameAnalysis(SymbolTable* symbols) override;

        virtual bool typeAnalysis(TypeTable* types) override;

        IdentifierNode* id() const {
            return _id;
        }

        ExpressionNode* value() const {
            return _value;
        }

        TypeNode* typeNode() const {
            return _type;
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

        virtual void printTree(std::ostream& out, std::string prefix = "") const override;

        virtual bool nameAnalysis(SymbolTable* symbols) override;

        virtual bool typeAnalysis(TypeTable* types) override;

        virtual std::string getName() const override {
            return "AssignExpressionNode";
        }

        virtual LValNode* dest() const {
            return _dest;
        }

        virtual ExpressionNode* value() const {
            return _value;
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

        virtual void printTree(std::ostream& out, std::string prefix = "") const override;

        virtual bool nameAnalysis(SymbolTable* symbols) override;

        virtual bool typeAnalysis(TypeTable* types) override;

        IdentifierNode* id() const {
            return _id;
        }

        std::vector<ExpressionNode*>* args() const {
            return _args;
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

        virtual void printTree(std::ostream& out, std::string prefix = "") const override;

        virtual bool nameAnalysis(SymbolTable* symbols) override;

        ExpressionNode* left() const {
            return _left;
        }

        ExpressionNode* right() const {
            return _right;
        }
    protected:
        ExpressionNode* _left;
        ExpressionNode* _right;
    };


    /** Base class for expression nodes over static types. */
    class PureBinaryExpressionNode : public BinaryExpressionNode {
    public:
        PureBinaryExpressionNode(Position* pos, ExpressionNode* left, ExpressionNode* right) : BinaryExpressionNode(pos, left, right) {}
        virtual ~PureBinaryExpressionNode() {}

        virtual bool typeAnalysis(TypeTable* types) override;

        virtual const Type* leftType() const = 0;

        virtual const Type* rightType() const = 0;

        virtual const Type* resultType() const = 0;

    };


    /** Base class for binary nodes that take bool -> bool -> bool */
    class PureBooleanBinaryExpressionNode : public PureBinaryExpressionNode {
    public:
        PureBooleanBinaryExpressionNode(Position* pos, ExpressionNode* left, ExpressionNode* right) : PureBinaryExpressionNode(pos, left, right) {}
        virtual ~PureBooleanBinaryExpressionNode() {}

        virtual const Type* leftType() const override {
            return PrimitiveType::of(TBOOL);
        }

        virtual const Type* rightType() const override {
            return PrimitiveType::of(TBOOL);
        }

        virtual const Type* resultType() const override {
            return PrimitiveType::of(TBOOL);
        }
    };


    /** Base class for binary nodes that take num -> num -> num */
    class PureNumberBinaryExpressionNode : public PureBinaryExpressionNode {
    public:
        PureNumberBinaryExpressionNode(Position* pos, ExpressionNode* left, ExpressionNode* right) : PureBinaryExpressionNode(pos, left, right) {}
        virtual ~PureNumberBinaryExpressionNode() {}

        virtual const Type* leftType() const override {
            return PrimitiveType::of(TNUM);
        }

        virtual const Type* rightType() const override {
            return PrimitiveType::of(TNUM);
        }

        virtual const Type* resultType() const override {
            return PrimitiveType::of(TNUM);
        }
    };


    /** Base class for binary nodes that take string -> string -> string */
    class PureStringBinaryExpressionNode : public PureBinaryExpressionNode {
    public:
        PureStringBinaryExpressionNode(Position* pos, ExpressionNode* left, ExpressionNode* right) : PureBinaryExpressionNode(pos, left, right) {}
        virtual ~PureStringBinaryExpressionNode() {}

        virtual const Type* leftType() const override {
            return PrimitiveType::of(TSTRING);
        }

        virtual const Type* rightType() const override {
            return PrimitiveType::of(TSTRING);
        }

        virtual const Type* resultType() const override {
            return PrimitiveType::of(TSTRING);
        }
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
    };


    /** AST node referencing equality comparison of two expressions. */
    class EqualsNode final : public BinaryExpressionNode {
    public:
        EqualsNode(Position* pos, ExpressionNode* left, ExpressionNode* right) : BinaryExpressionNode(pos, left, right) {}

        virtual std::string getName() const override {
            return "EqualsNode";
        }

        virtual bool typeAnalysis(TypeTable* types) override;

        virtual std::string toString() const override {
            return "EqualsNode<>";
        }
    };


    /** AST node referencing inequality comparison of two expressions. */
    class NotEqualsNode final : public BinaryExpressionNode {
    public:
        NotEqualsNode(Position* pos, ExpressionNode* left, ExpressionNode* right) : BinaryExpressionNode(pos, left, right) {}

        virtual std::string getName() const override {
            return "NotEqualsNode";
        }

        virtual bool typeAnalysis(TypeTable* types) override;

        virtual std::string toString() const override {
            return "NotEqualsNode<>";
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
    };

    /** AST node representing the addition of a value to the existing value of a lval. */
    class AddAssignExpressionNode final : public AssignExpressionNode {
    public:
        AddAssignExpressionNode(Position* pos, LValNode* dest, ExpressionNode* value) : 
            AssignExpressionNode(
                pos, 
                dest, 
                new AddNode(pos, dest, value)
            ) {}
        virtual ~AddAssignExpressionNode() {}

        virtual std::string getName() const override {
            return "AddAssignExpressionNode";
        }

        virtual std::string toString() const override {
            return "AddAssignExpressionNode<lval: " + _dest->toString() + ">";
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
    };


    /** AST node representing the multiplication of a value to the existing value of a lval. */
    class MultiplyAssignExpressionNode final : public AssignExpressionNode {
    public:
        MultiplyAssignExpressionNode(Position* pos, LValNode* dest, ExpressionNode* value) :
            AssignExpressionNode(
                pos, 
                dest, 
                new MultiplyNode(pos, dest, value)
            ) {}
        virtual ~MultiplyAssignExpressionNode() {}

        virtual std::string getName() const override {
            return "MultiplyAssignExpressionNode";
        }

        virtual std::string toString() const override {
            return "MultiplyAssignExpressionNode<lval: " + _dest->toString() + ">";
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
    };

    /** Base class for expressions of one operand. */
    class UnaryExpressionNode : public ExpressionNode {
    public:
        UnaryExpressionNode(Position* pos, ExpressionNode* exp) : ExpressionNode(pos), _exp(exp) {}
        virtual ~UnaryExpressionNode() {}
        virtual void printTree(std::ostream& out, std::string prefix = "") const override;

        virtual bool nameAnalysis(SymbolTable* symbols) override;

        virtual bool typeAnalysis(TypeTable* types) override = 0;

        ExpressionNode* exp() const {
            return _exp;
        }
    protected:
        ExpressionNode* _exp;
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

        virtual bool typeAnalysis(TypeTable* types) override;
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

        virtual void printTree(std::ostream& out, std::string prefix = "") const override;

        virtual bool nameAnalysis(SymbolTable* symbols) override;

        virtual bool typeAnalysis(TypeTable* types) override;

        ExpressionList* actuals() const {
            return _actuals;
        }

        virtual bool isValue() const override {
            return true;
        }

    protected:
        ExpressionList* _actuals;
        TypeNode* _disambiguationType = nullptr;
    };


    /** Base class for AST nodes that contain a body of statements. */
    class BlockStatementNode : public StatementNode {
    public:
        BlockStatementNode(Position* pos) : StatementNode(pos) {
            _body = new StatementList();
        }

        virtual ~BlockStatementNode() {}

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

        virtual void printTree(std::ostream& out, std::string prefix = "") const override;

        virtual bool nameAnalysis(SymbolTable* symbols) override;

        virtual bool typeAnalysis(TypeTable* types) override = 0;

        StatementList* body() const {
            return _body;
        }
    protected:
        StatementList* _body;
    };


    /** AST node representing an enumeration block. */
    class EnumerationStatement final : public BlockStatementNode {
    public:
        EnumerationStatement(Position* pos, IdentifierNode* enumerable, IdentifierNode* local)
            : BlockStatementNode(pos), _enumerable(enumerable), _local(local) {}

        virtual ~EnumerationStatement() {}

        virtual std::string getName() const override {
            return "EnumerationStatement";
        }

        virtual std::string toString() const override {
            return "EnumerationStatement<e: " + _enumerable->name() + ", as: " + _local->name() + ", #body: " + std::to_string(_body->size()) + ">";
        }

        virtual bool nameAnalysis(SymbolTable* symbols) override;

        virtual bool typeAnalysis(TypeTable* types) override;

        IdentifierNode* enumerable() const {
            return _enumerable;
        }

        IdentifierNode* local() const {
            return _local;
        }
    protected:
        IdentifierNode* _enumerable;
        IdentifierNode* _local;
    };


    /** AST node representing a with-resource block. */
    class WithStatement final : public BlockStatementNode {
    public:
        WithStatement(Position* pos, ExpressionNode* resource, IdentifierNode* local)
            : BlockStatementNode(pos), _resource(resource), _local(local) {}

        virtual ~WithStatement() {}

        virtual std::string getName() const override {
            return "WithStatement";
        }

        virtual std::string toString() const override {
            return "WithStatement<r: " + _resource->toString() + ", as: " + _local->name() + ">";
        }

        virtual bool nameAnalysis(SymbolTable* symbols) override;

        virtual bool typeAnalysis(TypeTable* types) override;

        ExpressionNode* resource() const {
            return _resource;
        }

        IdentifierNode* local() const {
            return _local;
        }
    protected:
        ExpressionNode* _resource;
        IdentifierNode* _local;
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

        virtual bool nameAnalysis(SymbolTable* symbols) override;
        virtual bool typeAnalysis(TypeTable* types) override;

        ExpressionNode* condition() const {
            return _condition;
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

        virtual bool nameAnalysis(SymbolTable* symbols) override;
        virtual bool typeAnalysis(TypeTable* types) override;

        ExpressionNode* condition() const {
            return _condition;
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

        virtual void printTree(std::ostream& out, std::string prefix = "") const override;

        virtual bool nameAnalysis(SymbolTable* symbols) override;

        virtual bool typeAnalysis(TypeTable* types) override;

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

        virtual void printTree(std::ostream& out, std::string prefix = "") const override;

        virtual bool nameAnalysis(SymbolTable* symbols) override;

        virtual bool typeAnalysis(TypeTable* types) override;

        MapBody* body() const {
            return _body;
        }

        virtual bool isValue() const override {
            return true;
        }

    protected:
        MapBody* _body;
        TypeNode* _disambiguationType;
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

        virtual bool nameAnalysis(SymbolTable* symbols) override { return true; }

        virtual bool typeAnalysis(TypeTable* types) override;
    protected:
        std::string _value;
    };

    /** AST node representing literal numbers. */
    class NumberLiteralExpressionNode final : public ExpressionNode {
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

        virtual bool nameAnalysis(SymbolTable* symbols) override { return true; }

        virtual bool typeAnalysis(TypeTable* types) override;
    protected:
        double _value;
    };

}
}


#endif
