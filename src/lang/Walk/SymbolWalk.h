#ifndef SWARMC_SYMBOLWALK_H
#define SWARMC_SYMBOLWALK_H

#include <string>
#include <unordered_map>
#include "Walk.h"

namespace swarmc::Lang::Walk {

using SymbolMap = std::unordered_map<std::string, SemanticSymbol*>;

class SymbolWalk : public Walk<SymbolMap*> {
public:
    SymbolWalk() : Walk<SymbolMap*>() {}
protected:
    SymbolMap* walkProgramNode(ProgramNode* node) override {
        auto* map = new SymbolMap();

        for (auto i : *node->body()) {
            SymbolMap* m = walk(i);
            map->insert(m->begin(), m->end());
            delete m;
        }

        return map;
    }

    SymbolMap* walkExpressionStatementNode(ExpressionStatementNode* node) override {
        return walk(node->expression());
    }

    SymbolMap* walkIdentifierNode(IdentifierNode* node) override {
        auto map = new SymbolMap();
        map->insert(std::pair<std::string, SemanticSymbol*>(node->symbol()->uuid(), node->symbol()));
        return map;
    }

    SymbolMap* walkMapAccessNode(MapAccessNode* node) override {
        return walk(node->path());
    }

    SymbolMap* walkEnumerableAccessNode(EnumerableAccessNode* node) override {
        SymbolMap* leftMap = walk(node->path());
        SymbolMap* rightMap = walk(node->index());

        leftMap->insert(rightMap->begin(), rightMap->end());
        delete rightMap;

        return leftMap;
    }

    SymbolMap* walkTypeLiteral(swarmc::Lang::TypeLiteral *node) override {
        return new SymbolMap();
    }

    SymbolMap* walkBooleanLiteralExpressionNode(BooleanLiteralExpressionNode* node) override {
        return new SymbolMap();
    }

    SymbolMap* walkVariableDeclarationNode(VariableDeclarationNode* node) override {
        SymbolMap* leftMap = walk(node->id());
        SymbolMap* rightMap = walk(node->value());

        leftMap->insert(rightMap->begin(), rightMap->end());
        delete rightMap;

        return leftMap;
    }

    SymbolMap* walkCallExpressionNode(CallExpressionNode* node) override {
        auto map = walk(node->func());

        for (auto i : *node->args()) {
            SymbolMap* m = walk(i);
            map->insert(m->begin(), m->end());
            delete m;
        }

        return map;
    }

    SymbolMap* walkIIFExpressionNode(IIFExpressionNode* node) override {
        SymbolMap* map = walk(node->expression());

        for (auto i : *node->args()) {
            SymbolMap* m = walk(i);
            map->insert(m->begin(), m->end());
            delete m;
        }

        return map;
    }

    SymbolMap* walkAndNode(AndNode* node) override {
        SymbolMap* leftMap = walk(node->left());
        SymbolMap* rightMap = walk(node->right());

        leftMap->insert(rightMap->begin(), rightMap->end());
        delete rightMap;

        return leftMap;
    }

    SymbolMap* walkOrNode(OrNode* node) override {
        SymbolMap* leftMap = walk(node->left());
        SymbolMap* rightMap = walk(node->right());

        leftMap->insert(rightMap->begin(), rightMap->end());
        delete rightMap;

        return leftMap;
    }

    SymbolMap* walkEqualsNode(EqualsNode* node) override {
        SymbolMap* leftMap = walk(node->left());
        SymbolMap* rightMap = walk(node->right());

        leftMap->insert(rightMap->begin(), rightMap->end());
        delete rightMap;

        return leftMap;
    }

    SymbolMap* walkNotEqualsNode(NotEqualsNode* node) override {
        SymbolMap* leftMap = walk(node->left());
        SymbolMap* rightMap = walk(node->right());

        leftMap->insert(rightMap->begin(), rightMap->end());
        delete rightMap;

        return leftMap;
    }

    SymbolMap* walkAddNode(AddNode* node) override {
        SymbolMap* leftMap = walk(node->left());
        SymbolMap* rightMap = walk(node->right());

        leftMap->insert(rightMap->begin(), rightMap->end());
        delete rightMap;

        return leftMap;
    }

    SymbolMap* walkSubtractNode(SubtractNode* node) override {
        SymbolMap* leftMap = walk(node->left());
        SymbolMap* rightMap = walk(node->right());

        leftMap->insert(rightMap->begin(), rightMap->end());
        delete rightMap;

        return leftMap;
    }

    SymbolMap* walkMultiplyNode(MultiplyNode* node) override {
        SymbolMap* leftMap = walk(node->left());
        SymbolMap* rightMap = walk(node->right());

        leftMap->insert(rightMap->begin(), rightMap->end());
        delete rightMap;

        return leftMap;
    }

    SymbolMap* walkDivideNode(DivideNode* node) override {
        SymbolMap* leftMap = walk(node->left());
        SymbolMap* rightMap = walk(node->right());

        leftMap->insert(rightMap->begin(), rightMap->end());
        delete rightMap;

        return leftMap;
    }

    SymbolMap* walkModulusNode(ModulusNode* node) override {
        SymbolMap* leftMap = walk(node->left());
        SymbolMap* rightMap = walk(node->right());

        leftMap->insert(rightMap->begin(), rightMap->end());
        delete rightMap;

        return leftMap;
    }

    SymbolMap* walkPowerNode(PowerNode* node) override {
        SymbolMap* leftMap = walk(node->left());
        SymbolMap* rightMap = walk(node->right());

        leftMap->insert(rightMap->begin(), rightMap->end());
        delete rightMap;

        return leftMap;
    }

    SymbolMap* walkNegativeExpressionNode(NegativeExpressionNode* node) override {
        return walk(node->exp());
    }

    SymbolMap* walkNotNode(NotNode* node) override {
        return walk(node->exp());
    }

    SymbolMap* walkEnumerationLiteralExpressionNode(EnumerationLiteralExpressionNode* node) override {
        auto map = new SymbolMap();

        for (auto i : *node->actuals()) {
            SymbolMap* m = walk(i);
            map->insert(m->begin(), m->end());
            delete m;
        }

        return map;
    }

    SymbolMap* walkEnumerationStatement(EnumerationStatement* node) override {
        SymbolMap* map = walk(node->local());

        for (auto i : *node->body()) {
            SymbolMap* m = walk(i);
            map->insert(m->begin(), m->end());
            delete m;
        }

        return map;
    }

    SymbolMap* walkWithStatement(WithStatement* node) override {
        SymbolMap* map = walk(node->local());

        for (auto i : *node->body()) {
            SymbolMap* m = walk(i);
            map->insert(m->begin(), m->end());
            delete m;
        }

        return map;
    }

    SymbolMap* walkIfStatement(IfStatement* node) override {
        SymbolMap* map = walk(node->condition());

        for (auto i : *node->body()) {
            SymbolMap* m = walk(i);
            map->insert(m->begin(), m->end());
            delete m;
        }

        return map;
    }

    SymbolMap* walkWhileStatement(WhileStatement* node) override {
        SymbolMap* map = walk(node->condition());

        for (auto i : *node->body()) {
            SymbolMap* m = walk(i);
            map->insert(m->begin(), m->end());
            delete m;
        }

        return map;
    }

    SymbolMap* walkContinueNode(ContinueNode* node) override {
        return new SymbolMap();
    }

    SymbolMap* walkBreakNode(BreakNode* node) override {
        return new SymbolMap();
    }

    SymbolMap* walkReturnStatementNode(ReturnStatementNode* node) override {
        if ( node->value() != nullptr ) {
            return walk(node->value());
        }

        return new SymbolMap();
    }

    SymbolMap* walkMapStatementNode(MapStatementNode* node) override {
        return walk(node->value());
    }

    SymbolMap* walkMapNode(MapNode* node) override {
        auto map = new SymbolMap();

        for (auto i : *node->body()) {
            SymbolMap* m = walk(i);
            map->insert(m->begin(), m->end());
            delete m;
        }

        return map;
    }

    SymbolMap* walkStringLiteralExpressionNode(StringLiteralExpressionNode* node) override {
        return new SymbolMap();
    }

    SymbolMap* walkNumberLiteralExpressionNode(NumberLiteralExpressionNode* node) override {
        return new SymbolMap();
    }

    SymbolMap* walkAssignExpressionNode(AssignExpressionNode* node) override {
        SymbolMap* leftMap = walk(node->dest());
        SymbolMap* rightMap = walk(node->value());

        leftMap->insert(rightMap->begin(), rightMap->end());
        delete rightMap;

        return leftMap;
    }

    SymbolMap* walkUnitNode(UnitNode* node) override {
        return new SymbolMap();
    }

    // TODO: determine if I need to walk formals
    SymbolMap* walkFunctionNode(FunctionNode* node) override {
        auto map = new SymbolMap();

        for (auto i : *node->body()) {
            SymbolMap* m = walk(i);
            map->insert(m->begin(), m->end());
            delete m;
        }

        return map;
    }

    SymbolMap* walkNumericComparisonExpressionNode(NumericComparisonExpressionNode* node) override {
        SymbolMap* leftMap = walk(node->left());
        SymbolMap* rightMap = walk(node->right());

        leftMap->insert(rightMap->begin(), rightMap->end());
        delete rightMap;

        return leftMap;
    }

    SymbolMap* walkTypeBodyNode(TypeBodyNode* node) override {
        return new SymbolMap();
    }

    SymbolMap* walkClassAccessNode(ClassAccessNode* node) override {
        SymbolMap* path = walk(node->path());
        SymbolMap* end = walk(node->end());

        path->insert(end->begin(), end->end());
        delete end;

        return path;
    }
    
    SymbolMap* walkIncludeStatementNode(IncludeStatementNode* node) override {
        return walk(node->path());
    }

    [[nodiscard]] std::string toString() const override {
        return "SymbolWalk<>";
    }
};

}

#endif