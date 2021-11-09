#ifndef SWARMC_AST_H
#define SWARMC_AST_H

#include <vector>
#include "../shared/IStringable.h"
#include "Position.h"
#include "Type.h"

namespace swarmc {
namespace Lang {

    class StatementNode;
    class ExpressionNode;
    class MapStatementNode;

    using StatementList = std::vector<StatementNode*>;
    using ExpressionList = std::vector<ExpressionNode*>;
    using MapBody = std::vector<MapStatementNode*>;

    /** Base class for all AST nodes. */
    class ASTNode : public IStringable {
    public:
        ASTNode(Position* pos) : _pos(pos) {};

        virtual ~ASTNode() {}

        /** Implements IStringable. */
        virtual std::string toString() const = 0;

        /** Get the node's Position instance. */
        virtual Position* position() const {
            return _pos;
        };

    private:
        Position* _pos = nullptr;
    };


    /** AST node representing the root of the program. */
    class ProgramNode final : public ASTNode {
    public:
        ProgramNode() : ASTNode(new Position(0, 0, 0, 0)) {
            _body = new std::vector<StatementNode*>();
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

        virtual std::string toString() const {
            return "ExpressionStatementNode<" + _exp->toString() + ">";
        }

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

        virtual std::string toString() const override {
            return "IdentifierNode<name: " + _name + ">";
        }

    protected:
        std::string _name;
    };


    /** Base class for AST nodes referencing types. */
    class TypeNode : public ASTNode {
    public:
        TypeNode(Position* pos) : ASTNode(pos) {}
        virtual ~TypeNode() {}
    };


    /** AST node referencing a primitive type declaration. */
    class PrimitiveTypeNode final : public TypeNode {
    public:
        PrimitiveTypeNode(Position* pos, PrimitiveType* t) : TypeNode(pos), _t(t) {}
        virtual ~PrimitiveTypeNode() {}

        virtual std::string toString() const {
            return "PrimitiveTypeNode<of: " + Type::valueTypeToString(_t->valueType()) + ">";
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

        virtual std::string toString() const {
            return "EnumerableTypeNode<of: " + _concrete->toString() + ">";
        }
    };


    /** AST node referencing a map type declaration. */
    class MapTypeNode final : public GenericTypeNode {
    public:
        MapTypeNode(Position* pos, TypeNode* concrete) : GenericTypeNode(pos, concrete) {}
        virtual ~MapTypeNode() {}

        virtual std::string toString() const {
            return "MapTypeNode<of: " + _concrete->toString() + ">";
        }
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

        virtual ~VariableDeclarationNode() {}

        virtual std::string toString() const {
            return "VariableDeclarationNode<name: " + _id->name() + ">";
        }

    protected:
        TypeNode* _type;
        IdentifierNode* _id;
        ExpressionNode* _value;
    };


    /** AST node representing an assignment of a value to an lval. */
    class AssignExpressionNode final : public StatementExpressionNode {
    public:
        AssignExpressionNode(Position* pos, LValNode* dest, ExpressionNode* value) : StatementExpressionNode(pos), _dest(dest), _value(value) {}
        virtual ~AssignExpressionNode() {}

        virtual std::string toString() const {
            return "AssignExpressionNode<lval: " + _dest->toString() + ">";
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

        virtual std::string toString() const {
            return "CallExpressionNode<id: " + _id->name() + ", #args: " + std::to_string(_args->size()) + ">";
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

    protected:
        ExpressionNode* _left;
        ExpressionNode* _right;
    };


    /** Base class for expressions of one operand. */
    class UnaryExpressionNode : public ExpressionNode {
    public:
        UnaryExpressionNode(Position* pos, ExpressionNode* exp) : ExpressionNode(pos), _exp(exp) {}
        virtual ~UnaryExpressionNode() {}

    protected:
        ExpressionNode* _exp;
    };


    /** AST node representing literal enumerations. */
    class EnumerationLiteralExpressionNode final : public ExpressionNode {
    public:
        EnumerationLiteralExpressionNode(Position* pos, ExpressionList* actuals) : ExpressionNode(pos), _actuals(actuals) {}
        virtual ~EnumerationLiteralExpressionNode() {}

        std::string toString() const {
            return "EnumerationLiteralExpressionNode<#actuals: " + std::to_string(_actuals->size()) + ">";
        }

    protected:
        ExpressionList* _actuals;
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

    protected:
        StatementList* _body;
    };


    /** AST node representing an enumeration block. */
    class EnumerationStatement final : public BlockStatementNode {
    public:
        EnumerationStatement(Position* pos, IdentifierNode* enumerable, IdentifierNode* local)
            : BlockStatementNode(pos), _enumerable(enumerable), _local(local) {}

        virtual ~EnumerationStatement() {}

        virtual std::string toString() const {
            return "EnumerationStatement<e: " + _enumerable->name() + ", as: " + _local->name() + ", #body: " + std::to_string(_body->size()) + ">";
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

        virtual std::string toString() const {
            return "WithStatement<r: " + _resource->toString() + ", as: " + _local->name() + ">";
        }

    protected:
        ExpressionNode* _resource;
        IdentifierNode* _local;
    };


    /** AST node referencing one entry in a map. */
    class MapStatementNode final : public ASTNode {
    public:
        MapStatementNode(Position* pos, IdentifierNode* id, ExpressionNode* value) : ASTNode(pos), _id(id), _value(value) {}
        virtual ~MapStatementNode() {}

        std::string toString() const {
            return "MapStatementNode<id: " + _id->name() + ">";
        }

    protected:
        IdentifierNode* _id;
        ExpressionNode* _value;
    };


    /** AST node referencing a map literal. */
    class MapNode final : public ExpressionNode {
    public:
        MapNode(Position* pos, MapBody* body) : ExpressionNode(pos), _body(body) {}
        virtual ~MapNode() {}

        std::string toString() const {
            return "MapNode<#body: " + std::to_string(_body->size()) + ">";
        }

    protected:
        MapBody* _body;
    };

}
}


#endif
