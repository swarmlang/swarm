#ifndef SWARMC_SYMBOLWALK_H
#define SWARMC_SYMBOLWALK_H

#include <string>
#include <unordered_map>
#include "Walk.h"

namespace swarmc {
namespace Lang {
namespace Walk {

using SymbolMap = std::unordered_map<std::string, SemanticSymbol*>;

class SymbolWalk : public Walk<SymbolMap*> {
public:
    SymbolWalk() : Walk<SymbolMap*>() {}
protected:
    virtual SymbolMap* walkProgramNode(ProgramNode* node) {
        SymbolMap* map = new SymbolMap();

        for (auto i : *node->body()) {
            SymbolMap* m = walk(i);
            map->insert(m->begin(), m->end());
            //delete m;
        }

        return map;
    }

    virtual SymbolMap* walkExpressionStatementNode(ExpressionStatementNode* node) {
        return new SymbolMap();
    }

    virtual SymbolMap* walkIdentifierNode(IdentifierNode* node) {
        SymbolMap* map = new SymbolMap();
        map->insert(std::pair<std::string, SemanticSymbol*>(node->symbol()->uuid(), node->symbol()));
        return map;
    }

    virtual SymbolMap* walkMapAccessNode(MapAccessNode* node) {
        return walk(node->path());
    }

    virtual SymbolMap* walkEnumerableAccessNode(EnumerableAccessNode* node) {
        return walk(node->path());
    }

    virtual SymbolMap* walkPrimitiveTypeNode(PrimitiveTypeNode* node) {
        return new SymbolMap();
    }

    virtual SymbolMap* walkEnumerableTypeNode(EnumerableTypeNode* node) {
        return new SymbolMap();
    }

    virtual SymbolMap* walkMapTypeNode(MapTypeNode* node) {
        return new SymbolMap();
    }

    virtual SymbolMap* walkBooleanLiteralExpressionNode(BooleanLiteralExpressionNode* node) {
        return new SymbolMap();
    }

    virtual SymbolMap* walkVariableDeclarationNode(VariableDeclarationNode* node) {
        return walk(node->id());
    }

    virtual SymbolMap* walkCallExpressionNode(CallExpressionNode* node) {
        return new SymbolMap();
    }

    virtual SymbolMap* walkAndNode(AndNode* node) {
        return new SymbolMap();
    }

    virtual SymbolMap* walkOrNode(OrNode* node) {
        return new SymbolMap();
    }

    virtual SymbolMap* walkEqualsNode(EqualsNode* node) {
        return new SymbolMap();
    }

    virtual SymbolMap* walkNotEqualsNode(NotEqualsNode* node) {
        return new SymbolMap();
    }

    virtual SymbolMap* walkAddNode(AddNode* node) {
        return new SymbolMap();
    }

    virtual SymbolMap* walkAddAssignExpressionNode(AddAssignExpressionNode* node) {
        return new SymbolMap();
    }

    virtual SymbolMap* walkSubtractNode(SubtractNode* node) {
        return new SymbolMap();
    }

    virtual SymbolMap* walkMultiplyNode(MultiplyNode* node) {
        return new SymbolMap();
    }

    virtual SymbolMap* walkMultiplyAssignExpressionNode(MultiplyAssignExpressionNode* node) {
        return new SymbolMap();
    }

    virtual SymbolMap* walkDivideNode(DivideNode* node) {
        return new SymbolMap();
    }

    virtual SymbolMap* walkModulusNode(ModulusNode* node) {
        return new SymbolMap();
    }

    virtual SymbolMap* walkPowerNode(PowerNode* node) {
        return new SymbolMap();
    }

    virtual SymbolMap* walkConcatenateNode(ConcatenateNode* node) {
        return new SymbolMap();
    }

    virtual SymbolMap* walkNegativeExpressionNode(NegativeExpressionNode* node) {
        return new SymbolMap();
    }

    virtual SymbolMap* walkNotNode(NotNode* node) {
        return new SymbolMap();
    }

    virtual SymbolMap* walkEnumerationLiteralExpressionNode(EnumerationLiteralExpressionNode* node) {
        return new SymbolMap();
    }

    virtual SymbolMap* walkEnumerationStatement(EnumerationStatement* node) {
        SymbolMap* map = walk(node->local());

        for (auto i : *node->body()) {
            SymbolMap* m = walk(i);
            map->insert(m->begin(), m->end());
            //delete m;
        }

        return map;
    }

    virtual SymbolMap* walkWithStatement(WithStatement* node) {
        SymbolMap* map = walk(node->local());

        for (auto i : *node->body()) {
            SymbolMap* m = walk(i);
            map->insert(m->begin(), m->end());
            //delete m;
        }

        return map;
    }

    virtual SymbolMap* walkIfStatement(IfStatement* node) {
        SymbolMap* map = new SymbolMap();

        for (auto i : *node->body()) {
            SymbolMap* m = walk(i);
            map->insert(m->begin(), m->end());
            //delete m;
        }

        return map;
    }

    virtual SymbolMap* walkWhileStatement(WhileStatement* node) {
        SymbolMap* map = new SymbolMap();

        for (auto i : *node->body()) {
            SymbolMap* m = walk(i);
            map->insert(m->begin(), m->end());
            //delete m;
        }

        return map;
    }

    virtual SymbolMap* walkMapStatementNode(MapStatementNode* node) {
        return new SymbolMap();
    }

    virtual SymbolMap* walkMapNode(MapNode* node) {
        return new SymbolMap();
    }

    virtual SymbolMap* walkStringLiteralExpressionNode(StringLiteralExpressionNode* node) {
        return new SymbolMap();
    }

    virtual SymbolMap* walkNumberLiteralExpressionNode(NumberLiteralExpressionNode* node) {
        return new SymbolMap();
    }

    virtual SymbolMap* walkAssignExpressionNode(AssignExpressionNode* node) {
        return new SymbolMap();
    }

    virtual SymbolMap* walkUnitNode(UnitNode* node) {
        return new SymbolMap();
    }

    virtual SymbolMap* walkNumericComparisonExpressionNode(NumericComparisonExpressionNode* node) {
        return new SymbolMap();
    }

    virtual SymbolMap* walkCapturedBlockStatementNode(CapturedBlockStatementNode* node) {
        SymbolMap* map = new SymbolMap();

        for (auto i : *node->body()) {
            SymbolMap* m = walk(i);
            map->insert(m->begin(), m->end());
            //delete m;
        }

        return map;
    }

    std::string toString() const override {
        return "SymbolWalk<>";
    }
};

}
}
}

#endif