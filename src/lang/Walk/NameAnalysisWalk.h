#ifndef SWARMC_NAME_ANALYSIS_WALK_H
#define SWARMC_NAME_ANALYSIS_WALK_H

#include "Walk.h"

namespace swarmc::Lang::Walk {

class NameAnalysisWalk : public Walk<bool> {
public:
    NameAnalysisWalk();
    ~NameAnalysisWalk() override;
protected:
    bool walkProgramNode(ProgramNode* node) override;
    bool walkExpressionStatementNode(ExpressionStatementNode* node) override;
    bool walkIdentifierNode(IdentifierNode* node) override;
    bool walkEnumerableAccessNode(EnumerableAccessNode* node) override;
    bool walkEnumerableAppendNode(EnumerableAppendNode* node) override;
    bool walkMapAccessNode(MapAccessNode* node) override;
    bool walkClassAccessNode(ClassAccessNode* node) override;
    bool walkIncludeStatementNode(IncludeStatementNode* node) override;
    bool walkTypeLiteral(swarmc::Lang::TypeLiteral *node) override;
    bool walkBooleanLiteralExpressionNode(BooleanLiteralExpressionNode* node) override;
    bool walkStringLiteralExpressionNode(StringLiteralExpressionNode* node) override;
    bool walkNumberLiteralExpressionNode(NumberLiteralExpressionNode* node) override;
    bool walkEnumerationLiteralExpressionNode(EnumerationLiteralExpressionNode* node) override;
    bool walkMapStatementNode(MapStatementNode* node) override;
    bool walkMapNode(MapNode* node) override;
    bool walkAssignExpressionNode(AssignExpressionNode* node) override;
    bool walkVariableDeclarationNode(VariableDeclarationNode* node) override;
    bool walkUninitializedVariableDeclarationNode(UninitializedVariableDeclarationNode* node) override;
    bool walkUseNode(UseNode* node) override;
    bool walkReturnStatementNode(ReturnStatementNode* node) override;
    bool walkFunctionNode(FunctionNode* node) override;
    bool walkConstructorNode(ConstructorNode* node) override;
    bool walkTypeBodyNode(TypeBodyNode* node) override;
    bool walkCallExpressionNode(CallExpressionNode* node) override;
    bool walkDeferCallExpressionNode(DeferCallExpressionNode* node) override;
    bool walkAndNode(AndNode* node) override;
    bool walkOrNode(OrNode* node) override;
    bool walkEqualsNode(EqualsNode* node) override;
    bool walkNumericComparisonExpressionNode(NumericComparisonExpressionNode* node) override;
    bool walkNotEqualsNode(NotEqualsNode* node) override;
    bool walkAddNode(AddNode* node) override;
    bool walkSubtractNode(SubtractNode* node) override;
    bool walkMultiplyNode(MultiplyNode* node) override;
    bool walkDivideNode(DivideNode* node) override;
    bool walkModulusNode(ModulusNode* node) override;
    bool walkPowerNode(PowerNode* node) override;
    bool walkNegativeExpressionNode(NegativeExpressionNode* node) override;
    bool walkSqrtNode(SqrtNode* node) override;
    bool walkNotNode(NotNode* node) override;
    bool walkEnumerationStatement(EnumerationStatement* node) override;
    bool walkWithStatement(WithStatement* node) override;
    bool walkIfStatement(IfStatement* node) override;
    bool walkWhileStatement(WhileStatement* node) override;
    bool walkContinueNode(ContinueNode* node) override;
    bool walkBreakNode(BreakNode* node) override;

    [[nodiscard]] std::string toString() const override {
        return "NameAnalysisWalk<>";
    }
private:
    SymbolTable* _symbols;
    std::stack<Type::Object*> _objects;

    bool walkBlockStatementNode(BlockStatementNode* node) ;
    bool walkType(Type::Type* type);
    bool walkTypeRec(Type::Type* type, std::set<std::size_t>& visited);
};

}

#endif
