#ifndef SWARMC_SYMBOL_SCRUB_WALK_H
#define SWARMC_SYMBOL_SCRUB_WALK_H

#include "Walk.h"
#include "../../Reporting.h"
#include "../SymbolTable.h"
#include "../AST.h"

namespace swarmc {
namespace Lang {
namespace Walk {

class SymbolScrubWalk : public Walk<void> {
public:
    SymbolScrubWalk(SemanticSymbol* symbol) : Walk<void>(), _symbol(symbol) {}

protected:
    SemanticSymbol* _symbol;

    virtual void walkProgramNode(ProgramNode* node) {
        for ( auto stmt : *node->body() ) {
            walk(stmt);
        }
    }

    virtual void walkExpressionStatementNode(ExpressionStatementNode* node) {
        return walk(node->expression());
    }

    virtual void walkIdentifierNode(IdentifierNode* node) {
        if ( node->_symbol != nullptr && node->_symbol->uuid() == _symbol->uuid() ) {
            node->_symbol = nullptr;
        }
    }

    virtual void walkMapAccessNode(MapAccessNode* node) {
        return walk(node->path());
    }

    virtual void walkEnumerableAccessNode(EnumerableAccessNode* node) {
        walk(node->path());
        walk(node->index());
    }

    // virtual void walkPrimitiveTypeNode(PrimitiveTypeNode* node) {}

    // virtual void walkEnumerableTypeNode(EnumerableTypeNode* node) {}

    // virtual void walkMapTypeNode(MapTypeNode* node) {}

    virtual void walkTypeLiteral(TypeLiteral* node) {}

    virtual void walkBooleanLiteralExpressionNode(BooleanLiteralExpressionNode* node) {}

    virtual void walkVariableDeclarationNode(VariableDeclarationNode* node) {
        if ( node->id()->symbol() != nullptr && node->id()->symbol()->uuid() == _symbol->uuid() ) {
            node->id()->_symbol = nullptr;
        }

        walk(node->value());
    }

    virtual void walkCallExpressionNode(CallExpressionNode* node) {
        walk(node->id());

        for ( auto arg : *node->args() ) {
            walk(arg);
        }
    }

    virtual void walkIIFExpressionNode(IIFExpressionNode* node) {
        walk(node->expression());

        for ( auto arg : *node->args() ) {
            walk(arg);
        }
    }

    virtual void walkAndNode(AndNode* node) {
        walk(node->left());
        walk(node->right());
    }

    virtual void walkOrNode(OrNode* node) {
        walk(node->left());
        walk(node->right());
    }

    virtual void walkEqualsNode(EqualsNode* node) {
        walk(node->left());
        walk(node->right());
    }

    virtual void walkNotEqualsNode(NotEqualsNode* node) {
        walk(node->left());
        walk(node->right());
    }

    virtual void walkAddNode(AddNode* node) {
        walk(node->left());
        walk(node->right());
    }

    virtual void walkSubtractNode(SubtractNode* node) {
        walk(node->left());
        walk(node->right());
    }

    virtual void walkMultiplyNode(MultiplyNode* node) {
        walk(node->left());
        walk(node->right());
    }

    virtual void walkDivideNode(DivideNode* node) {
        walk(node->left());
        walk(node->right());
    }

    virtual void walkModulusNode(ModulusNode* node) {
        walk(node->left());
        walk(node->right());
    }

    virtual void walkPowerNode(PowerNode* node) {
        walk(node->left());
        walk(node->right());
    }

    virtual void walkConcatenateNode(ConcatenateNode* node) {
        walk(node->left());
        walk(node->right());
    }

    virtual void walkNegativeExpressionNode(NegativeExpressionNode* node) {
        walk(node->exp());
    }

    virtual void walkNotNode(NotNode* node) {
        walk(node->exp());
    }

    virtual void walkEnumerationLiteralExpressionNode(EnumerationLiteralExpressionNode* node) {
        for ( auto actual : *node->actuals() ) {
            walk(actual);
        }
    }

    virtual void walkBlockStatementNode(BlockStatementNode* node) {
        for ( auto stmt : *node->body() ) {
            walk(stmt);
        }
    }

    virtual void walkEnumerationStatement(EnumerationStatement* node) {
        walk(node->enumerable());

        if ( node->local()->symbol() != nullptr && node->local()->symbol()->uuid() == _symbol->uuid() ) {
            node->local()->_symbol = nullptr;
        }

        walkBlockStatementNode(node);
    }

    virtual void walkWithStatement(WithStatement* node) {
        walk(node->resource());

        if ( node->local()->symbol() != nullptr && node->local()->symbol()->uuid() == _symbol->uuid() ) {
            node->local()->_symbol = nullptr;
        }

        walkBlockStatementNode(node);
    }

    virtual void walkIfStatement(IfStatement* node) {
        walk(node->condition());
        walkBlockStatementNode(node);
    }

    virtual void walkWhileStatement(WhileStatement* node) {
        walk(node->condition());
        walkBlockStatementNode(node);
    }

    virtual void walkContinueNode(ContinueNode* node) {
        return;
    }

    virtual void walkBreakNode(BreakNode* node) {
        return;
    }

    virtual void walkReturnStatementNode(ReturnStatementNode* node) {
        if ( node->value() != nullptr) {
            walk(node->value());
        }
    }

    virtual void walkMapStatementNode(MapStatementNode* node) {
        walk(node->value());
    }

    virtual void walkMapNode(MapNode* node) {
        for ( auto entry : *node->body() ) {
            walk(entry);
        }
    }

    virtual void walkStringLiteralExpressionNode(StringLiteralExpressionNode* node) {}

    virtual void walkNumberLiteralExpressionNode(NumberLiteralExpressionNode* node) {}

    virtual void walkAssignExpressionNode(AssignExpressionNode* node) {
        walk(node->dest());
        walk(node->value());
    }

    virtual void walkUnitNode(UnitNode* node) {}

    virtual void walkFunctionNode(FunctionNode* node) {
        // TODO: determine if I need to scrub formals
        for ( auto stmt : *node->body() ) {
            walk(stmt);
        }
    }

    virtual void walkNumericComparisonExpressionNode(NumericComparisonExpressionNode* node) {
        walk(node->left());
        walk(node->right());
    }

    virtual std::string toString() const {
        return "SymbolScrubWalk<>";
    }
};

}
}
}

#endif