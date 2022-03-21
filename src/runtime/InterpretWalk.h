#ifndef SWARMC_INTERPRETWALK_H
#define SWARMC_INTERPRETWALK_H

#include <assert.h>
#include <math.h>
#include "../Configuration.h"
#include "../errors/SwarmError.h"
#include "../lang/AST.h"
#include "../lang/Walk/Walk.h"
#include "../lang/Walk/NameAnalysisWalk.h"
#include "../lang/Walk/SymbolScrubWalk.h"
#include "LocalSymbolValueStore.h"
#include "SharedSymbolValueStore.h"
#include "prologue/IPrologueFunction.h"
#include "queue/Waiter.h"
#include "queue/ExecutionQueue.h"

// Note: in the future, we'll need to add a RuntimePosition or similar.
//       Right now, positions are copied from the node being evaluated. - GM

using namespace swarmc::Lang;

namespace swarmc {
namespace Runtime {

    class InterpretWalk final : public Lang::Walk::Walk<ASTNode*> {
    public:
        InterpretWalk() : Lang::Walk::Walk<ASTNode*>() {
            _local = new LocalSymbolValueStore;

            Console::get()->debug("Creating new InterpretWalk!");

            if ( Configuration::FORCE_LOCAL ) {
                _shared = new LocalSymbolValueStore;
            } else {
                _shared = new SharedSymbolValueStore;
            }
        }

        virtual std::string toString() const {
            return "InterpretWalk<locals>";
        }

    protected:
        ISymbolValueStore* _local;
        ISymbolValueStore* _shared;
        ExecutionQueue* _queue = new ExecutionQueue;

        virtual ASTNode* walkProgramNode(ProgramNode* node) {
            ASTNode* last = nullptr;

            for ( auto stmt : *node->body() ) {
                last = walk(stmt);
            }

            return last;
        }

        virtual ASTNode* walkExpressionStatementNode(ExpressionStatementNode* node) {
            return walk(node->expression());
        }

        virtual ASTNode* walkIdentifierNode(IdentifierNode* node) {
            auto store = getStore(node->symbol());
            ExpressionNode* value = nullptr;

            return store->withLockedSymbol<ExpressionNode*>(node->symbol(), [store, node, value]() mutable {
                return node->getValue(store);
            });
        }

        virtual ASTNode* walkMapAccessNode(MapAccessNode* node) {
            auto store = getStore(node->lockable());

            return store->withLockedSymbol<ExpressionNode*>(node->lockable(), [this, node, store]() mutable {
                auto value = node->getValue(store);
                assert(value != nullptr);
                return value;
            });
        }

        virtual ASTNode* walkPrimitiveTypeNode(PrimitiveTypeNode* node) {
            return nullptr;
        }

        virtual ASTNode* walkEnumerableTypeNode(EnumerableTypeNode* node) {
            return nullptr;
        }

        virtual ASTNode* walkMapTypeNode(MapTypeNode* node) {
            return nullptr;
        }

        virtual ASTNode* walkBooleanLiteralExpressionNode(BooleanLiteralExpressionNode* node) {
            return node;
        }

        virtual ASTNode* walkVariableDeclarationNode(VariableDeclarationNode* node) {
            ASTNode* rval = walk(node->value());
            assert(rval->isValue() && rval->isExpression());

            auto store = getStore(node->id()->symbol());
            return store->withLockedSymbol<VariableDeclarationNode*>(node->id()->symbol(), [node, rval, store]() mutable {
                auto value = (ExpressionNode*) rval;
                node->id()->setValue(store, value);
                return new VariableDeclarationNode(node->position()->copy(), node->typeNode(), node->id(), value);
            });
        }

        virtual ASTNode* walkCallExpressionNode(CallExpressionNode* node) {
            auto resolvedArgs = new ExpressionList;
            for ( auto arg : *node->args() ) {
                auto resolved = walk(arg);
                assert(resolved->isExpression());
                resolvedArgs->push_back((ExpressionNode*) resolved);
            }

            if ( node->id()->symbol()->isPrologue() ) {
                // We're trying to call a built-in prologue function. Right now, we inject these
                // artificially so we don't have to serialize them with the AST.
                auto fn = Prologue::IPrologueFunction::resolveByName(node->id()->symbol()->name());
                assert(fn != nullptr);
                assert(fn->validateCall(resolvedArgs));

                return fn->call(resolvedArgs);
            }

            throw Errors::SwarmError("Unable to invoke invalid non-Prologue function symbol: " + node->id()->toString());
        }

        virtual ASTNode* walkAndNode(AndNode* node) {
            ASTNode* left = walk(node->left());
            ASTNode* right = walk(node->right());

            assert(left->getName() == "BooleanLiteralExpressionNode");
            assert(right->getName() == "BooleanLiteralExpressionNode");

            BooleanLiteralExpressionNode* leftBool = (BooleanLiteralExpressionNode*) left;
            BooleanLiteralExpressionNode* rightBool = (BooleanLiteralExpressionNode*) right;

            return new BooleanLiteralExpressionNode(node->position()->copy(), leftBool->value() && rightBool->value());
        }

        virtual ASTNode* walkOrNode(OrNode* node) {
            ASTNode* left = walk(node->left());
            ASTNode* right = walk(node->right());

            assert(left->getName() == "BooleanLiteralExpressionNode");
            assert(right->getName() == "BooleanLiteralExpressionNode");

            BooleanLiteralExpressionNode* leftBool = (BooleanLiteralExpressionNode*) left;
            BooleanLiteralExpressionNode* rightBool = (BooleanLiteralExpressionNode*) right;

            return new BooleanLiteralExpressionNode(node->position()->copy(), leftBool->value() || rightBool->value());
        }

        virtual ASTNode* walkEqualsNode(EqualsNode* node) {
            auto left = walk(node->left());
            auto right = walk(node->right());

            assert(left->isExpression() && right->isExpression());

            auto leftExp = (ExpressionNode*) left;
            auto rightExp = (ExpressionNode*) right;

            bool equal = leftExp->type()->is(rightExp->type()) && leftExp->equals(rightExp);
            return new BooleanLiteralExpressionNode(node->position()->copy(), equal);
        }

        virtual ASTNode* walkNotEqualsNode(NotEqualsNode* node) {
            auto left = walk(node->left());
            auto right = walk(node->right());

            assert(left->isExpression() && right->isExpression());

            auto leftExp = (ExpressionNode*) left;
            auto rightExp = (ExpressionNode*) right;

            bool equal = leftExp->type()->is(rightExp->type()) && leftExp->equals(rightExp);
            return new BooleanLiteralExpressionNode(node->position()->copy(), !equal);
        }

        virtual ASTNode* walkAddNode(AddNode* node) {
            ASTNode* left = walk(node->left());
            ASTNode* right = walk(node->right());

            assert(left->getName() == "NumberLiteralExpressionNode");
            assert(right->getName() == "NumberLiteralExpressionNode");

            NumberLiteralExpressionNode* leftNum = (NumberLiteralExpressionNode*) left;
            NumberLiteralExpressionNode* rightNum = (NumberLiteralExpressionNode*) right;

            // FIXME check overflow/underflow

            return new NumberLiteralExpressionNode(node->position()->copy(), leftNum->value() + rightNum->value());
        }

        virtual ASTNode* walkAddAssignExpressionNode(AddAssignExpressionNode* node) {
            // Get the current value of the lval
            auto lval = node->dest();
            auto store = getStore(lval->lockable());

            return store->withLockedSymbol<ExpressionNode*>(lval->lockable(), [this, node, lval, store]() mutable {
                auto initialNode = lval->getValue(store);
                assert(initialNode->isValue() && initialNode->getName() == "NumberLiteralExpressionNode");

                // Get the amount we're adding to it
                ASTNode* right = walk(node->value());
                assert(right->isValue() && right->getName() == "NumberLiteralExpressionNode");
                auto addValue = (NumberLiteralExpressionNode*) right;

                // Perform the addition in Swarm
                ASTNode* result = walk(new AddNode(node->position()->copy(), initialNode, addValue));
                assert(result->isValue() && result->getName() == "NumberLiteralExpressionNode");

                // Assign and return the result
                return assign(lval, (NumberLiteralExpressionNode*) result);
            });
        }

        virtual ASTNode* walkSubtractNode(SubtractNode* node) {
            ASTNode* left = walk(node->left());
            ASTNode* right = walk(node->right());

            assert(left->getName() == "NumberLiteralExpressionNode");
            assert(right->getName() == "NumberLiteralExpressionNode");

            NumberLiteralExpressionNode* leftNum = (NumberLiteralExpressionNode*) left;
            NumberLiteralExpressionNode* rightNum = (NumberLiteralExpressionNode*) right;

            // FIXME check overflow/underflow

            return new NumberLiteralExpressionNode(node->position()->copy(), leftNum->value() - rightNum->value());
        }

        virtual ASTNode* walkMultiplyNode(MultiplyNode* node) {
            ASTNode* left = walk(node->left());
            ASTNode* right = walk(node->right());

            assert(left->getName() == "NumberLiteralExpressionNode");
            assert(right->getName() == "NumberLiteralExpressionNode");

            NumberLiteralExpressionNode* leftNum = (NumberLiteralExpressionNode*) left;
            NumberLiteralExpressionNode* rightNum = (NumberLiteralExpressionNode*) right;

            // FIXME check overflow/underflow

            return new NumberLiteralExpressionNode(node->position()->copy(), leftNum->value() * rightNum->value());
        }

        virtual ASTNode* walkMultiplyAssignExpressionNode(MultiplyAssignExpressionNode* node) {
            // Get the current value of the lval
            auto lval = node->dest();
            auto store = getStore(lval->lockable());

            return store->withLockedSymbol<ExpressionNode*>(lval->lockable(), [this, node, lval, store]() mutable {
                auto initialNode = lval->getValue(store);
                assert(initialNode->isValue() && initialNode->getName() == "NumberLiteralExpressionNode");

                // Get the amount we're adding to it
                ASTNode* right = walk(node->value());
                assert(right->isValue() && right->getName() == "NumberLiteralExpressionNode");
                auto addValue = (NumberLiteralExpressionNode*) right;

                // Perform the addition in Swarm
                ASTNode* result = walk(new MultiplyNode(node->position()->copy(), initialNode, addValue));
                assert(result->isValue() && result->getName() == "NumberLiteralExpressionNode");

                // Assign and return the result
                return assign(lval, (NumberLiteralExpressionNode*) result);
            });
        }

        virtual ASTNode* walkDivideNode(DivideNode* node) {
            ASTNode* left = walk(node->left());
            ASTNode* right = walk(node->right());

            assert(left->getName() == "NumberLiteralExpressionNode");
            assert(right->getName() == "NumberLiteralExpressionNode");

            NumberLiteralExpressionNode* leftNum = (NumberLiteralExpressionNode*) left;
            NumberLiteralExpressionNode* rightNum = (NumberLiteralExpressionNode*) right;

            // FIXME check overflow/underflow, division by zero

            return new NumberLiteralExpressionNode(node->position()->copy(), leftNum->value() / rightNum->value());
        }

        virtual ASTNode* walkModulusNode(ModulusNode* node) {
            ASTNode* left = walk(node->left());
            ASTNode* right = walk(node->right());

            assert(left->getName() == "NumberLiteralExpressionNode");
            assert(right->getName() == "NumberLiteralExpressionNode");

            NumberLiteralExpressionNode* leftNum = (NumberLiteralExpressionNode*) left;
            NumberLiteralExpressionNode* rightNum = (NumberLiteralExpressionNode*) right;

            // FIXME check overflow/underflow, division by zero

            int leftInt = std::round(leftNum->value());
            int rightInt = std::round(rightNum->value());

            return new NumberLiteralExpressionNode(node->position()->copy(), leftInt % rightInt);
        }

        virtual ASTNode* walkPowerNode(PowerNode* node) {
            ASTNode* left = walk(node->left());
            ASTNode* right = walk(node->right());

            assert(left->getName() == "NumberLiteralExpressionNode");
            assert(right->getName() == "NumberLiteralExpressionNode");

            NumberLiteralExpressionNode* leftNum = (NumberLiteralExpressionNode*) left;
            NumberLiteralExpressionNode* rightNum = (NumberLiteralExpressionNode*) right;

            // FIXME check overflow/underflow, division by zero

            return new NumberLiteralExpressionNode(node->position()->copy(), std::pow(leftNum->value(), rightNum->value()));
        }

        virtual ASTNode* walkConcatenateNode(ConcatenateNode* node) {
            ASTNode* left = walk(node->left());
            ASTNode* right = walk(node->right());

            assert(left->getName() == "StringLiteralExpressionNode");
            assert(right->getName() == "StringLiteralExpressionNode");

            StringLiteralExpressionNode* leftStr = (StringLiteralExpressionNode*) left;
            StringLiteralExpressionNode* rightStr = (StringLiteralExpressionNode*) right;

            return new StringLiteralExpressionNode(node->position()->copy(), leftStr->value() + rightStr->value());
        }

        virtual ASTNode* walkNumericComparisonExpressionNode(NumericComparisonExpressionNode* node) {
            ASTNode* left = walk(node->left());
            ASTNode* right = walk(node->right());
            assert(left->isValue() && left->getName() == "NumberLiteralExpressionNode");
            assert(right->isValue() && right->getName() == "NumberLiteralExpressionNode");

            bool result;
            if ( node->comparisonType() == Lang::NumberComparisonType::LESS_THAN ) {
                result = ((NumberLiteralExpressionNode*) left)->value() < ((NumberLiteralExpressionNode*) right)->value();
            } else if ( node->comparisonType() == Lang::NumberComparisonType::LESS_THAN_OR_EQUAL ) {
                result = ((NumberLiteralExpressionNode*) left)->value() <= ((NumberLiteralExpressionNode*) right)->value();
            } else if ( node->comparisonType() == Lang::NumberComparisonType::GREATER_THAN ) {
                result = ((NumberLiteralExpressionNode*) left)->value() > ((NumberLiteralExpressionNode*) right)->value();
            } else {
                result = ((NumberLiteralExpressionNode*) left)->value() >= ((NumberLiteralExpressionNode*) right)->value();
            }

            return new BooleanLiteralExpressionNode(node->position()->copy(), result);
        }

        virtual ASTNode* walkNotNode(NotNode* node) {
            ASTNode* val = walk(node);
            assert(val->getName() == "BooleanLiteralExpressionNode");
            BooleanLiteralExpressionNode* boolVal = (BooleanLiteralExpressionNode*) val;

            return new BooleanLiteralExpressionNode(node->position()->copy(), !boolVal->value());
        }

        virtual ASTNode* walkNegativeExpressionNode(NegativeExpressionNode* node) override {
            ASTNode* val = walk(node->exp());
            assert(val->isValue() && val->getName() == "NumberLiteralExpressionNode");
            return new NumberLiteralExpressionNode(node->position()->copy(), -(((NumberLiteralExpressionNode*) val)->value()));
        }

        virtual ASTNode* walkEnumerationLiteralExpressionNode(EnumerationLiteralExpressionNode* node) {
            ExpressionList* reduced = new ExpressionList;

            for ( auto entry : *node->actuals() ) {
                ASTNode* val = walk(entry);
                assert(val->isValue());
                reduced->push_back((ExpressionNode*) val);
            }

            return new EnumerationLiteralExpressionNode(node->position()->copy(), reduced, node->_disambiguationType);
        }

        virtual ASTNode* walkEnumerationStatement(EnumerationStatement* node) {
            ASTNode* val = walk(node->enumerable());
            assert(val->getName() == "EnumerationLiteralExpressionNode");
            EnumerationLiteralExpressionNode* enumVal = (EnumerationLiteralExpressionNode*) val;

            if ( Configuration::FORCE_LOCAL ) {
                IdentifierNode* local = node->local();
                for ( auto entry : *enumVal->actuals() ) {
                    assign(local, entry);

                    for ( auto stmt : *node->body() ) {
                        walk(stmt);
                    }
                }
            } else {
                size_t limit = Configuration::ENUMERATION_UNROLLING_LIMIT;
                auto actuals = *enumVal->actuals();

                size_t batches = actuals.size() / limit + (actuals.size() % limit != 0);
                size_t i = 0;
                for ( size_t batch_i = 1; batch_i <= batches; batch_i += 1 ) {
                    StatementList list;

                    for ( ; i < (limit * batch_i) && i < actuals.size(); i += 1 ) {
                        auto runBlock = new CapturedBlockStatementNode(node->position()->copy());

                        auto decl = new VariableDeclarationNode(
                            node->position()->copy(),
                            TypeNode::newForType(node->local()->type()->copy()),
                            node->local()->copy(),
                            actuals[i]->copy()
                        );

                        runBlock->pushStatement(decl);
                        runBlock->assumeAndReduceStatements(node->copyBody());

                        Lang::Walk::SymbolScrubWalk scrub(node->local()->symbol());
                        scrub.walk(runBlock);

                        Lang::Walk::NameAnalysisWalk nameAnalysis;
                        assert(nameAnalysis.walk(runBlock));

                        list.push_back(runBlock);
                    }

                    _queue->bulkEvaluate(&list);
                }
            }

            return nullptr;
        }

        virtual ASTNode* walkCapturedBlockStatementNode(CapturedBlockStatementNode* node) {
            for ( auto stmt : *node->body() ) {
                walk(stmt);
            }

            return nullptr;
        }

        virtual ASTNode* walkWithStatement(WithStatement* node) {
//            ASTNode* resource = walk(node->resource());
            // TODO bind local to value

            for ( auto stmt : *node->body() ) {
                walk(stmt);
            }

            return nullptr;
        }

        virtual ASTNode* walkIfStatement(IfStatement* node) {
            ASTNode* cond = walk(node->condition());
            assert(cond->getName() == "BooleanLiteralExpressionNode");
            BooleanLiteralExpressionNode* condVal = (BooleanLiteralExpressionNode*) cond;

            if ( condVal->value() ) {
                for ( auto stmt : *node->body() ) {
                    walk(stmt);
                }
            }

            return nullptr;
        }

        virtual ASTNode* walkWhileStatement(WhileStatement* node) {
            ASTNode* cond = walk(node->condition());
            assert(cond->getName() == "BooleanLiteralExpressionNode");
            BooleanLiteralExpressionNode* condVal = (BooleanLiteralExpressionNode*) cond;

            while ( condVal->value() ) {
                for ( auto stmt : *node->body() ) {
                    walk(stmt);
                }

                cond = walk(node->condition());
                assert(cond->getName() == "BooleanLiteralExpressionNode");
                condVal = (BooleanLiteralExpressionNode*) cond;
            }

            return nullptr;
        }

        virtual ASTNode* walkMapStatementNode(MapStatementNode* node) {
            ASTNode* val = walk(node->value());
            assert(val->isValue() && val->isExpression());
            return new MapStatementNode(node->position()->copy(), node->id(), (ExpressionNode*) val);
        }

        virtual ASTNode* walkMapNode(MapNode* node) {
            MapBody* reduced = new MapBody;

            for ( auto entry : *node->body() ) {
                ASTNode* val = walk(entry);
                assert(val->getName() == "MapStatementNode");
                reduced->push_back((MapStatementNode*) val);
            }

            return new MapNode(node->position()->copy(), reduced);
        }

        virtual ASTNode* walkStringLiteralExpressionNode(StringLiteralExpressionNode* node) {
            return node;
        }

        virtual ASTNode* walkNumberLiteralExpressionNode(NumberLiteralExpressionNode* node) {
            return node;
        }

        virtual ASTNode* walkAssignExpressionNode(AssignExpressionNode* node) {
            LValNode* lval = node->dest();
            return getStore(lval->lockable())
                ->withLockedSymbol<ExpressionNode*>(lval->lockable(), [this, node, lval]() mutable {
                    ASTNode* rvalNode = walk(node->value());
                    assert(rvalNode->isExpression() && rvalNode->isValue());
                    auto rval = (ExpressionNode*) rvalNode;
                    return assign(lval, rval);
                });
        }

        virtual ExpressionNode* assign(LValNode* lval, ExpressionNode* rval) {
            auto store = getStore(lval->lockable());

            return store->withLockedSymbol<ExpressionNode*>(lval->lockable(), [lval, rval, store]() mutable {
                if ( rval->type()->isPrimitiveType() ) rval = rval->copy();
                lval->setValue(store, rval);
                return rval;
            });
        }

        virtual ASTNode* walkEnumerableAccessNode(EnumerableAccessNode* node) override {
            auto store = getStore(node->lockable());
            return store->withLockedSymbol<ExpressionNode*>(node->lockable(), [node, store]() mutable {
                return node->getValue(store);
            });
        }

        virtual ASTNode* walkUnitNode(UnitNode* node) override {
            return node;
        }

        virtual ISymbolValueStore* getStore(SemanticSymbol* symbol) {
            if ( symbol->shared() ) {
                return _shared;
            }

            return _local;
        }
    };

}
}

#endif
