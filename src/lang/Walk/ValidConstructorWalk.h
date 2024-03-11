#ifndef SWARMC_VALID_CONSTRUCTOR_WALK_H
#define SWARMC_VALID_CONSTRUCTOR_WALK_H

#include "Walk.h"

namespace swarmc::Lang::Walk {

class ValidConstructorWalk : Walk<void> {
public:
    [[nodiscard]] static bool isValidConstructor(TypeBodyNode* type, ConstructorNode* constructor);
protected:
    explicit ValidConstructorWalk() : Walk<void>("Constructor Verification") {}

    static void populate(ValidConstructorWalk& vcw, TypeBodyNode* type, ConstructorNode* constructor, std::string name);

    void walkProgramNode(ProgramNode* node) override;
    void walkExpressionStatementNode(ExpressionStatementNode* node) override;
    void walkIdentifierNode(IdentifierNode* node) override {}
    void walkEnumerableAccessNode(EnumerableAccessNode* node) override;
    void walkEnumerableAppendNode(EnumerableAppendNode* node) override;
    void walkMapAccessNode(MapAccessNode* node) override;
    void walkClassAccessNode(ClassAccessNode* node) override;
    void walkIncludeStatementNode(IncludeStatementNode* node) override {}
    void walkTypeLiteral(swarmc::Lang::TypeLiteral *node) override {}
    void walkBooleanLiteralExpressionNode(BooleanLiteralExpressionNode* node) override {}
    void walkStringLiteralExpressionNode(StringLiteralExpressionNode* node) override {}
    void walkNumberLiteralExpressionNode(NumberLiteralExpressionNode* node) override {}
    void walkEnumerationLiteralExpressionNode(EnumerationLiteralExpressionNode* node) override {}
    void walkMapStatementNode(MapStatementNode* node) override {}
    void walkMapNode(MapNode* node) override {}
    void walkAssignExpressionNode(AssignExpressionNode* node) override;
    void walkVariableDeclarationNode(VariableDeclarationNode* node) override;
    void walkUninitializedVariableDeclarationNode(UninitializedVariableDeclarationNode* node) override {}
    void walkUseNode(UseNode* node) override {}
    void walkReturnStatementNode(ReturnStatementNode* node) override;
    void walkFunctionNode(FunctionNode* node) override;
    void walkConstructorNode(ConstructorNode* node) override {}
    void walkTypeBodyNode(TypeBodyNode* node) override {}
    void walkCallExpressionNode(CallExpressionNode* node) override;
    void walkDeferCallExpressionNode(DeferCallExpressionNode* node) override;
    void walkAndNode(AndNode* node) override {}
    void walkOrNode(OrNode* node) override {}
    void walkEqualsNode(EqualsNode* node) override {}
    void walkNumericComparisonExpressionNode(NumericComparisonExpressionNode* node) override {}
    void walkNotEqualsNode(NotEqualsNode* node) override {}
    void walkAddNode(AddNode* node) override {}
    void walkSubtractNode(SubtractNode* node) override {}
    void walkMultiplyNode(MultiplyNode* node) override {}
    void walkDivideNode(DivideNode* node) override {}
    void walkModulusNode(ModulusNode* node) override {}
    void walkPowerNode(PowerNode* node) override {}
    void walkNthRootNode(NthRootNode* node) override {}
    void walkNegativeExpressionNode(NegativeExpressionNode* node) override {}
    void walkNotNode(NotNode* node) override {}
    void walkEnumerationStatement(EnumerationStatement* node) override {}
    void walkWithStatement(WithStatement* node) override;
    void walkIfStatement(IfStatement* node) override;
    void walkWhileStatement(WhileStatement* node) override;
    void walkContinueNode(ContinueNode* node) override {}
    void walkBreakNode(BreakNode* node) override {}

    [[nodiscard]] std::string toString() const override {
        return "ValidConstructorWalk<>";
    }

    void walkBlockStatementNode(BlockStatementNode* node);
private:
    void setPossibleFunctions(SemanticSymbol* destSym, ExpressionNode* value);

    std::set<SemanticSymbol*> _symbols;
    std::map<SemanticSymbol*, std::vector<FunctionNode*>> _possibleFunctions;
    std::map<SemanticSymbol*, std::string> _errNameMap;
    bool _inTopLayer = true, _goodReturns = true;
};

}

#endif
