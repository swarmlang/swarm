#ifndef SWARMC_SYMBOL_SCRUB_WALK_H
#define SWARMC_SYMBOL_SCRUB_WALK_H

#include "Walk.h"
#include "../../Reporting.h"
#include "../SymbolTable.h"
#include "../AST.h"

namespace swarmc::Lang::Walk {

class SymbolScrubWalk : public Walk<void> {
public:
    SymbolScrubWalk(SemanticSymbol* symbol) : Walk<void>(), _symbol(symbol) {}

protected:
    SemanticSymbol* _symbol;

    void walkProgramNode(ProgramNode* node) override {
        for ( auto stmt : *node->body() ) {
            walk(stmt);
        }
    }

    void walkExpressionStatementNode(ExpressionStatementNode* node) override {
        return walk(node->expression());
    }

    void walkIdentifierNode(IdentifierNode* node) override {
        if ( node->_symbol != nullptr && node->_symbol->uuid() == _symbol->uuid() ) {
            node->_symbol = nullptr;
        }
    }

    void walkMapAccessNode(MapAccessNode* node) override {
        return walk(node->path());
    }

    void walkEnumerableAccessNode(EnumerableAccessNode* node) override {
        walk(node->path());
        walk(node->index());
    }

    // void walkPrimitiveTypeNode(PrimitiveTypeNode* override node) {}

    // void walkEnumerableTypeNode(EnumerableTypeNode* override node) {}

    // void walkMapTypeNode(MapTypeNode* override node) {}

    void walkTypeLiteral(TypeLiteral* node) override {}

    void walkBooleanLiteralExpressionNode(BooleanLiteralExpressionNode* node) override {}

    void walkVariableDeclarationNode(VariableDeclarationNode* node) override {
        if ( node->id()->symbol() != nullptr && node->id()->symbol()->uuid() == _symbol->uuid() ) {
            node->id()->_symbol = nullptr;
        }

        walk(node->value());
    }

    void walkCallExpressionNode(CallExpressionNode* node) override {
        walk(node->id());

        for ( auto arg : *node->args() ) {
            walk(arg);
        }
    }

    void walkIIFExpressionNode(IIFExpressionNode* node) override {
        walk(node->expression());

        for ( auto arg : *node->args() ) {
            walk(arg);
        }
    }

    void walkAndNode(AndNode* node) override {
        walk(node->left());
        walk(node->right());
    }

    void walkOrNode(OrNode* node) override {
        walk(node->left());
        walk(node->right());
    }

    void walkEqualsNode(EqualsNode* node) override {
        walk(node->left());
        walk(node->right());
    }

    void walkNotEqualsNode(NotEqualsNode* node) override {
        walk(node->left());
        walk(node->right());
    }

    void walkAddNode(AddNode* node) override {
        walk(node->left());
        walk(node->right());
    }

    void walkSubtractNode(SubtractNode* node) override {
        walk(node->left());
        walk(node->right());
    }

    void walkMultiplyNode(MultiplyNode* node) override {
        walk(node->left());
        walk(node->right());
    }

    void walkDivideNode(DivideNode* node) override {
        walk(node->left());
        walk(node->right());
    }

    void walkModulusNode(ModulusNode* node) override {
        walk(node->left());
        walk(node->right());
    }

    void walkPowerNode(PowerNode* node) override {
        walk(node->left());
        walk(node->right());
    }

    void walkConcatenateNode(ConcatenateNode* node) override {
        walk(node->left());
        walk(node->right());
    }

    void walkNegativeExpressionNode(NegativeExpressionNode* node) override {
        walk(node->exp());
    }

    void walkNotNode(NotNode* node) override {
        walk(node->exp());
    }

    void walkEnumerationLiteralExpressionNode(EnumerationLiteralExpressionNode* node) override {
        for ( auto actual : *node->actuals() ) {
            walk(actual);
        }
    }

    void walkBlockStatementNode(BlockStatementNode* node) override {
        for ( auto stmt : *node->body() ) {
            walk(stmt);
        }
    }

    void walkEnumerationStatement(EnumerationStatement* node) override {
        walk(node->enumerable());

        if ( node->local()->symbol() != nullptr && node->local()->symbol()->uuid() == _symbol->uuid() ) {
            node->local()->_symbol = nullptr;
        }

        walkBlockStatementNode(node);
    }

    void walkWithStatement(WithStatement* node) override {
        walk(node->resource());

        if ( node->local()->symbol() != nullptr && node->local()->symbol()->uuid() == _symbol->uuid() ) {
            node->local()->_symbol = nullptr;
        }

        walkBlockStatementNode(node);
    }

    void walkIfStatement(IfStatement* node) override {
        walk(node->condition());
        walkBlockStatementNode(node);
    }

    void walkWhileStatement(WhileStatement* node) override {
        walk(node->condition());
        walkBlockStatementNode(node);
    }

    void walkContinueNode(ContinueNode* node) override {
        return;
    }

    void walkBreakNode(BreakNode* node) override {
        return;
    }

    void walkReturnStatementNode(ReturnStatementNode* node) override {
        if ( node->value() != nullptr) {
            walk(node->value());
        }
    }

    void walkMapStatementNode(MapStatementNode* node) override {
        walk(node->value());
    }

    void walkMapNode(MapNode* node) override {
        for ( auto entry : *node->body() ) {
            walk(entry);
        }
    }

    void walkStringLiteralExpressionNode(StringLiteralExpressionNode* node) override {}

    void walkNumberLiteralExpressionNode(NumberLiteralExpressionNode* node) override {}

    void walkAssignExpressionNode(AssignExpressionNode* node) override {
        walk(node->dest());
        walk(node->value());
    }

    void walkUnitNode(UnitNode* node) override {}

    void walkFunctionNode(FunctionNode* node) override {
        // TODO: determine if I need to scrub formals
        for ( auto stmt : *node->body() ) {
            walk(stmt);
        }
    }

    void walkNumericComparisonExpressionNode(NumericComparisonExpressionNode* node) override {
        walk(node->left());
        walk(node->right());
    }

    std::string toString() const override {
        return "SymbolScrubWalk<>";
    }
};

}

#endif