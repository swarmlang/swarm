#ifndef SWARMC_ASTMAPREDUCEWALK_H
#define SWARMC_ASTMAPREDUCEWALK_H

#include <optional>
#include "Walk.h"

namespace swarmc::Lang::Walk {

template <typename TReturn>
class ASTMapReduce : public Walk<std::optional<TReturn>> {
public:
    ASTMapReduce(std::string log, std::function<std::optional<TReturn>(ASTNode*)> map, std::function<TReturn(TReturn, TReturn)> reducer)
        : Walk<std::optional<TReturn>>(log), _map(map), _reducer(reducer), _skip([](ASTNode*) { return false; }), _logName(log) {}
    ASTMapReduce(std::string log, std::function<std::optional<TReturn>(ASTNode*)> map, std::function<TReturn(TReturn, TReturn)> reducer,
        std::function<bool(ASTNode*)> skip) : Walk<std::optional<TReturn>>(log), _map(map), _reducer(reducer), _skip(skip), _logName(log) {}
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
        std::string logName = _logName + " + " + other._logName;
        if ( cst == CombineSkipType::AND ) return ASTMapReduce(logName, mapc, _reducer, [this,other](ASTNode* n) {
            return _skip(n) && other._skip(n);
        });
        if ( cst == CombineSkipType::OR ) return ASTMapReduce(logName, mapc, _reducer, [this,other](ASTNode* n) {
            return _skip(n) || other._skip(n);
        });
        if ( cst == CombineSkipType::SECOND ) return ASTMapReduce(logName, mapc, _reducer, other._skip);
        return ASTMapReduce(logName, mapc, _reducer, _skip);
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

    virtual std::optional<TReturn> walkEnumerableAppendNode(EnumerableAppendNode* node) override {
        if ( _skip(node) ) return std::nullopt;
        std::list<std::optional<TReturn>> rets = {
            _map(node),
            this->walk(node->path())
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

    virtual std::optional<TReturn> walkUseNode(UseNode* node) override {
        if ( _skip(node) ) return std::nullopt;
        std::list<std::optional<TReturn>> rets = { _map(node) };
        for ( auto id : *node->ids() ) {
            rets.push_back(this->walk(id));
        }
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
        for ( auto pc : *node->parentConstructors() ) {
            rets.push_back(this->walk(pc));
        }
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

    virtual std::optional<TReturn> walkNthRootNode(NthRootNode* node) override {
        return walkBinaryExpressionNode(node);
    }

    virtual std::optional<TReturn> walkNegativeExpressionNode(NegativeExpressionNode* node) override {
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
    std::string _logName;

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

// implementations

// Removes statements after a return/continue/break, continues at the end of whiles, and void returns at the end of functions
static ASTMapReduce<bool> RemoveRedundantCFB = ASTMapReduce<bool>(
    "AST Remove Post-CFB",
    [](ASTNode* node) {
        bool flag = false;
        auto log = Logging::get()->get("AST Remove Post-CFB");
        if ( node->isBlock() || node->getTag() == ASTNodeTag::FUNCTION ) {
            auto bsn = (node->getTag() == ASTNodeTag::FUNCTION)
                ? ((FunctionNode*)node)->body()
                : ((BlockStatementNode*)node)->body();
            auto i = bsn->begin();
            for ( ; i != bsn->end(); i++ ) {
                if ( (*i)->getTag() == ASTNodeTag::RETURN
                    || (*i)->getTag() == ASTNodeTag::CONTINUE
                    || (*i)->getTag() == ASTNodeTag::BREAK )
                {
                    i++;
                    break;
                }
            }

            // remove post-return/continue/break statements
            if ( i != bsn->end() ) {
                auto p = new Position((*i)->position(), bsn->back()->position());
                log->debug(s(p) + " Remove dead statements");
                GC_LOCAL_REF(p)
                while ( i != bsn->end() ) {
                    freeref(*i);
                    i = bsn->erase(i);
                }
                flag = true;
            }

            // remove unnecessary continue
            if ( node->getTag() == ASTNodeTag::WHILE ) {
                if ( bsn->size() > 0 && bsn->back()->getTag() == ASTNodeTag::CONTINUE ) {
                    log->debug(s(bsn->back()->position()) + " Removed continue at end of While statement");
                    freeref(bsn->back());
                    bsn->resize(bsn->size() - 1);
                    flag = true;
                }
            }

            // remove unnecessary return
            if ( node->getTag() == ASTNodeTag::FUNCTION ) {
                if ( bsn->size() > 0 && bsn->back()->getTag() == ASTNodeTag::RETURN && ((ReturnStatementNode*)bsn->back())->value() != nullptr ) {
                    log->debug(s(bsn->back()->position()) + " Removed void return at end of function");
                    freeref(bsn->back());
                    bsn->resize(bsn->size() - 1);
                    flag = true;
                }
            }
        }
        return flag;
    },
    [](bool l, bool r) { return l || r; }
);

// Returns `true` if the AST has a return statement belonging to this scope
static ASTMapReduce<bool> HasReturn = ASTMapReduce<bool>(
    "AST Has Return",
    [](ASTNode* n) {
        return n->getTag() == ASTNodeTag::RETURN;
    },
    [](bool l, bool r) { return l || r; },
    [](ASTNode* n) {
        return n->getTag() == ASTNodeTag::FUNCTION
            || n->getTag() == ASTNodeTag::ENUMERATE;
    }
);

// Returns `true` if the AST has a continue statement belonging to this scope
static ASTMapReduce<bool> HasContinue = ASTMapReduce<bool>(
    "AST Has Continue",
    [](ASTNode* n) {
        return n->getTag() == ASTNodeTag::CONTINUE;
    },
    [](bool l, bool r) { return l || r; },
    [](ASTNode* n) {
        return n->getTag() == ASTNodeTag::FUNCTION
            || n->getTag() == ASTNodeTag::WHILE
            || n->getTag() == ASTNodeTag::ENUMERATE;
    }
);

// Returns `true` if the AST has a break statement belonging to this scope
static ASTMapReduce<bool> HasBreak = ASTMapReduce<bool>(
    "AST Has Break",
    [](ASTNode* n) {
        return n->getTag() == ASTNodeTag::BREAK;
    },
    [](bool l, bool r) { return l || r; },
    [](ASTNode* n) {
        return n->getTag() == ASTNodeTag::FUNCTION
            || n->getTag() == ASTNodeTag::WHILE
            || n->getTag() == ASTNodeTag::ENUMERATE;
    }
);

}

#endif
