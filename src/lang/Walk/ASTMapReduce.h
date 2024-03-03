#ifndef SWARMC_ASTMAPREDUCEWALK_H
#define SWARMC_ASTMAPREDUCEWALK_H

#include <optional>
#include "Walk.h"

namespace swarmc::Lang::Walk {

template <typename TReturn>
class ASTMapReduce : public Walk<std::optional<TReturn>> {
public:
    ASTMapReduce(std::function<std::optional<TReturn>(ASTNode*)> map, std::function<TReturn(TReturn, TReturn)> reducer) 
        : Walk<std::optional<TReturn>>(), _map(map), _reducer(reducer), _skip([](ASTNode*) { return false; }) {}
    ASTMapReduce(std::function<std::optional<TReturn>(ASTNode*)> map, std::function<TReturn(TReturn, TReturn)> reducer, 
        std::function<bool(ASTNode*)> skip) : Walk<std::optional<TReturn>>(), _map(map), _reducer(reducer), _skip(skip) {}
    ~ASTMapReduce() = default;

    enum class CombineSkipType {
        AND,OR,FIRST,SECOND
    };

    /** Creates a new ASTMapReduce with the `map` and `skip` functions combined. `reduce` will be taken from the calling object*/
    ASTMapReduce<TReturn> combine(const ASTMapReduce<TReturn>& other, CombineSkipType cst) const {
        auto mapc = [this,other](ASTNode* n) {
            std::list<std::optional<TReturn>> l;
            l.push_back(_map(n));
            l.push_back(other._map(n));
            return reduce(l);
        };
        if ( cst == CombineSkipType::AND ) return ASTMapReduce(mapc, _reducer, [this,other](ASTNode* n) { 
            return _skip(n) && other._skip(n); 
        });
        if ( cst == CombineSkipType::OR ) return ASTMapReduce(mapc, _reducer, [this,other](ASTNode* n) { 
            return _skip(n) || other._skip(n); 
        });
        if ( cst == CombineSkipType::SECOND ) return ASTMapReduce(mapc, _reducer, other._skip);
        return ASTMapReduce(mapc, _reducer, _skip);
    }

    virtual std::optional<TReturn> walkStatementList(StatementListWrapper* node) {
        if ( node->body()->size() == 0 ) return std::nullopt;
        std::list<std::optional<TReturn>> rets;
        for ( auto stmt : *node->body() ) {
            rets.push_back(this->walk(stmt));
        }
        return reduce(rets);
    }
protected:
    virtual std::optional<TReturn> walkProgramNode(ProgramNode* node) override {
        if ( _skip(node) ) return std::nullopt;
        std::list<std::optional<TReturn>> rets = { _map(node), walkStatementList(node) };
        return reduce(rets);
    }

    virtual std::optional<TReturn> walkExpressionStatementNode(ExpressionStatementNode* node) override {
        if ( _skip(node) ) return std::nullopt;
        return this->walk(node->expression());
    }

    virtual std::optional<TReturn> walkIdentifierNode(IdentifierNode* node) override {
        if ( _skip(node) ) return std::nullopt;
        return _map(node);
    }

    virtual std::optional<TReturn> walkEnumerableAccessNode(EnumerableAccessNode* node) override {
        if ( _skip(node) ) return std::nullopt;
        std::list<std::optional<TReturn>> rets = { 
            _map(node),
            this->walk(node->path()),
            this->walk(node->index())
        };
        return reduce(rets);
    }

    virtual std::optional<TReturn> walkMapAccessNode(MapAccessNode* node) override {
        if ( _skip(node) ) return std::nullopt;
        std::list<std::optional<TReturn>> rets = { 
            _map(node),
            this->walk(node->path()),
            this->walk(node->end())
        };
        return reduce(rets);
    }

    virtual std::optional<TReturn> walkClassAccessNode(ClassAccessNode* node) override {
        if ( _skip(node) ) return std::nullopt;
        std::list<std::optional<TReturn>> rets = { 
            _map(node),
            this->walk(node->path()),
            this->walk(node->end())
        };
        return reduce(rets);
    }

    virtual std::optional<TReturn> walkIncludeStatementNode(IncludeStatementNode* node) override {
        if ( _skip(node) ) return std::nullopt;
        std::list<std::optional<TReturn>> rets = { _map(node), this->walk(node->path()) };
        if ( node->identifiers() != nullptr ) {
            for ( auto i : *node->identifiers() ) {
                rets.push_back(this->walk(i));
            }
        }
        return reduce(rets);
    }

    virtual std::optional<TReturn> walkTypeLiteral(TypeLiteral* node) override {
        if ( _skip(node) ) return std::nullopt;
        return _map(node);
    }

    virtual std::optional<TReturn> walkBooleanLiteralExpressionNode(BooleanLiteralExpressionNode* node) override {
        if ( _skip(node) ) return std::nullopt;
        return _map(node);
    }

    virtual std::optional<TReturn> walkStringLiteralExpressionNode(StringLiteralExpressionNode* node) override {
        if ( _skip(node) ) return std::nullopt;
        return _map(node);
    }

    virtual std::optional<TReturn> walkNumberLiteralExpressionNode(NumberLiteralExpressionNode* node) override {
        if ( _skip(node) ) return std::nullopt;
        return _map(node);
    }

    virtual std::optional<TReturn> walkEnumerationLiteralExpressionNode(EnumerationLiteralExpressionNode* node) override {
        if ( _skip(node) ) return std::nullopt;
        std::list<std::optional<TReturn>> rets = { _map(node) };
        for ( auto n : *node->actuals() ) {
            rets.push_back(this->walk(n));
        }
        return reduce(rets);
    }

    virtual std::optional<TReturn> walkMapStatementNode(MapStatementNode* node) override {
        if ( _skip(node) ) return std::nullopt;
        std::list<std::optional<TReturn>> rets = { 
            _map(node),
            this->walk(node->id()),
            this->walk(node->value())
        };
        return reduce(rets);
    }

    virtual std::optional<TReturn> walkMapNode(MapNode* node) override {
        if ( _skip(node) ) return std::nullopt;
        std::list<std::optional<TReturn>> rets = { _map(node) };
        for ( auto stmt : *node->body() ) {
            rets.push_back(this->walk(stmt));
        }
        return reduce(rets);
    }

    virtual std::optional<TReturn> walkAssignExpressionNode(AssignExpressionNode* node) override {
        if ( _skip(node) ) return std::nullopt;
        std::list<std::optional<TReturn>> rets = { 
            _map(node),
            this->walk(node->dest()),
            this->walk(node->value())
        };
        return reduce(rets);
    }

    virtual std::optional<TReturn> walkVariableDeclarationNode(VariableDeclarationNode* node) override {
        if ( _skip(node) ) return std::nullopt;
        std::list<std::optional<TReturn>> rets = { 
            _map(node),
            this->walk(node->typeNode()),
            this->walk(node->assignment())
        };
        return reduce(rets);
    }

    virtual std::optional<TReturn> walkUninitializedVariableDeclarationNode(UninitializedVariableDeclarationNode* node) override {
        if ( _skip(node) ) return std::nullopt;
        std::list<std::optional<TReturn>> rets = { 
            _map(node),
            this->walk(node->typeNode()),
            this->walk(node->id())
        };
        return reduce(rets);
    }

    virtual std::optional<TReturn> walkReturnStatementNode(ReturnStatementNode* node) override {
        if ( _skip(node) ) return std::nullopt;
        std::list<std::optional<TReturn>> rets = { _map(node) };
        if ( node->value() != nullptr ) rets.push_back(this->walk(node->value()));
        return reduce(rets);
    }

    virtual std::optional<TReturn> walkFunctionNode(FunctionNode* node) override {
        if ( _skip(node) ) return std::nullopt;
        std::list<std::optional<TReturn>> rets = { 
            _map(node),
            this->walk(node->typeNode())
        };
        for ( auto f : *node->formals() ) {
            rets.push_back(this->walk(f.first));
            rets.push_back(this->walk(f.second));
        }
        rets.push_back(walkStatementList(node));
        return reduce(rets);
    }

    virtual std::optional<TReturn> walkConstructorNode(ConstructorNode* node) override {
        if ( _skip(node) ) return std::nullopt;
        std::list<std::optional<TReturn>> rets = {
            _map(node),
            this->walk(node->func())
        };
        return reduce(rets);
    }

    virtual std::optional<TReturn> walkTypeBodyNode(TypeBodyNode* node) override {
        if ( _skip(node) ) return std::nullopt;
        std::list<std::optional<TReturn>> rets = { _map(node) };
        for ( auto decl : *node->declarations() ) {
            rets.push_back(this->walk(decl));
        }
        for ( auto cons : *node->constructors() ) {
            rets.push_back(this->walk(cons));
        }
        return reduce(rets);
    }

    virtual std::optional<TReturn> walkCallExpressionNode(CallExpressionNode* node) override {
        if ( _skip(node) ) return std::nullopt;
        std::list<std::optional<TReturn>> rets = { 
            _map(node),
            this->walk(node->func())
        };
        for ( auto n : *node->args() ) {
            rets.push_back(this->walk(n));
        }
        return reduce(rets);
    }

    virtual std::optional<TReturn> walkDeferCallExpressionNode(DeferCallExpressionNode* node) override {
        if ( _skip(node) ) return std::nullopt;
        std::list<std::optional<TReturn>> rets = { 
            _map(node),
            this->walk(node->call())
        };
        return reduce(rets);
    }

    virtual std::optional<TReturn> walkAndNode(AndNode* node) override {
        return walkBinaryExpressionNode(node);
    }

    virtual std::optional<TReturn> walkOrNode(OrNode* node) override {
        return walkBinaryExpressionNode(node);
    }

    virtual std::optional<TReturn> walkEqualsNode(EqualsNode* node) override {
        return walkBinaryExpressionNode(node);
    }

    virtual std::optional<TReturn> walkNumericComparisonExpressionNode(NumericComparisonExpressionNode* node) override {
        return walkBinaryExpressionNode(node);
    }

    virtual std::optional<TReturn> walkNotEqualsNode(NotEqualsNode* node) override {
        return walkBinaryExpressionNode(node);
    }

    virtual std::optional<TReturn> walkAddNode(AddNode* node) override {
        return walkBinaryExpressionNode(node);
    }

    virtual std::optional<TReturn> walkSubtractNode(SubtractNode* node) override {
        return walkBinaryExpressionNode(node);
    }

    virtual std::optional<TReturn> walkMultiplyNode(MultiplyNode* node) override {
        return walkBinaryExpressionNode(node);
    }

    virtual std::optional<TReturn> walkDivideNode(DivideNode* node) override {
        return walkBinaryExpressionNode(node);
    }

    virtual std::optional<TReturn> walkModulusNode(ModulusNode* node) override {
        return walkBinaryExpressionNode(node);
    }

    virtual std::optional<TReturn> walkPowerNode(PowerNode* node) override {
        return walkBinaryExpressionNode(node);
    }

    virtual std::optional<TReturn> walkNegativeExpressionNode(NegativeExpressionNode* node) override {
        return walkUnaryExpressionNode(node);
    }

    virtual std::optional<TReturn> walkSqrtNode(SqrtNode* node) override {
        return walkUnaryExpressionNode(node);
    }

    virtual std::optional<TReturn> walkNotNode(NotNode* node) override {
        return walkUnaryExpressionNode(node);
    }

    virtual std::optional<TReturn> walkEnumerationStatement(EnumerationStatement* node) override {
        if ( _skip(node) ) return std::nullopt;
        std::list<std::optional<TReturn>> rets = { 
            _map(node),
            this->walk(node->enumerable()),
            this->walk(node->local())
        };
        if ( node->index() != nullptr ) rets.push_back(this->walk(node->index()));
        rets.push_back(walkStatementList(node));
        return reduce(rets);
    }

    virtual std::optional<TReturn> walkWithStatement(WithStatement* node) override {
        if ( _skip(node) ) return std::nullopt;
        std::list<std::optional<TReturn>> rets = { 
            _map(node),
            this->walk(node->resource()),
            this->walk(node->local())
        };
        rets.push_back(walkStatementList(node));
        return reduce(rets);
    }

    virtual std::optional<TReturn> walkIfStatement(IfStatement* node) override {
        if ( _skip(node) ) return std::nullopt;
        std::list<std::optional<TReturn>> rets = { 
            _map(node),
            this->walk(node->condition())
        };
        rets.push_back(walkStatementList(node));
        return reduce(rets);
    }

    virtual std::optional<TReturn> walkWhileStatement(WhileStatement* node) override {
        if ( _skip(node) ) return std::nullopt;
        std::list<std::optional<TReturn>> rets = { 
            _map(node),
            this->walk(node->condition())
        };
        rets.push_back(walkStatementList(node));
        return reduce(rets);
    }

    virtual std::optional<TReturn> walkContinueNode(ContinueNode* node) override {
        if ( _skip(node) ) return std::nullopt;
        return _map(node);
    }

    virtual std::optional<TReturn> walkBreakNode(BreakNode* node) override {
        if ( _skip(node) ) return std::nullopt;
        return _map(node);
    }

    [[nodiscard]] std::string toString() const override {
        return "ASTMapReduce<>";
    }
private:
    std::function<std::optional<TReturn>(ASTNode*)> _map;
    std::function<TReturn(TReturn, TReturn)> _reducer;
    std::function<bool(ASTNode*)> _skip;

    virtual std::optional<TReturn> walkBinaryExpressionNode(BinaryExpressionNode* node) {
        if ( _skip(node) ) return std::nullopt;
        std::list<std::optional<TReturn>> rets = { 
            _map(node),
            this->walk(node->left()),
            this->walk(node->right())
        };
        return reduce(rets);
    }

    virtual std::optional<TReturn> walkUnaryExpressionNode(UnaryExpressionNode* node) {
        if ( _skip(node) ) return std::nullopt;
        std::list<std::optional<TReturn>> rets = { _map(node), this->walk(node->exp()) };
        return reduce(rets);
    }

    std::optional<TReturn> reduce(std::list<std::optional<TReturn>>& rets) const {
        TReturn acc;
        // clear nullopts from front
        while ( rets.size() > 0 && !rets.front().has_value() ) {
            rets.pop_front();
        }
        if ( rets.size() == 0 ) return std::nullopt;
        acc = rets.front().value();
        rets.pop_front();
        while ( rets.size() > 0 ) {
            if ( rets.front().has_value() ) {
                acc = _reducer(acc, rets.front().value());
            }
            rets.pop_front();
        }
        return acc;
    }
};

}

#endif