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
}

    class StatementNode;
    class DeclarationNode;
    class ExpressionNode;
    class MapStatementNode;
    class TypeLiteral;
    class IdentifierNode;
    class ClassAccessNode;
    class ConstructorNode;
    class TypeBodyNode;

    using StatementList = std::vector<StatementNode*>;
    using ExpressionList = std::vector<ExpressionNode*>;
    using MapBody = std::vector<MapStatementNode*>;
    using FormalList = std::vector<std::pair<TypeLiteral*, IdentifierNode*>>;
    using DeclarationList = std::vector<DeclarationNode*>;
    using UsedSymbols = std::pair<std::set<VariableSymbol*>, std::set<VariableSymbol*>>;

    enum class ASTNodeTag : std::size_t {
        PROGRAM,
        EXPRESSIONSTATEMENT,
        IDENTIFIER,
        ENUMERABLEACCESS,
        ENUMERABLEAPPEND,
        MAPACCESS,
        CLASSACCESS,
        INCLUDE,
        TYPELITERAL,
        BOOLEANLITERAL,
        STRINGLITERAL,
        NUMBERLITERAL,
        ENUMERATIONLITERAL,
        MAPSTATEMENT,
        MAPLITERAL,
        ASSIGN,
        VARIABLEDECLARATION,
        UNINITIALIZEDVARIABLEDECLARATION,
        USE,
        RETURN,
        FUNCTION,
        CONSTRUCTOR,
        TYPEBODY,
        CALL,
        DEFERCALL,
        AND,
        OR,
        EQUALS,
        NUMERICCOMPARISON,
        NOTEQUALS,
        ADD,
        SUBTRACT,
        MULTIPLY,
        DIVIDE,
        MODULUS,
        POWER,
        NTHROOT,
        NEGATIVE,
        NOT,
        ENUMERATE,
        WITH,
        IF,
        WHILE,
        CONTINUE,
        BREAK,
    };

    /** Base class for all AST nodes. */
    class ASTNode : public IStringable, public IUsesConsole, public IRefCountable {
    public:
        explicit ASTNode(Position* pos) : IUsesConsole(), _pos(useref(pos)) {};

        ~ASTNode() override {
            freeref(_pos);
        };

        /** Implements IStringable. */
        [[nodiscard]] std::string toString() const override = 0;

        [[nodiscard]] virtual ASTNode* copy() const = 0;

        /** Get the node's Position instance. */
        [[nodiscard]] virtual Position* position() const {
            return _pos;
        };

        [[nodiscard]] virtual ASTNodeTag getTag() const = 0;

        [[nodiscard]] virtual bool isStatement() const {
            return false;
        }

        [[nodiscard]] virtual bool isResource() const {
            return false;
        }

        [[nodiscard]] virtual bool isExpression() const {
            return false;
        }

        [[nodiscard]] virtual bool isValue() const {
            return false;
        }

        [[nodiscard]] virtual bool isLVal() const {
            return false;
        }

        [[nodiscard]] virtual bool isType() const {
            return false;
        }

        [[nodiscard]] virtual bool isBlock() const {
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

        [[nodiscard]] virtual bool isStatement() const override {
            return true;
        }

        [[nodiscard]] virtual StatementNode* copy() const override = 0;
    };


    class StatementListWrapper {
    public:
        StatementListWrapper() : _body(new StatementList()) {}

        StatementListWrapper(StatementList::iterator begin, StatementList::iterator end) : _body(new StatementList(begin, end)) {
            for ( auto stmt : *_body ) useref(stmt);
        }

        virtual ~StatementListWrapper() {
            for ( auto stmt : *_body ) freeref(stmt);
            delete _body;
        }

        /** Push a new statement to the end of the body. */
        void pushStatement(StatementNode* statement) {
            _body->push_back(useref(statement));
        }

        /**
         * Given a list of statements, concatentate them onto the
         * body and delete the original list container.
         */
        void assumeAndReduceStatements(StatementList* body) {
            for ( auto statement : *body ) {
                _body->push_back(statement);
            }

            delete body;
        }

        [[nodiscard]] StatementList* body() const {
            return _body;
        }

        [[nodiscard]] StatementList* copyBody() const {
            auto other = new StatementList();
            for ( auto stmt : *body() ) other->push_back(useref(stmt->copy()));
            return other;
        }

    protected:
        StatementList* _body;
    };

    /** AST node representing the root of the program. */
    class ProgramNode final : public ASTNode, public StatementListWrapper {
    public:
        ProgramNode() : ASTNode(new Position("", 0, 0, 0, 0)), StatementListWrapper() {}

        virtual ~ProgramNode() = default;

        [[nodiscard]] virtual ASTNodeTag getTag() const override {
            return ASTNodeTag::PROGRAM;
        }

        /**
         * Demote this program to a list of statements.
         * This destructs the current instance, so only the
         * returned StatementList is valid.
         */
        [[nodiscard]] StatementList* reduceToStatements() {
            StatementList* list = new StatementList();
            for ( auto stmt : *_body ) {
                useref(stmt);
                list->push_back(stmt);
            }
            delete this;
            return list;
        }

        [[nodiscard]] virtual std::string toString() const override {
            return "ProgramNode<#body: " + s(_body->size()) + ">";
        }

        [[nodiscard]] virtual ProgramNode* copy() const override;
    };

    /** AST node representing code that evaluates to a value. */
    class ExpressionNode : public ASTNode {
    public:
        ExpressionNode(Position* pos) : ASTNode(pos) {}
        virtual ~ExpressionNode() {}

        [[nodiscard]] virtual bool isExpression() const override {
            return true;
        }

        [[nodiscard]] virtual ExpressionNode* copy() const override = 0;

        [[nodiscard]] virtual Type::Type* type() const = 0;
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
        [[nodiscard]] virtual StatementExpressionNode* copy() const override = 0;
    };


    /**
     * AST node representing a statement of an expression. Only works for expressions
     * that derive StatementExpressionNode.
     */
    class ExpressionStatementNode final : public StatementNode {
    public:
        ExpressionStatementNode(Position* pos, StatementExpressionNode* exp) : StatementNode(pos), _exp(useref(exp)) {}
        virtual ~ExpressionStatementNode() {
            freeref(_exp);
        }

        [[nodiscard]] virtual ASTNodeTag getTag() const override {
            return ASTNodeTag::EXPRESSIONSTATEMENT;
        }

        [[nodiscard]] virtual std::string toString() const override {
            return "ExpressionStatementNode<" + _exp->toString() + ">";
        }

        [[nodiscard]] StatementExpressionNode* expression() const {
            return _exp;
        }

        [[nodiscard]] virtual ExpressionStatementNode* copy() const override {
            return new ExpressionStatementNode(position(), _exp->copy());
        }

    protected:
        StatementExpressionNode* _exp;
    };


    /** Base class for AST nodes that can have a value assigned to them. */
    class LValNode : public ExpressionNode {
    public:
        LValNode(Position* pos) : ExpressionNode(pos) {}
        virtual ~LValNode() {}

        [[nodiscard]] bool isLVal() const override {
            return true;
        }

        [[nodiscard]] virtual bool shared() const = 0;

        [[nodiscard]] LValNode* copy() const override = 0;

        [[nodiscard]] Type::Type* type() const override = 0;
    };


    /** AST node representing an identifier. */
    class IdentifierNode final : public LValNode {
    public:
        IdentifierNode(Position* pos, std::string name) : LValNode(pos), _name(std::move(name)) {}
        ~IdentifierNode() {
            freeref(_symbol);
        }

        [[nodiscard]] const std::string name() { return _name; }

        [[nodiscard]] virtual ASTNodeTag getTag() const override {
            return ASTNodeTag::IDENTIFIER;
        }

        [[nodiscard]] virtual std::string  toString() const override {
            return "IdentifierNode<name: " + _name + ">";
        }

        /** Get the semantic symbol associated with this identifier in its scope. */
        [[nodiscard]] SemanticSymbol* symbol() const {
            return _symbol;
        }

        void overrideSymbol(SemanticSymbol* sym) {
            _symbol = swapref(_symbol, sym);
        }

        [[nodiscard]] virtual bool shared() const override {
            if (_symbol == nullptr) {
                throw Errors::SwarmError("Attempt to get sharedness of symbol-less identifier: " + _name);
            }

            return _symbol->shared();
        }

        [[nodiscard]] virtual IdentifierNode* copy() const override {
            auto other = new IdentifierNode(position(), _name);
            other->_symbol = useref(_symbol);
            return other;
        }

        [[nodiscard]] virtual Type::Type* type() const override {
            return _symbol->type();
        }

    protected:
        std::string _name;
        SemanticSymbol* _symbol = nullptr;

        friend class Walk::NameAnalysisWalk;
        friend class Walk::TypeAnalysisWalk;
        friend class Walk::DeSerializeWalk;
    };

    /** Node for accessing data from an array */
    class EnumerableAccessNode final : public LValNode {
    public:
        EnumerableAccessNode(Position* pos, ExpressionNode* path, ExpressionNode* index) : LValNode(pos), _path(useref(path)), _index(useref(index)) {}
        virtual ~EnumerableAccessNode() {
            freeref(_path);
            freeref(_index);
        }

        [[nodiscard]] virtual ASTNodeTag getTag() const override {
            return ASTNodeTag::ENUMERABLEACCESS;
        }

        [[nodiscard]] virtual std::string  toString() const override {
            std::stringstream s;
            s << "EnumerableAccessNode<path: " << _path->toString() << ", index: " << _index->toString() << ">";
            return s.str();
        }

        [[nodiscard]] ExpressionNode* path() const {
            return _path;
        }

        [[nodiscard]] ExpressionNode* index() const {
            return _index;
        }

        [[nodiscard]] bool shared() const override {
            if ( _path->isLVal() ) return ((LValNode*)_path)->shared();
            return false;
        }

        [[nodiscard]] virtual EnumerableAccessNode* copy() const override {
            return new EnumerableAccessNode(position(), _path->copy(), _index->copy());
        }

        [[nodiscard]] virtual Type::Type* type() const override {
            auto baseType = _path->type();
            assert(baseType->intrinsic() == Type::Intrinsic::ENUMERABLE);
            return ((Type::Enumerable*) baseType)->values();
        }
    private:
        ExpressionNode* _path;
        ExpressionNode* _index;
    };

    /** Node for appending data to an array */
    class EnumerableAppendNode final : public LValNode {
    public:
        EnumerableAppendNode(Position* pos, ExpressionNode* path) : LValNode(pos), _path(useref(path)) {}
        virtual ~EnumerableAppendNode() {
            freeref(_path);
        }

        [[nodiscard]] virtual ASTNodeTag getTag() const override {
            return ASTNodeTag::ENUMERABLEAPPEND;
        }

        [[nodiscard]] virtual std::string  toString() const override {
            std::stringstream s;
            s << "EnumerableAppendNode<path: " << _path->toString() << ">";
            return s.str();
        }

        [[nodiscard]] ExpressionNode* path() const {
            return _path;
        }

        [[nodiscard]] bool shared() const override {
            if ( _path->isLVal() ) return ((LValNode*)_path)->shared();
            return false;
        }

        [[nodiscard]] virtual EnumerableAppendNode* copy() const override {
            return new EnumerableAppendNode(position(), _path->copy());
        }

        [[nodiscard]] virtual Type::Type* type() const override {
            auto baseType = _path->type();
            assert(baseType->intrinsic() == Type::Intrinsic::ENUMERABLE);
            return ((Type::Enumerable*) baseType)->values();
        }
    private:
        ExpressionNode* _path;
    };

    /** Node for accessing data from a map */
    class MapAccessNode final : public LValNode {
    public:
        MapAccessNode(Position* pos, ExpressionNode* path, IdentifierNode* end) : LValNode(pos), _path(useref(path)), _end(useref(end)) {}
        virtual ~MapAccessNode() {
            freeref(_path);
            freeref(_end);
        }

        [[nodiscard]] virtual ASTNodeTag getTag() const override {
            return ASTNodeTag::MAPACCESS;
        }

        [[nodiscard]] virtual std::string  toString() const override {
            return "MapAccessNode<path: " + _path->toString() + " id: " + _end->name() + ">";
        }

        [[nodiscard]] ExpressionNode* path() const {
            return _path;
        }

        [[nodiscard]] IdentifierNode* end() const {
            return _end;
        }

        [[nodiscard]] bool shared() const override {
            if ( _path->isLVal() ) return ((LValNode*)_path)->shared();
            return false;
        }

        [[nodiscard]] virtual MapAccessNode* copy() const override {
            return new MapAccessNode(position(), _path->copy(), _end->copy());
        }

        [[nodiscard]] virtual Type::Type* type() const override {
            auto pathType = _path->type();
            assert(pathType->intrinsic() == Type::Intrinsic::MAP);
            return ((Type::Map*) pathType)->values();
        }
    private:
        ExpressionNode* _path;
        IdentifierNode* _end;
    };

    class ClassAccessNode : public LValNode {
    public:
        ClassAccessNode(Position* pos, ExpressionNode* path, IdentifierNode* end) : LValNode(pos), _path(useref(path)), _end(useref(end)) {}
        ~ClassAccessNode() {
            freeref(_path);
            freeref(_end);
        }

        [[nodiscard]] ASTNodeTag getTag() const override {
            return ASTNodeTag::CLASSACCESS;
        }

        [[nodiscard]] std::string toString() const override {
            return "ClassAccessNode<path: " + _path->toString() + " id: " + _end->name() + ">";
        }

        [[nodiscard]] ExpressionNode* path() const {
            return _path;
        }

        [[nodiscard]] IdentifierNode* end() const {
            return _end;
        }

        [[nodiscard]] bool shared() const override {
            if ( _path->isLVal() ) return ((LValNode*)_path)->shared();
            return false;
        }

        [[nodiscard]] virtual ClassAccessNode* copy() const override {
            return new ClassAccessNode(position(), _path->copy(), _end->copy());
        }

        [[nodiscard]] virtual Type::Type* type() const override {
            auto pathType = _path->type();
            assert(pathType->intrinsic() == Type::Intrinsic::OBJECT);
            return ((Type::Object*) pathType)->getProperty(_end->name());
        }
    private:
        ExpressionNode* _path;
        IdentifierNode* _end;
    };

    class IncludeStatementNode final : public StatementNode {
    public:
        IncludeStatementNode(Position* pos, ClassAccessNode* path, std::vector<IdentifierNode*>* ids) : StatementNode(pos), _path(useref(path)), _identifiers(ids) {}
        virtual ~IncludeStatementNode() {
            freeref(_path);
            if ( _identifiers != nullptr ) {
                for (auto i : *_identifiers) freeref(i);
                delete _identifiers;
            }
        }

        [[nodiscard]] ASTNodeTag getTag() const override {
            return ASTNodeTag::INCLUDE;
        }

        [[nodiscard]] std::string toString() const override {
            return "IncludeStatementNode<" + _path->toString() + ">";
        }

        [[nodiscard]] IncludeStatementNode* copy() const override {
            auto ids = new std::vector<IdentifierNode*>();
            for ( auto id : *_identifiers ) {
                ids->push_back(useref(id->copy()));
            }
            return new IncludeStatementNode(position(), _path->copy(), ids);
        }

        [[nodiscard]] ClassAccessNode* path() const {
            return _path;
        }

        [[nodiscard]] std::vector<IdentifierNode*>* identifiers() const {
            return _identifiers;
        }
    private:
        ClassAccessNode* _path;
        std::vector<IdentifierNode*>* _identifiers;
    };

    class TypeLiteral : public ExpressionNode {
    public:
        TypeLiteral(Position* pos, swarmc::Type::Type* type) : ExpressionNode(pos), _type(useref(type)) {}

        ~TypeLiteral() {
            freeref(_type);
        }

        [[nodiscard]] TypeLiteral* copy() const override {
            return new TypeLiteral(position(), _type);
        }

        [[nodiscard]] ASTNodeTag getTag() const override {
            return ASTNodeTag::TYPELITERAL;
        }

        [[nodiscard]] Type::Type* type() const override {
            return Type::Primitive::of(Type::Intrinsic::TYPE);
        }

        [[nodiscard]] Type::Type* value() const {
            return _type;
        }

        bool disambiguateValue() {
            auto temp = _type;
            try {
                _type = useref(_type->disambiguateStatically());
                freeref(temp);
            } catch (swarmc::Errors::SwarmError& e) {
                std::string m(e.what());
                Logging::get()->get("Name Analysis")->error(s(this->position()) + " " + m);
                return false;
            }
            return true;
        }

        [[nodiscard]] std::string toString() const override {
            return "Type<" + _type->toString() + ">";
        }
    protected:
        swarmc::Type::Type* _type;

        friend Walk::NameAnalysisWalk;
        friend Walk::TypeAnalysisWalk;
    };

    /** AST node referencing a literal boolean value. */
    class BooleanLiteralExpressionNode final : public ExpressionNode {
    public:
        BooleanLiteralExpressionNode(Position* pos, const bool val) : ExpressionNode(pos), _val(std::move(val)) {}

        [[nodiscard]] virtual ASTNodeTag getTag() const override {
            return ASTNodeTag::BOOLEANLITERAL;
        }

        [[nodiscard]] virtual std::string  toString() const override {
            return "BoolLiteralNode<of: " + s(_val) + ">";
        }

        /** Get the value of the literal expression. */
        [[nodiscard]] virtual bool value() const {
            return _val;
        }

        [[nodiscard]] virtual bool isValue() const override {
            return true;
        }

        [[nodiscard]] virtual BooleanLiteralExpressionNode* copy() const override {
            return new BooleanLiteralExpressionNode(position(), _val);
        }

        [[nodiscard]] virtual Type::Type* type() const override {
            return Type::Primitive::of(Type::Intrinsic::BOOLEAN);
        }

    private:
        const bool _val;
    };

        /** AST node representing literal strings. */
    class StringLiteralExpressionNode final : public ExpressionNode {
    public:
        StringLiteralExpressionNode(Position* pos, std::string value) : ExpressionNode(pos), _value(std::move(value)) {}
        virtual ~StringLiteralExpressionNode() {}

        [[nodiscard]] virtual ASTNodeTag getTag() const override {
            return ASTNodeTag::STRINGLITERAL;
        }

        [[nodiscard]] std::string toString() const override {
            return "StringLiteralExpressionNode<#value: '" + _value + "'>";
        }

        [[nodiscard]] std::string value() {
            return _value;
        }

        [[nodiscard]] virtual bool isValue() const override {
            return true;
        }

        [[nodiscard]] virtual StringLiteralExpressionNode* copy() const override {
            return new StringLiteralExpressionNode(position(), _value);
        }

        [[nodiscard]] virtual Type::Type* type() const override {
            return Type::Primitive::of(Type::Intrinsic::STRING);
        }
    protected:
        std::string _value;
    };

    /** AST node representing literal numbers. */
    class NumberLiteralExpressionNode : public ExpressionNode {
    public:
        NumberLiteralExpressionNode(Position* pos, double value) : ExpressionNode(pos), _value(std::move(value)) {}
        virtual ~NumberLiteralExpressionNode() {}

        [[nodiscard]] virtual ASTNodeTag getTag() const override {
            return ASTNodeTag::NUMBERLITERAL;
        }

        [[nodiscard]] std::string toString() const override {
            return "NumberLiteralExpressionNode<#value: " + s(_value) + ">";
        }

        [[nodiscard]] double value() const {
            return _value;
        }

        [[nodiscard]] virtual bool isValue() const override {
            return true;
        }

        [[nodiscard]] virtual NumberLiteralExpressionNode* copy() const override {
            return new NumberLiteralExpressionNode(position(), _value);
        }

        [[nodiscard]] virtual Type::Type* type() const override {
            return Type::Primitive::of(Type::Intrinsic::NUMBER);
        }
    protected:
        double _value;
    };


    /** AST node representing literal enumerations. */
    class EnumerationLiteralExpressionNode final : public ExpressionNode {
    public:
        EnumerationLiteralExpressionNode(Position* pos, ExpressionList* actuals) : ExpressionNode(pos), _actuals(actuals), _type(nullptr) {}
        EnumerationLiteralExpressionNode(Position* pos, ExpressionList* actuals, TypeLiteral* type) : ExpressionNode(pos), _actuals(actuals), _type(useref(type)) {}
        virtual ~EnumerationLiteralExpressionNode() {
            freeref(_type);
            for ( auto a : *_actuals ) freeref(a);
            delete _actuals;
        }

        [[nodiscard]] std::string toString() const override {
            return "EnumerationLiteralExpressionNode<#actuals: " + s(_actuals->size()) + ">";
        }

        [[nodiscard]] virtual ASTNodeTag getTag() const override {
            return ASTNodeTag::ENUMERATIONLITERAL;
        }

        [[nodiscard]] ExpressionList* actuals() const {
            return _actuals;
        }

        [[nodiscard]] virtual bool isValue() const override {
            return true;
        }

        [[nodiscard]] virtual bool isEmpty() const {
            return _actuals->empty();
        }

        [[nodiscard]] virtual bool hasIndex(size_t idx) const {
            return idx < _actuals->size();
        }

        [[nodiscard]] virtual ExpressionNode* getIndex(size_t idx) const {
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

        [[nodiscard]] virtual EnumerationLiteralExpressionNode* copy() const override {
            auto actuals = new ExpressionList;
            for ( auto actual : *_actuals ) actuals->push_back(useref(actual->copy()));

            return new EnumerationLiteralExpressionNode(position(), actuals, _type->copy());
        }

        [[nodiscard]] virtual Type::Type* type() const override {
            assert(_type != nullptr);
            return _type->value();
        }

    protected:
        ExpressionList* _actuals;
        TypeLiteral* _type;

        friend class Walk::TypeAnalysisWalk;
    };

        /** AST node referencing one entry in a map. */
    class MapStatementNode final : public ASTNode {
    public:
        MapStatementNode(Position* pos, IdentifierNode* id, ExpressionNode* value) : ASTNode(pos), _id(useref(id)), _value(useref(value)) {}
        virtual ~MapStatementNode() {
            freeref(_id);
            freeref(_value);
        }

        [[nodiscard]] virtual ASTNodeTag getTag() const override {
            return ASTNodeTag::MAPSTATEMENT;
        }

        [[nodiscard]] virtual std::string  toString() const override {
            return "MapStatementNode<id: " + _id->name() + ">";
        }

        /**
         * Get the identifier for the key of this entry.
         * NOTE: This identifier is *NOT* a "real" identifier in the sense that it
         *       has no semantic symbol attached to it and, therefore, no type directly.
         *       Instead, its type is inferred from the type of the map.
         */
        [[nodiscard]] virtual IdentifierNode* id() const {
            return _id;
        }

        [[nodiscard]] ExpressionNode* value() const {
            return _value;
        }

        void setValue(ExpressionNode* value) {
            assert(value->isValue());
            _value = value;
        }

        [[nodiscard]] virtual MapStatementNode* copy() const override {
            return new MapStatementNode(position(), _id->copy(), _value->copy());
        }

    protected:
        IdentifierNode* _id;
        ExpressionNode* _value;
    };


    /** AST node referencing a map literal. */
    class MapNode final : public ExpressionNode {
    public:
        MapNode(Position* pos, MapBody* body) : ExpressionNode(pos), _body(body), _type(nullptr) {}
        MapNode(Position* pos, MapBody* body, TypeLiteral* type) : ExpressionNode(pos), _body(body), _type(useref(type)) {}
        virtual ~MapNode() {
            freeref(_type);
            for ( auto stmt : *_body ) freeref(stmt);
            delete _body;
        }

        [[nodiscard]] virtual ASTNodeTag getTag() const override {
            return ASTNodeTag::MAPLITERAL;
        }

        [[nodiscard]] std::string toString() const override {
            return "MapNode<#body: " + s(_body->size()) + ">";
        }

        [[nodiscard]] MapBody* body() const {
            return _body;
        }

        [[nodiscard]] virtual bool isValue() const override {
            return true;
        }

        [[nodiscard]] virtual bool hasKey(IdentifierNode* name) const {
            return getBodyNode(name) != nullptr;
        }

        [[nodiscard]] virtual ExpressionNode* getKey(IdentifierNode* name) const {
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

        [[nodiscard]] virtual MapNode* copy() const override {
            auto body = new MapBody;
            for ( auto entry : *_body ) body->push_back(useref(entry->copy()));
            return new MapNode(position(), body, _type->copy());
        }

        [[nodiscard]] virtual Type::Type* type() const override {
            assert(_type != nullptr);
            return _type->value();
        }

    protected:
        MapBody* _body;
        TypeLiteral* _type;

        [[nodiscard]] virtual MapStatementNode* getBodyNode(IdentifierNode* name) const {
            for ( auto stmt : *_body ) {
                if ( stmt->id()->name() == name->name() ) {
                    return stmt;
                }
            }

            return nullptr;
        }

        friend class Walk::TypeAnalysisWalk;
    };


    /** AST node representing an assignment of a value to an lval. */
    class AssignExpressionNode : public StatementExpressionNode {
    public:
        AssignExpressionNode(Position* pos, LValNode* dest, ExpressionNode* value) : StatementExpressionNode(pos), _dest(useref(dest)), _value(useref(value)) {}
        virtual ~AssignExpressionNode() {
            freeref(_dest);
            freeref(_value);
        }

        [[nodiscard]] virtual std::string  toString() const override {
            return "AssignExpressionNode<lval: " + _dest->toString() + ">";
        }

        [[nodiscard]] virtual ASTNodeTag getTag() const override {
            return ASTNodeTag::ASSIGN;
        }

        [[nodiscard]] virtual LValNode* dest() const {
            return _dest;
        }

        [[nodiscard]] virtual ExpressionNode* value() const {
            return _value;
        }

        [[nodiscard]] virtual AssignExpressionNode* copy() const override {
            return new AssignExpressionNode(position(), _dest->copy(), _value->copy());
        }

        [[nodiscard]] virtual Type::Type* type() const override {
            return _value->type();
        }

    protected:
        LValNode* _dest;
        ExpressionNode* _value;
    };


    /** Base class for AST nodes declaring things in scope. */
    class DeclarationNode : public StatementNode {
    public:
        DeclarationNode(Position* pos) : StatementNode(pos) {}
        virtual ~DeclarationNode() {}
        [[nodiscard]] virtual DeclarationNode* copy() const override = 0;
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
        VariableDeclarationNode(Position* pos, TypeLiteral* type, AssignExpressionNode* assignment, bool shared)
            : DeclarationNode(pos), _type(useref(type)), _assignment(useref(assignment)), _shared(std::move(shared)) {}

        virtual ~VariableDeclarationNode() {
            freeref(_type);
            freeref(_assignment);
        }

        [[nodiscard]] virtual ASTNodeTag getTag() const override {
            return ASTNodeTag::VARIABLEDECLARATION;
        }

        [[nodiscard]] virtual std::string  toString() const override {
            return "VariableDeclarationNode<name: " + id()->name() + ", shared:" +
                (_shared ? "true" : "false") + ">";
        }

        [[nodiscard]] IdentifierNode* id() const {
            assert(_assignment->dest()->getTag() == ASTNodeTag::IDENTIFIER);
            return (IdentifierNode*)_assignment->dest();
        }

        [[nodiscard]] ExpressionNode* value() const {
            return _assignment->value();
        }

        [[nodiscard]] AssignExpressionNode* assignment() const {
            return _assignment;
        }

        [[nodiscard]] bool shared() const {
            return _shared;
        }

        [[nodiscard]] TypeLiteral* typeNode() const {
            return _type;
        }

        [[nodiscard]] virtual VariableDeclarationNode* copy() const override {
            return new VariableDeclarationNode(
                position(),
                _type->copy(),
                _assignment->copy(),
                _shared
            );
        }

    protected:
        TypeLiteral* _type;
        AssignExpressionNode* _assignment;
        bool _shared;
    };

    class UninitializedVariableDeclarationNode final : public DeclarationNode {
    public:
        UninitializedVariableDeclarationNode(Position* pos, TypeLiteral* type, IdentifierNode* id) : DeclarationNode(pos), _type(useref(type)), _id(useref(id)) {}

        virtual ~UninitializedVariableDeclarationNode() {
            freeref(_type);
            freeref(_id);
        }

        [[nodiscard]] virtual ASTNodeTag getTag() const override {
            return ASTNodeTag::UNINITIALIZEDVARIABLEDECLARATION;
        }

        [[nodiscard]] virtual std::string  toString() const override {
            return "UninitializedVariableDeclarationNode<name: " + _id->name() + ">";
        }

        [[nodiscard]] IdentifierNode* id() const {
            return _id;
        }

        [[nodiscard]] TypeLiteral* typeNode() const {
            return _type;
        }

        [[nodiscard]] virtual UninitializedVariableDeclarationNode* copy() const override {
            return new UninitializedVariableDeclarationNode(
                position(),
                _type->copy(),
                _id->copy()
            );
        }

    protected:
        TypeLiteral* _type;
        IdentifierNode* _id;
    };

    class UseNode final : public DeclarationNode {
    public:
        UseNode(Position* pos, std::vector<IdentifierNode*>* ids) : DeclarationNode(pos), _ids(ids) {}

        ~UseNode() {
            for ( auto id : *_ids ) freeref(id);
            delete _ids;
        }

        [[nodiscard]] ASTNodeTag getTag() const override {
            return ASTNodeTag::USE;
        }

        [[nodiscard]] std::vector<IdentifierNode*>* ids() const {
            return _ids;
        }

        [[nodiscard]] UseNode* copy() const override {
            auto ids = new std::vector<IdentifierNode*>();
            for ( auto id : *_ids ) {
                ids->push_back(useref(id->copy()));
            }
            return new UseNode(
                position(),
                ids
            );
        }

        [[nodiscard]] std::string toString() const override {
            return "UseNode<#ids: " + s(_ids->size()) + ">";
        }
    private:
        std::vector<IdentifierNode*>* _ids;
    };

    class ReturnStatementNode : public StatementNode {
    public:
        ReturnStatementNode(Position* pos, ExpressionNode* value) : StatementNode(pos), _value(useref(value)) {}
        virtual ~ReturnStatementNode() {
            freeref(_value);
        }

        [[nodiscard]] virtual std::string  toString() const override {
            if (_value == nullptr) {
                return "ReturnStatementNode<>";
            }
            return "ReturnStatementNode<lval: " + _value->toString() + ">";
        }

        [[nodiscard]] virtual ASTNodeTag getTag() const override {
            return ASTNodeTag::RETURN;
        }

        [[nodiscard]] virtual ReturnStatementNode* copy() const override {
            return new ReturnStatementNode(position(), _value->copy());
        }

        [[nodiscard]] ExpressionNode* value() const {
            return _value;
        }
    private:
        ExpressionNode* _value;
    };

    class FunctionNode : public ExpressionNode, public StatementListWrapper {
    public:
        FunctionNode(Position* pos, TypeLiteral* type, FormalList* formals)
            : ExpressionNode(pos), StatementListWrapper(), _type(useref(type)), 
            _formals(formals), _symbols(new std::vector<VariableSymbol*>()) {}

        virtual ~FunctionNode() {
            for ( auto f : *_formals ) {
                freeref(f.first);
                freeref(f.second);
            }
            delete _formals;
            freeref(_type);
            for ( auto s : *_symbols ) freeref(s);
            delete _symbols;
        }

        [[nodiscard]] virtual ASTNodeTag getTag() const override {
            return ASTNodeTag::FUNCTION;
        }

        [[nodiscard]] virtual std::string toString() const override {
            return "FunctionNode<type: " + _type->toString() + ">";
        }

        [[nodiscard]] TypeLiteral* typeNode() const {
            return _type;
        }

        [[nodiscard]] FormalList* formals() const {
            return _formals;
        }

        [[nodiscard]] std::vector<VariableSymbol*>* usedSymbols() const {
            return _symbols;
        }

        void appendUsedSymbol(VariableSymbol* sym) {
            _symbols->push_back(useref(sym));
        }

        void setUsedSymbols(UsedSymbols& symbols) {
            for ( auto sym : *_symbols ) {
                if ( stl::contains(symbols.second, sym) ) {
                    symbols.second.erase(sym);
                    freeref(sym);
                }
            }
            _symbols->clear();
            for ( auto s : symbols.second ) {
                _symbols->push_back(useref(s));
            }
        }

        [[nodiscard]] virtual Type::Type* type() const override {
            return _type->value();
        }

        [[nodiscard]] virtual FunctionNode* copy() const override {
            auto formals = new FormalList();
            for ( auto f : *_formals ) {
                formals->push_back(
                    std::pair<TypeLiteral*, IdentifierNode*>(
                        useref(f.first->copy()), useref(f.second->copy())));
            }

            auto fn = new FunctionNode(position(), _type->copy(), formals);
            fn->_body = copyBody();

            for ( auto s : *_symbols ) {
                fn->_symbols->push_back(useref(s));
            }

            return fn;
        }
    protected:
        TypeLiteral* _type;
        FormalList* _formals;
        std::vector<VariableSymbol*>* _symbols;
    };

    class ConstructorNode final : public DeclarationNode {
    public:
        ConstructorNode(Position* pos, FunctionNode* func, ExpressionList* parentCons) : DeclarationNode(pos), _func(useref(func)), _parentConstructors(parentCons), _partOfType(nullptr) {
            _name = "constructor" + s(++ConstructorNode::nameID);
        }
        ~ConstructorNode() {
            freeref(_func);
            freeref(_partOfType);
            for ( auto c : *_parentConstructors ) freeref(c);
            delete _parentConstructors;
        }

        [[nodiscard]] virtual ASTNodeTag getTag() const override {
            return ASTNodeTag::CONSTRUCTOR;
        }

        [[nodiscard]] virtual std::string  toString() const override {
            return "ConstructorNode<name: " + _name + ">";
        }

        [[nodiscard]] virtual ConstructorNode* copy() const override {
            ExpressionList* parents = new ExpressionList();
            for ( auto c : *_parentConstructors ) parents->push_back(useref(c->copy()));
            return new ConstructorNode(
                position(),
                _func->copy(),
                parents
            );
        }

        [[nodiscard]] FunctionNode* func() const {
            return _func;
        }

        [[nodiscard]] ExpressionList* parentConstructors() const {
            return _parentConstructors;
        }

        [[nodiscard]] std::string name() const {
            return _name;
        }

        [[nodiscard]] Type::Object* partOf() const {
            return _partOfType;
        }
    protected:
        FunctionNode* _func;
        ExpressionList* _parentConstructors;
        Type::Object* _partOfType;
        std::string _name;
        static size_t nameID;

        friend swarmc::Lang::TypeBodyNode;
    };

    class TypeBodyNode final : public TypeLiteral {
    public:
        TypeBodyNode(Position* pos, DeclarationList* decls) : TypeLiteral(pos, nullptr),
            _declarations(new DeclarationList()),
            _parents(new DeclarationList()),
            _constructors(new std::vector<ConstructorNode*>())
        {
            for ( auto d : *decls ) {
                if ( d->getTag() == ASTNodeTag::CONSTRUCTOR ) {
                    _constructors->push_back((ConstructorNode*)d);
                } else if ( d->getTag() == ASTNodeTag::USE ) {
                    _parents->push_back(d);
                } else {
                    _declarations->push_back(d);
                }
            }
            // FIXME: remove once multiinheritance supported
            if ( _parents->size() > 1 || (_parents->size() == 1 && ((UseNode*)_parents->at(0))->ids()->size() > 1) ) {
                throw Errors::SwarmError(s(pos) + " Swarm currently only support single inheritance!");
            }
            // default constructor
            if ( _constructors->size() == 0 ) {
                _constructors->push_back(useref(new ConstructorNode(
                    pos,
                    new FunctionNode(
                        pos,
                        new TypeLiteral(pos, new Type::Lambda0(Type::Primitive::of(Type::Intrinsic::VOID))),
                        new FormalList()
                    ),
                    new ExpressionList()
                )));
            } 
            delete decls;
        }

        ~TypeBodyNode() {
            for (auto d : *_declarations) freeref(d);
            delete _declarations;
            for (auto p : *_parents) freeref(p);
            delete _parents;
            for (auto c : *_constructors) freeref(c);
            delete _constructors;
        }

        void setType(Type::Object* type) {
            if ( _type != nullptr ) {
                throw Errors::SwarmError("Attempt to reassign value of Type Body");
            }
            _type = useref(type);
            for ( auto c : *_constructors ) {
                c->_partOfType = useref(type);
            }
        }

        [[nodiscard]] ASTNodeTag getTag() const override {
            return ASTNodeTag::TYPEBODY;
        }

        [[nodiscard]] std::string toString() const override {
            if ( _type == nullptr ) return "TypeBodyNode<>";
            return "TypeBodyNode<#type:" + _type->toString() + ">";
        }

        [[nodiscard]] TypeBodyNode* copy() const override {
            auto decls = new DeclarationList();
            for ( auto d : *_declarations ) {
                decls->push_back(useref(d->copy()));
            }
            for ( auto p : *_parents ) {
                decls->push_back(useref(p->copy()));
            }
            for ( auto c : *_constructors ) {
                decls->push_back(useref(c->copy()));
            }
            auto t = new TypeBodyNode(position(), decls);
            if ( _type != nullptr ) {
                t->setType(((Type::Object*)_type)->copy());
            }
            return t;
        }

        [[nodiscard]] DeclarationList* declarations() const {
            return _declarations;
        }

        [[nodiscard]] DeclarationList* parents() const {
            return _parents;
        }

        [[nodiscard]] std::vector<ConstructorNode*>* constructors() const {
            return _constructors;
        }

    private:
        DeclarationList* _declarations;
        DeclarationList* _parents;
        std::vector<ConstructorNode*>* _constructors;
    };

    /** AST node representing a call to a function. */
    class CallExpressionNode final : public StatementExpressionNode {
    public:
        CallExpressionNode(Position* pos, ExpressionNode* func, std::vector<ExpressionNode*>* args) : StatementExpressionNode(pos), _func(useref(func)), _args(args) {}
        virtual ~CallExpressionNode() {
            freeref(_func);
            freeref(_type);
            freeref(_constructor);
            for ( auto arg : *_args ) freeref(arg);
            delete _args;
        }

        [[nodiscard]] virtual ASTNodeTag getTag() const override {
            return ASTNodeTag::CALL;
        }

        [[nodiscard]] virtual std::string  toString() const override {
            return "CallExpressionNode<#func: " + _func->toString() + ",#args: " + s(_args->size()) + ">";
        }

        [[nodiscard]] ExpressionNode* func() const {
            return _func;
        }

        [[nodiscard]] std::vector<ExpressionNode*>* args() const {
            return _args;
        }

        [[nodiscard]] virtual CallExpressionNode* copy() const override {
            auto args = new std::vector<ExpressionNode*>;
            for ( auto arg : *_args ) {
                args->push_back(useref(arg->copy()));
            }

            return new CallExpressionNode(position(), _func->copy(), args);
        }

        [[nodiscard]] virtual Type::Type* type() const override {
            return _type;
        }

        [[nodiscard]] ConstructorNode* constructor() const {
            return _constructor;
        }

    protected:
        ExpressionNode* _func;
        ConstructorNode* _constructor = nullptr;
        std::vector<ExpressionNode*>* _args;
        Type::Type* _type = nullptr;

        friend Walk::TypeAnalysisWalk;
    };


    /** AST node referencing deferment of a function call */
    class DeferCallExpressionNode final : public StatementExpressionNode {
    public:
        DeferCallExpressionNode(Position* pos, CallExpressionNode* call) : StatementExpressionNode(pos), _call(useref(call)) {}

        [[nodiscard]] virtual ASTNodeTag getTag() const override {
            return ASTNodeTag::DEFERCALL;
        }

        [[nodiscard]] virtual std::string  toString() const override {
            return "DeferCallExpressionNode<#call: " + _call->toString() + ">";
        }

        [[nodiscard]] virtual DeferCallExpressionNode* copy() const override {
            return new DeferCallExpressionNode(position(), _call->copy());
        }

        [[nodiscard]] virtual CallExpressionNode* call() const {
            return _call;
        }

        [[nodiscard]] virtual Type::Type* type() const override {
            return _call->type();
        }

    protected:
        CallExpressionNode* _call;
    };


    /** Base class for expressions of two operands. */
    class BinaryExpressionNode : public ExpressionNode {
    public:
        BinaryExpressionNode(Position* pos, ExpressionNode* left, ExpressionNode* right) : ExpressionNode(pos), _left(useref(left)), _right(useref(right)) {}
        virtual ~BinaryExpressionNode() {
            freeref(_left);
            freeref(_right);
        }

        [[nodiscard]] ExpressionNode* left() const {
            return _left;
        }

        [[nodiscard]] ExpressionNode* right() const {
            return _right;
        }

        [[nodiscard]] virtual BinaryExpressionNode* copy() const override = 0;
    protected:
        ExpressionNode* _left;
        ExpressionNode* _right;
    };


    /** Base class for expression nodes over static types. */
    class PureBinaryExpressionNode : public BinaryExpressionNode {
    public:
        PureBinaryExpressionNode(Position* pos, ExpressionNode* left, ExpressionNode* right) : BinaryExpressionNode(pos, left, right) {}
        virtual ~PureBinaryExpressionNode() {}

        [[nodiscard]] virtual Type::Type* leftType() const = 0;

        [[nodiscard]] virtual Type::Type* rightType() const = 0;

        [[nodiscard]] virtual Type::Type* resultType() const = 0;

        [[nodiscard]] virtual PureBinaryExpressionNode* copy() const override = 0;

        [[nodiscard]] virtual Type::Type* type() const override {
            return resultType();
        }
    };


    /** Base class for binary nodes that take bool -> bool -> bool */
    class PureBooleanBinaryExpressionNode : public PureBinaryExpressionNode {
    public:
        PureBooleanBinaryExpressionNode(Position* pos, ExpressionNode* left, ExpressionNode* right) : PureBinaryExpressionNode(pos, left, right) {}
        virtual ~PureBooleanBinaryExpressionNode() {}

        [[nodiscard]] virtual Type::Type* leftType() const override {
            return Type::Primitive::of(Type::Intrinsic::BOOLEAN);
        }

        [[nodiscard]] virtual Type::Type* rightType() const override {
            return Type::Primitive::of(Type::Intrinsic::BOOLEAN);
        }

        [[nodiscard]] virtual Type::Type* resultType() const override {
            return Type::Primitive::of(Type::Intrinsic::BOOLEAN);
        }

        [[nodiscard]] virtual PureBooleanBinaryExpressionNode* copy() const override = 0;
    };


    /** Base class for binary nodes that take num -> num -> num */
    class PureNumberBinaryExpressionNode : public PureBinaryExpressionNode {
    public:
        PureNumberBinaryExpressionNode(Position* pos, ExpressionNode* left, ExpressionNode* right) : PureBinaryExpressionNode(pos, left, right) {}
        virtual ~PureNumberBinaryExpressionNode() {}

        [[nodiscard]] virtual Type::Type* leftType() const override {
            return Type::Primitive::of(Type::Intrinsic::NUMBER);
        }

        [[nodiscard]] virtual Type::Type* rightType() const override {
            return Type::Primitive::of(Type::Intrinsic::NUMBER);
        }

        [[nodiscard]] virtual Type::Type* resultType() const override {
            return Type::Primitive::of(Type::Intrinsic::NUMBER);
        }

        [[nodiscard]] virtual PureNumberBinaryExpressionNode* copy() const override = 0;
    };


    /** Base class for binary nodes that take string -> string -> string */
    class PureStringBinaryExpressionNode : public PureBinaryExpressionNode {
    public:
        PureStringBinaryExpressionNode(Position* pos, ExpressionNode* left, ExpressionNode* right) : PureBinaryExpressionNode(pos, left, right) {}
        virtual ~PureStringBinaryExpressionNode() {}

        [[nodiscard]] virtual Type::Type* leftType() const override {
            return Type::Primitive::of(Type::Intrinsic::STRING);
        }

        [[nodiscard]] virtual Type::Type* rightType() const override {
            return Type::Primitive::of(Type::Intrinsic::STRING);
        }

        [[nodiscard]] virtual Type::Type* resultType() const override {
            return Type::Primitive::of(Type::Intrinsic::STRING);
        }

        [[nodiscard]] virtual PureStringBinaryExpressionNode* copy() const override = 0;
    };


    /** AST node referencing boolean AND of two expressions. */
    class AndNode final : public PureBooleanBinaryExpressionNode {
    public:
        AndNode(Position* pos, ExpressionNode* left, ExpressionNode* right): PureBooleanBinaryExpressionNode(pos, left, right) {}

        [[nodiscard]] virtual ASTNodeTag getTag() const override {
            return ASTNodeTag::AND;
        }

        [[nodiscard]] virtual std::string  toString() const override {
            return "AndNode<>";
        }

        [[nodiscard]] virtual AndNode* copy() const override {
            return new AndNode(position(), _left->copy(), _right->copy());
        }
    };


    /** AST node referencing boolean OR of two expressions. */
    class OrNode final : public PureBooleanBinaryExpressionNode {
    public:
        OrNode(Position* pos, ExpressionNode* left, ExpressionNode* right): PureBooleanBinaryExpressionNode(pos, left, right) {}

        [[nodiscard]] virtual ASTNodeTag getTag() const override {
            return ASTNodeTag::OR;
        }

        [[nodiscard]] virtual std::string  toString() const override {
            return "OrNode<>";
        }

        [[nodiscard]] virtual OrNode* copy() const override {
            return new OrNode(position(), _left->copy(), _right->copy());
        }
    };


    /** AST node referencing equality comparison of two expressions. */
    class EqualsNode final : public BinaryExpressionNode {
    public:
        EqualsNode(Position* pos, ExpressionNode* left, ExpressionNode* right) : BinaryExpressionNode(pos, left, right) {}

        [[nodiscard]] virtual ASTNodeTag getTag() const override {
            return ASTNodeTag::EQUALS;
        }

        [[nodiscard]] virtual std::string  toString() const override {
            return "EqualsNode<>";
        }

        [[nodiscard]] virtual EqualsNode* copy() const override {
            return new EqualsNode(position(), _left->copy(), _right->copy());
        }

        [[nodiscard]] virtual Type::Type* type() const override {
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
        NumericComparisonExpressionNode(Position* pos, NumberComparisonType comparisonType, ExpressionNode* left, ExpressionNode* right) : PureBinaryExpressionNode(pos, left, right), _comparisonType(std::move(comparisonType)) {}

        [[nodiscard]] Type::Type* leftType() const override {
            return Type::Primitive::of(Type::Intrinsic::NUMBER);
        }

        [[nodiscard]] Type::Type* rightType() const override {
            return Type::Primitive::of(Type::Intrinsic::NUMBER);
        }

        [[nodiscard]] Type::Type* resultType() const override {
            return Type::Primitive::of(Type::Intrinsic::BOOLEAN);
        }

        [[nodiscard]] PureBinaryExpressionNode* copy() const override {
            return new NumericComparisonExpressionNode(position(), _comparisonType, _left->copy(), _right->copy());
        }

        [[nodiscard]] ASTNodeTag getTag() const override {
            return ASTNodeTag::NUMERICCOMPARISON;
        }

        [[nodiscard]] std::string toString() const override {
            return "NumericComparisonExpressionNode<type: " + comparisonTypeToString() + ">";
        }

        [[nodiscard]] NumberComparisonType comparisonType() const {
            return _comparisonType;
        }

        [[nodiscard]] std::string comparisonTypeToString() const {
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

        [[nodiscard]] virtual ASTNodeTag getTag() const override {
            return ASTNodeTag::NOTEQUALS;
        }

        [[nodiscard]] virtual std::string  toString() const override {
            return "NotEqualsNode<>";
        }

        [[nodiscard]] virtual NotEqualsNode* copy() const override {
            return new NotEqualsNode(position(), _left->copy(), _right->copy());
        }

        [[nodiscard]] virtual Type::Type* type() const override {
            return Type::Primitive::of(Type::Intrinsic::BOOLEAN);
        }
    };

    /** AST node referencing addition of two values. */
    class AddNode final : public PureBinaryExpressionNode {
    public:
        AddNode(Position* pos, ExpressionNode* left, ExpressionNode* right) : PureBinaryExpressionNode(pos, left, right), _concatenation(false) {}

        [[nodiscard]] virtual ASTNodeTag getTag() const override {
            return ASTNodeTag::ADD;
        }

        [[nodiscard]] virtual std::string  toString() const override {
            return "AddNode<>";
        }

        [[nodiscard]] virtual Type::Type* leftType() const override {
            if ( _concatenation ) {
                return Type::Primitive::of(Type::Intrinsic::STRING);
            }
            return Type::Primitive::of(Type::Intrinsic::NUMBER);
        }

        [[nodiscard]] virtual Type::Type* rightType() const override {
            if ( _concatenation ) {
                return Type::Primitive::of(Type::Intrinsic::STRING);
            }
            return Type::Primitive::of(Type::Intrinsic::NUMBER);
        }

        [[nodiscard]] virtual Type::Type* resultType() const override {
            if ( _concatenation ) {
                return Type::Primitive::of(Type::Intrinsic::STRING);
            }
            return Type::Primitive::of(Type::Intrinsic::NUMBER);
        }

        [[nodiscard]] virtual AddNode* copy() const override {
            return new AddNode(position(), _left->copy(), _right->copy());
        }

        void setConcat(bool c) {
            _concatenation = c;
        }

        [[nodiscard]] bool concatenation() const {
            return _concatenation;
        }
    private:
        bool _concatenation;
    };

    /** AST node referencing subtraction of two values. */
    class SubtractNode final : public PureNumberBinaryExpressionNode {
    public:
        SubtractNode(Position* pos, ExpressionNode* left, ExpressionNode* right) : PureNumberBinaryExpressionNode(pos, left, right) {}

        [[nodiscard]] virtual ASTNodeTag getTag() const override {
            return ASTNodeTag::SUBTRACT;
        }

        [[nodiscard]] virtual std::string  toString() const override {
            return "SubtractNode<>";
        }

        [[nodiscard]] virtual SubtractNode* copy() const override {
            return new SubtractNode(position(), _left->copy(), _right->copy());
        }
    };


    /** AST node referencing multiplication of two values. */
    class MultiplyNode final : public PureNumberBinaryExpressionNode {
    public:
        MultiplyNode(Position* pos, ExpressionNode* left, ExpressionNode* right) : PureNumberBinaryExpressionNode(pos, left, right) {}

        [[nodiscard]] virtual ASTNodeTag getTag() const override {
            return ASTNodeTag::MULTIPLY;
        }

        [[nodiscard]] virtual std::string  toString() const override {
            return "MultiplyNode<>";
        }

        [[nodiscard]] virtual MultiplyNode* copy() const override {
            return new MultiplyNode(position(), _left->copy(), _right->copy());
        }
    };

    /** AST node referencing division of two values. */
    class DivideNode final : public PureNumberBinaryExpressionNode {
    public:
        DivideNode(Position* pos, ExpressionNode* left, ExpressionNode* right) : PureNumberBinaryExpressionNode(pos, left, right) {}

        [[nodiscard]] virtual ASTNodeTag getTag() const override {
            return ASTNodeTag::DIVIDE;
        }

        [[nodiscard]] virtual std::string  toString() const override {
            return "DivideNode<>";
        }

        [[nodiscard]] virtual DivideNode* copy() const override {
            return new DivideNode(position(), _left->copy(), _right->copy());
        }
    };


    /** AST node referencing the modulus of two values. */
    class ModulusNode final : public PureNumberBinaryExpressionNode {
    public:
        ModulusNode(Position* pos, ExpressionNode* left, ExpressionNode* right) : PureNumberBinaryExpressionNode(pos, left, right) {}

        [[nodiscard]] virtual ASTNodeTag getTag() const override {
            return ASTNodeTag::MODULUS;
        }

        [[nodiscard]] virtual std::string  toString() const override {
            return "ModulusNode<>";
        }

        [[nodiscard]] virtual ModulusNode* copy() const override {
            return new ModulusNode(position(), _left->copy(), _right->copy());
        }
    };


    /** AST node referencing the exponential of two values. */
    class PowerNode final : public PureNumberBinaryExpressionNode {
    public:
        PowerNode(Position* pos, ExpressionNode* left, ExpressionNode* right) : PureNumberBinaryExpressionNode(pos, left, right) {}

        [[nodiscard]] virtual ASTNodeTag getTag() const override {
            return ASTNodeTag::POWER;
        }

        [[nodiscard]] virtual std::string  toString() const override {
            return "PowerNode<>";
        }

        [[nodiscard]] virtual PowerNode* copy() const override {
            return new PowerNode(position(), _left->copy(), _right->copy());
        }
    };

    class NthRootNode final : public PureNumberBinaryExpressionNode {
    public:
        NthRootNode(Position* pos, ExpressionNode* n, ExpressionNode* exp) : PureNumberBinaryExpressionNode(pos, n, exp) {}

        [[nodiscard]] virtual ASTNodeTag getTag() const override {
            return ASTNodeTag::NTHROOT;
        }

        [[nodiscard]] virtual std::string  toString() const override {
            return "SqrtNode<>";
        }

        [[nodiscard]] virtual NthRootNode* copy() const override {
            return new NthRootNode(position(), _left->copy(), _right->copy());
        }

        [[nodiscard]] virtual Type::Type* type() const override {
            return Type::Primitive::of(Type::Intrinsic::NUMBER);
        }
    };

    /** Base class for expressions of one operand. */
    class UnaryExpressionNode : public ExpressionNode {
    public:
        UnaryExpressionNode(Position* pos, ExpressionNode* exp) : ExpressionNode(pos), _exp(useref(exp)) {}
        virtual ~UnaryExpressionNode() {
            freeref(_exp);
        }

        [[nodiscard]] ExpressionNode* exp() const {
            return _exp;
        }

        [[nodiscard]] virtual UnaryExpressionNode* copy() const override = 0;
    protected:
        ExpressionNode* _exp;
    };

    /** AST node referencing numeric negation of an expression */
    class NegativeExpressionNode final : public UnaryExpressionNode {
    public:
        NegativeExpressionNode(Position* pos, ExpressionNode* exp) : UnaryExpressionNode(pos, exp) {}

        [[nodiscard]] virtual ASTNodeTag getTag() const override {
            return ASTNodeTag::NEGATIVE;
        }

        [[nodiscard]] virtual std::string  toString() const override {
            return "NegativeExpressionNode<>";
        }

        [[nodiscard]] virtual NegativeExpressionNode* copy() const override {
            return new NegativeExpressionNode(position(), _exp->copy());
        }

        [[nodiscard]] virtual Type::Type* type() const override {
            return Type::Primitive::of(Type::Intrinsic::NUMBER);
        }
    };

    /** AST node referencing boolean negation of an expression. */
    class NotNode final : public UnaryExpressionNode {
    public:
        NotNode(Position* pos, ExpressionNode* exp) : UnaryExpressionNode(pos, exp) {}

        [[nodiscard]] virtual ASTNodeTag getTag() const override {
            return ASTNodeTag::NOT;
        }

        [[nodiscard]] virtual std::string  toString() const override {
            return "NotNode<>";
        }

        [[nodiscard]] virtual NotNode* copy() const override {
            return new NotNode(position(), _exp->copy());
        }

        [[nodiscard]] virtual Type::Type* type() const override {
            return Type::Primitive::of(Type::Intrinsic::BOOLEAN);
        }
    };


    /** Base class for AST nodes that contain a body of statements. */
    class BlockStatementNode : public StatementNode, public StatementListWrapper {
    public:
        BlockStatementNode(Position* pos) : StatementNode(pos), StatementListWrapper() {}

        virtual ~BlockStatementNode() = default;

        [[nodiscard]] virtual BlockStatementNode* copy() const override = 0;

        [[nodiscard]] bool isBlock() const override { return true; }
    };

    /** AST node representing an enumeration block. */
    class EnumerationStatement final : public BlockStatementNode {
    public:
        EnumerationStatement(Position* pos, ExpressionNode* enumerable, IdentifierNode* local, IdentifierNode* index, bool shared)
            : BlockStatementNode(pos), _enumerable(useref(enumerable)), _local(useref(local)), _index(useref(index)), _shared(std::move(shared)) {}
        EnumerationStatement(Position* pos, ExpressionNode* enumerable, IdentifierNode* local, bool shared)
            : BlockStatementNode(pos), _enumerable(useref(enumerable)), _local(useref(local)), _index(nullptr), _shared(std::move(shared)) {}

        virtual ~EnumerationStatement() {
            freeref(_enumerable);
            freeref(_local);
            freeref(_index);
        }

        [[nodiscard]] virtual ASTNodeTag getTag() const override {
            return ASTNodeTag::ENUMERATE;
        }

        [[nodiscard]] virtual std::string  toString() const override {
            return "EnumerationStatement<e: " + _enumerable->toString() + ", as: " + _local->name() + ", #body: " + s(_body->size()) + ">";
        }

        [[nodiscard]] ExpressionNode* enumerable() const {
            return _enumerable;
        }

        [[nodiscard]] IdentifierNode* local() const {
            return _local;
        }

        [[nodiscard]] IdentifierNode* index() const {
            return _index;
        }

        [[nodiscard]] bool shared() const {
            return _shared;
        }

        [[nodiscard]] virtual EnumerationStatement* copy() const override {
            EnumerationStatement* other;
            if (_index == nullptr) {
                other = new EnumerationStatement(position(), _enumerable->copy(),
                    _local->copy(), _shared);
            } else {
                other = new EnumerationStatement(position(), _enumerable->copy(),
                    _local->copy(), _index->copy(), _shared);
            }
            other->assumeAndReduceStatements(copyBody());
            return other;
        }
    protected:
        ExpressionNode* _enumerable;
        IdentifierNode* _local, * _index;
        bool _shared;

        friend class Walk::NameAnalysisWalk;
    };


    /** AST node representing a with-resource block. */
    class WithStatement final : public BlockStatementNode {
    public:
        WithStatement(Position* pos, ExpressionNode* resource, IdentifierNode* local, bool shared)
            : BlockStatementNode(pos), _resource(useref(resource)), _local(useref(local)), _shared(std::move(shared)) {}

        virtual ~WithStatement() {
            freeref(_resource);
            freeref(_local);
        }

        [[nodiscard]] virtual ASTNodeTag getTag() const override {
            return ASTNodeTag::WITH;
        }

        [[nodiscard]] virtual std::string  toString() const override {
            return "WithStatement<r: " + _resource->toString() + ", as: " + _local->name() + ">";
        }

        [[nodiscard]] ExpressionNode* resource() const {
            return _resource;
        }

        [[nodiscard]] IdentifierNode* local() const {
            return _local;
        }

        [[nodiscard]] bool shared() const {
            return _shared;
        }

        [[nodiscard]] virtual WithStatement* copy() const override {
            auto other = new WithStatement(position(), _resource->copy(), _local->copy(), _shared);
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
            : BlockStatementNode(pos), _condition(useref(condition)) {}

        virtual ~IfStatement() {
            freeref(_condition);
        }

        [[nodiscard]] virtual ASTNodeTag getTag() const override {
            return ASTNodeTag::IF;
        }

        [[nodiscard]] virtual std::string  toString() const override {
            return "IfStatement<f: if " + _condition->toString() + " then, #body: " + s(_body->size()) + ">";
        }

        [[nodiscard]] ExpressionNode* condition() const {
            return _condition;
        }

        [[nodiscard]] virtual IfStatement* copy() const override {
            auto other = new IfStatement(position(), _condition->copy());
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
            : BlockStatementNode(pos), _condition(useref(condition)) {}

        virtual ~WhileStatement() {
            freeref(_condition);
        }

        [[nodiscard]] virtual ASTNodeTag getTag() const override {
            return ASTNodeTag::WHILE;
        }

        [[nodiscard]] virtual std::string  toString() const override {
            return "WhileStatement<w: while " + _condition->toString() + " then, #body: " + s(_body->size()) + ">";
        }

        [[nodiscard]] ExpressionNode* condition() const {
            return _condition;
        }

        [[nodiscard]] virtual WhileStatement* copy() const override {
            auto other = new WhileStatement(position(), _condition->copy());
            other->assumeAndReduceStatements(copyBody());
            return other;
        }
    protected:
        ExpressionNode* _condition;
    };

        class ContinueNode final : public StatementNode {
    public:
        ContinueNode(Position* pos) : StatementNode(pos) {}
        virtual ~ContinueNode() {}

        [[nodiscard]] std::string toString() const override {
            return "ContinueNode<>";
        }

        [[nodiscard]] ASTNodeTag getTag() const override {
            return ASTNodeTag::CONTINUE;
        }

        [[nodiscard]] virtual ContinueNode* copy() const override {
            return new ContinueNode(position());
        }
    };

    class BreakNode : public StatementNode {
    public:
        BreakNode(Position* pos) : StatementNode(pos) {}
        virtual ~BreakNode() {}

        [[nodiscard]] std::string toString() const override {
            return "BreakNode<>";
        }

        [[nodiscard]] ASTNodeTag getTag() const override {
            return ASTNodeTag::BREAK;
        }

        [[nodiscard]] virtual BreakNode* copy() const override {
            return new BreakNode(position());
        }
    };


    /** Value containing a prologue resource reference. */
    class PrologueResourceNode : public ExpressionNode {
    public:
        PrologueResourceNode(Position* pos) : ExpressionNode(pos) {}
        virtual ~PrologueResourceNode() {}

        [[nodiscard]] virtual ExpressionNode* value() = 0;

        [[nodiscard]] virtual bool isOpened() const {
            return _opened;
        }

        [[nodiscard]] virtual bool isValue() const override {
            return true;
        }

        [[nodiscard]] virtual bool isResource() const override {
            return true;
        }

    protected:
        bool _opened = false;
    };

}

namespace nslib {
    [[nodiscard]] std::string s(swarmc::Lang::ASTNodeTag);
}


#endif
