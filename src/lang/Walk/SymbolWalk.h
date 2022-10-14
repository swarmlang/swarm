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
            delete m;
        }

        return map;
    }

    virtual SymbolMap* walkExpressionStatementNode(ExpressionStatementNode* node) {
        return walk(node->expression());
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

    virtual SymbolMap* walkTypeLiteral(swarmc::Lang::TypeLiteral *node) {
        return new SymbolMap();
    }

    virtual SymbolMap* walkBooleanLiteralExpressionNode(BooleanLiteralExpressionNode* node) {
        return new SymbolMap();
    }

    virtual SymbolMap* walkVariableDeclarationNode(VariableDeclarationNode* node) {
        SymbolMap* leftMap = walk(node->id());
        SymbolMap* rightMap = walk(node->value());

        leftMap->insert(rightMap->begin(), rightMap->end());
        delete rightMap;

        return leftMap;
    }

    virtual SymbolMap* walkCallExpressionNode(CallExpressionNode* node) {
        SymbolMap* map = new SymbolMap();

        for (auto i : *node->args()) {
            SymbolMap* m = walk(i);
            map->insert(m->begin(), m->end());
            delete m;
        }

        return map;
    }

    virtual SymbolMap* walkAndNode(AndNode* node) {
        SymbolMap* leftMap = walk(node->left());
        SymbolMap* rightMap = walk(node->right());

        leftMap->insert(rightMap->begin(), rightMap->end());
        delete rightMap;

        return leftMap;
    }

    virtual SymbolMap* walkOrNode(OrNode* node) {
        SymbolMap* leftMap = walk(node->left());
        SymbolMap* rightMap = walk(node->right());

        leftMap->insert(rightMap->begin(), rightMap->end());
        delete rightMap;

        return leftMap;
    }

    virtual SymbolMap* walkEqualsNode(EqualsNode* node) {
        SymbolMap* leftMap = walk(node->left());
        SymbolMap* rightMap = walk(node->right());

        leftMap->insert(rightMap->begin(), rightMap->end());
        delete rightMap;

        return leftMap;
    }

    virtual SymbolMap* walkNotEqualsNode(NotEqualsNode* node) {
        SymbolMap* leftMap = walk(node->left());
        SymbolMap* rightMap = walk(node->right());

        leftMap->insert(rightMap->begin(), rightMap->end());
        delete rightMap;

        return leftMap;
    }

    virtual SymbolMap* walkAddNode(AddNode* node) {
        SymbolMap* leftMap = walk(node->left());
        SymbolMap* rightMap = walk(node->right());

        leftMap->insert(rightMap->begin(), rightMap->end());
        delete rightMap;

        return leftMap;
    }

    virtual SymbolMap* walkSubtractNode(SubtractNode* node) {
        SymbolMap* leftMap = walk(node->left());
        SymbolMap* rightMap = walk(node->right());

        leftMap->insert(rightMap->begin(), rightMap->end());
        delete rightMap;

        return leftMap;
    }

    virtual SymbolMap* walkMultiplyNode(MultiplyNode* node) {
        SymbolMap* leftMap = walk(node->left());
        SymbolMap* rightMap = walk(node->right());

        leftMap->insert(rightMap->begin(), rightMap->end());
        delete rightMap;

        return leftMap;
    }

    virtual SymbolMap* walkDivideNode(DivideNode* node) {
        SymbolMap* leftMap = walk(node->left());
        SymbolMap* rightMap = walk(node->right());

        leftMap->insert(rightMap->begin(), rightMap->end());
        delete rightMap;

        return leftMap;
    }

    virtual SymbolMap* walkModulusNode(ModulusNode* node) {
        SymbolMap* leftMap = walk(node->left());
        SymbolMap* rightMap = walk(node->right());

        leftMap->insert(rightMap->begin(), rightMap->end());
        delete rightMap;

        return leftMap;
    }

    virtual SymbolMap* walkPowerNode(PowerNode* node) {
        SymbolMap* leftMap = walk(node->left());
        SymbolMap* rightMap = walk(node->right());

        leftMap->insert(rightMap->begin(), rightMap->end());
        delete rightMap;

        return leftMap;
    }

    virtual SymbolMap* walkConcatenateNode(ConcatenateNode* node) {
        SymbolMap* leftMap = walk(node->left());
        SymbolMap* rightMap = walk(node->right());

        leftMap->insert(rightMap->begin(), rightMap->end());
        delete rightMap;

        return leftMap;
    }

    virtual SymbolMap* walkNegativeExpressionNode(NegativeExpressionNode* node) {
        return walk(node->exp());
    }

    virtual SymbolMap* walkNotNode(NotNode* node) {
        return walk(node->exp());
    }

    virtual SymbolMap* walkEnumerationLiteralExpressionNode(EnumerationLiteralExpressionNode* node) {
        SymbolMap* map = new SymbolMap();

        for (auto i : *node->actuals()) {
            SymbolMap* m = walk(i);
            map->insert(m->begin(), m->end());
            delete m;
        }

        return map;
    }

    virtual SymbolMap* walkEnumerationStatement(EnumerationStatement* node) {
        SymbolMap* map = walk(node->local());

        for (auto i : *node->body()) {
            SymbolMap* m = walk(i);
            map->insert(m->begin(), m->end());
            delete m;
        }

        return map;
    }

    virtual SymbolMap* walkWithStatement(WithStatement* node) {
        SymbolMap* map = walk(node->local());

        for (auto i : *node->body()) {
            SymbolMap* m = walk(i);
            map->insert(m->begin(), m->end());
            delete m;
        }

        return map;
    }

    virtual SymbolMap* walkIfStatement(IfStatement* node) {
        SymbolMap* map = walk(node->condition());

        for (auto i : *node->body()) {
            SymbolMap* m = walk(i);
            map->insert(m->begin(), m->end());
            delete m;
        }

        return map;
    }

    virtual SymbolMap* walkWhileStatement(WhileStatement* node) {
        SymbolMap* map = walk(node->condition());

        for (auto i : *node->body()) {
            SymbolMap* m = walk(i);
            map->insert(m->begin(), m->end());
            delete m;
        }

        return map;
    }

    virtual SymbolMap* walkMapStatementNode(MapStatementNode* node) {
        return walk(node->value());
    }

    virtual SymbolMap* walkMapNode(MapNode* node) {
        SymbolMap* map = new SymbolMap();

        for (auto i : *node->body()) {
            SymbolMap* m = walk(i);
            map->insert(m->begin(), m->end());
            delete m;
        }

        return map;
    }

    virtual SymbolMap* walkStringLiteralExpressionNode(StringLiteralExpressionNode* node) {
        return new SymbolMap();
    }

    virtual SymbolMap* walkNumberLiteralExpressionNode(NumberLiteralExpressionNode* node) {
        return new SymbolMap();
    }

    virtual SymbolMap* walkAssignExpressionNode(AssignExpressionNode* node) {
        SymbolMap* leftMap = walk(node->dest());
        SymbolMap* rightMap = walk(node->value());

        leftMap->insert(rightMap->begin(), rightMap->end());
        delete rightMap;

        return leftMap;
    }

    virtual SymbolMap* walkUnitNode(UnitNode* node) {
        return new SymbolMap();
    }

    // TODO: determine if I need to walk formals
    virtual SymbolMap* walkOneLineFunctionNode(OneLineFunctionNode* node) {
        SymbolMap* map = walk(node->body());

        return map;
    }

    // TODO: determine if I need to walk formals
    virtual SymbolMap* walkMultiLineFunctionNode(MultiLineFunctionNode* node) {
        SymbolMap* map = new SymbolMap();

        for (auto i : *node->body()) {
            SymbolMap* m = walk(i);
            map->insert(m->begin(), m->end());
            delete m;
        }

        return map;
    }

    virtual SymbolMap* walkNumericComparisonExpressionNode(NumericComparisonExpressionNode* node) {
        SymbolMap* leftMap = walk(node->left());
        SymbolMap* rightMap = walk(node->right());

        leftMap->insert(rightMap->begin(), rightMap->end());
        delete rightMap;

        return leftMap;
    }

    virtual SymbolMap* walkCapturedBlockStatementNode(CapturedBlockStatementNode* node) {
        SymbolMap* map = new SymbolMap();

        for (auto i : *node->body()) {
            SymbolMap* m = walk(i);
            map->insert(m->begin(), m->end());
            delete m;
        }

        return map;
    }

    virtual SymbolMap* walkTagResourceNode(TagResourceNode* node) {
        return new SymbolMap();
    }

    std::string toString() const override {
        return "SymbolWalk<>";
    }
};

}
}
}

#endif