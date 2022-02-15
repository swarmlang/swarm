#ifndef SWARMC_INTERPRETWALK_H
#define SWARMC_INTERPRETWALK_H

#include <assert.h>
#include <math.h>
#include "../errors/SwarmError.h"
#include "../lang/AST.h"
#include "../lang/Walk.h"
#include "LocalSymbolValueStore.h"

using namespace swarmc::Lang;

namespace swarmc {
namespace Runtime {

    class InterpretWalk : public Lang::Walk<ASTNode*> {
    public:
        InterpretWalk() : Lang::Walk<ASTNode*>() {
            _local = new LocalSymbolValueStore;
        }

    protected:
        LocalSymbolValueStore* _local;

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
            auto value = node->getValue(_local);
            assert(value != nullptr);
            return value;
        }

        virtual ASTNode* walkMapAccessNode(MapAccessNode* node) {
            auto value = node->getValue(_local);
            assert(value != nullptr);
            return value;
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

            auto value = (ExpressionNode*) rval;
            node->id()->setValue(_local, value);
            return new VariableDeclarationNode(nullptr, node->typeNode(), node->id(), value, node->shared());
        }

        virtual ASTNode* walkCallExpressionNode(CallExpressionNode* node) = 0;

        virtual ASTNode* walkAndNode(AndNode* node) {
            ASTNode* left = walk(node->left());
            ASTNode* right = walk(node->right());

            assert(left->getName() == "BooleanLiteralExpressionNode");
            assert(right->getName() == "BooleanLiteralExpressionNode");

            BooleanLiteralExpressionNode* leftBool = (BooleanLiteralExpressionNode*) left;
            BooleanLiteralExpressionNode* rightBool = (BooleanLiteralExpressionNode*) right;

            return new BooleanLiteralExpressionNode(nullptr, leftBool->value() && rightBool->value());
        }

        virtual ASTNode* walkOrNode(OrNode* node) {
            ASTNode* left = walk(node->left());
            ASTNode* right = walk(node->right());

            assert(left->getName() == "BooleanLiteralExpressionNode");
            assert(right->getName() == "BooleanLiteralExpressionNode");

            BooleanLiteralExpressionNode* leftBool = (BooleanLiteralExpressionNode*) left;
            BooleanLiteralExpressionNode* rightBool = (BooleanLiteralExpressionNode*) right;

            return new BooleanLiteralExpressionNode(nullptr, leftBool->value() || rightBool->value());
        }

        virtual ASTNode* walkEqualsNode(EqualsNode* node) = 0;

        virtual ASTNode* walkNotEqualsNode(NotEqualsNode* node) = 0;

        virtual ASTNode* walkAddNode(AddNode* node) {
            ASTNode* left = walk(node->left());
            ASTNode* right = walk(node->right());

            assert(left->getName() == "NumberLiteralExpressionNode");
            assert(right->getName() == "NumberLiteralExpressionNode");

            NumberLiteralExpressionNode* leftNum = (NumberLiteralExpressionNode*) left;
            NumberLiteralExpressionNode* rightNum = (NumberLiteralExpressionNode*) right;

            // FIXME check overflow/underflow

            return new NumberLiteralExpressionNode(nullptr, leftNum->value() + rightNum->value());
        }

        virtual ASTNode* walkAddAssignExpressionNode(AddAssignExpressionNode* node) {
            // get the value of the lval
            // add to the value
            // set the new value of the lval

            auto lval = node->dest();
            auto initialNode = lval->getValue(_local);
            assert(initialNode->isValue() && initialNode->getName() == "NumberLiteralExpressionNode");

            ASTNode* right = walk(node->value());
        }

        virtual ASTNode* walkSubtractNode(SubtractNode* node) {
            ASTNode* left = walk(node->left());
            ASTNode* right = walk(node->right());

            assert(left->getName() == "NumberLiteralExpressionNode");
            assert(right->getName() == "NumberLiteralExpressionNode");

            NumberLiteralExpressionNode* leftNum = (NumberLiteralExpressionNode*) left;
            NumberLiteralExpressionNode* rightNum = (NumberLiteralExpressionNode*) right;

            // FIXME check overflow/underflow

            return new NumberLiteralExpressionNode(nullptr, leftNum->value() - rightNum->value());
        }

        virtual ASTNode* walkMultiplyNode(MultiplyNode* node) {
            ASTNode* left = walk(node->left());
            ASTNode* right = walk(node->right());

            assert(left->getName() == "NumberLiteralExpressionNode");
            assert(right->getName() == "NumberLiteralExpressionNode");

            NumberLiteralExpressionNode* leftNum = (NumberLiteralExpressionNode*) left;
            NumberLiteralExpressionNode* rightNum = (NumberLiteralExpressionNode*) right;

            // FIXME check overflow/underflow

            return new NumberLiteralExpressionNode(nullptr, leftNum->value() * rightNum->value());
        }

        virtual ASTNode* walkMultiplyAssignExpressionNode(MultiplyAssignExpressionNode* node) = 0;

        virtual ASTNode* walkDivideNode(DivideNode* node) {
            ASTNode* left = walk(node->left());
            ASTNode* right = walk(node->right());

            assert(left->getName() == "NumberLiteralExpressionNode");
            assert(right->getName() == "NumberLiteralExpressionNode");

            NumberLiteralExpressionNode* leftNum = (NumberLiteralExpressionNode*) left;
            NumberLiteralExpressionNode* rightNum = (NumberLiteralExpressionNode*) right;

            // FIXME check overflow/underflow, division by zero

            return new NumberLiteralExpressionNode(nullptr, leftNum->value() / rightNum->value());
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

            return new NumberLiteralExpressionNode(nullptr, leftInt % rightInt);
        }

        virtual ASTNode* walkPowerNode(PowerNode* node) {
            ASTNode* left = walk(node->left());
            ASTNode* right = walk(node->right());

            assert(left->getName() == "NumberLiteralExpressionNode");
            assert(right->getName() == "NumberLiteralExpressionNode");

            NumberLiteralExpressionNode* leftNum = (NumberLiteralExpressionNode*) left;
            NumberLiteralExpressionNode* rightNum = (NumberLiteralExpressionNode*) right;

            // FIXME check overflow/underflow, division by zero

            return new NumberLiteralExpressionNode(nullptr, std::pow(leftNum->value(), rightNum->value()));
        }

        virtual ASTNode* walkConcatenateNode(ConcatenateNode* node) {
            ASTNode* left = walk(node->left());
            ASTNode* right = walk(node->right());

            assert(left->getName() == "StringLiteralExpressionNode");
            assert(right->getName() == "StringLiteralExpressionNode");

            StringLiteralExpressionNode* leftStr = (StringLiteralExpressionNode*) left;
            StringLiteralExpressionNode* rightStr = (StringLiteralExpressionNode*) right;

            return new StringLiteralExpressionNode(nullptr, leftStr->value() + rightStr->value());
        }

        virtual ASTNode* walkNotNode(NotNode* node) {
            ASTNode* val = walk(node);
            assert(val->getName() == "BooleanLiteralExpressionNode");
            BooleanLiteralExpressionNode* boolVal = (BooleanLiteralExpressionNode*) val;

            return new BooleanLiteralExpressionNode(nullptr, !boolVal->value());
        }

        virtual ASTNode* walkEnumerationLiteralExpressionNode(EnumerationLiteralExpressionNode* node) {
            ExpressionList* reduced = new ExpressionList;

            for ( auto entry : *node->actuals() ) {
                ASTNode* val = walk(entry);
                assert(val->isValue());
                reduced->push_back((ExpressionNode*) val);
            }

            return new EnumerationLiteralExpressionNode(nullptr, reduced);
        }

        virtual ASTNode* walkEnumerationStatement(EnumerationStatement* node) {
            ASTNode* val = walk(node);
            assert(val->getName() == "EnumerationLiteralExpressionNode");
            EnumerationLiteralExpressionNode* enumVal = (EnumerationLiteralExpressionNode*) val;

            for ( auto val : *enumVal->actuals() ) {
                // TODO bind local to value

                for ( auto stmt : *node->body() ) {
                    walk(stmt);
                }
            }

            return nullptr;
        }

        virtual ASTNode* walkWithStatement(WithStatement* node) {
            ASTNode* resource = walk(node->resource());
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

            while ( condVal ) {
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
            ASTNode* val = walk(node);
            assert(val->isValue() && val->isExpression());
            return new MapStatementNode(nullptr, node->id(), (ExpressionNode*) val);
        }

        virtual ASTNode* walkMapNode(MapNode* node) {
            MapBody* reduced = new MapBody;

            for ( auto entry : *node->body() ) {
                ASTNode* val = walk(entry);
                assert(val->getName() == "MapStatementNode");
                reduced->push_back((MapStatementNode*) val);
            }

            return new MapNode(nullptr, reduced);
        }

        virtual ASTNode* walkStringLiteralExpressionNode(StringLiteralExpressionNode* node) {
            return node;
        }

        virtual ASTNode* walkNumberLiteralExpressionNode(NumberLiteralExpressionNode* node) {
            return node;
        }

        virtual ASTNode* walkAssignExpressionNode(AssignExpressionNode* node) = 0;
    };

}
}

#endif
