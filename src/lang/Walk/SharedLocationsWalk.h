#ifndef SWARMC_SHARED_LOCATIONS_WALK_H
#define SWARMC_SHARED_LOCATIONS_WALK_H

#include "Walk.h"

namespace swarmc::ISA {
    class LocationReference;
}

namespace swarmc::Lang::Walk {

using SharedLocationsMap = std::unordered_map<const SemanticSymbol*, std::size_t>;
class SharedLocationsWalk;

class SharedLocations {
public:
    SharedLocations() = default;
    [[nodiscard]] bool has(const ISA::LocationReference* loc) const;
    [[nodiscard]] bool locked(const ISA::LocationReference* loc) const;
    void dec(const ISA::LocationReference* loc);
    void remove(const ISA::LocationReference* loc);
    bool shouldUnlock(const ISA::LocationReference* loc) const;
    [[nodiscard]] SharedLocationsMap::size_type size() const;
    [[nodiscard]] SharedLocationsMap::iterator begin() noexcept;
    [[nodiscard]] SharedLocationsMap::iterator end() noexcept;
    static void registerLoc(const ISA::LocationReference* loc, const SemanticSymbol* sym);
private:
    SharedLocationsMap _map;
    std::unordered_map<const SemanticSymbol*, bool> _locked;

    static std::map<std::string, const SemanticSymbol*> _locRefToSymbol;

    friend SharedLocationsWalk;
};

class SharedLocationsWalk : public Walk<SharedLocationsMap> {
public:
    static SharedLocations getLocs(ASTNode* node);

    [[nodiscard]] std::string toString() const override {
        return "SharedLocationsWalk<>";
    }
protected:
    SharedLocationsWalk();
    [[nodiscard]] virtual SharedLocationsMap walkProgramNode(ProgramNode* node) override;
    [[nodiscard]] virtual SharedLocationsMap walkExpressionStatementNode(ExpressionStatementNode* node) override;
    [[nodiscard]] virtual SharedLocationsMap walkIdentifierNode(IdentifierNode* node) override;
    [[nodiscard]] virtual SharedLocationsMap walkEnumerableAccessNode(EnumerableAccessNode* node) override;
    [[nodiscard]] virtual SharedLocationsMap walkEnumerableAppendNode(EnumerableAppendNode* node) override;
    [[nodiscard]] virtual SharedLocationsMap walkMapAccessNode(MapAccessNode* node) override;
    [[nodiscard]] virtual SharedLocationsMap walkClassAccessNode(ClassAccessNode* node) override;
    [[nodiscard]] virtual SharedLocationsMap walkIncludeStatementNode(IncludeStatementNode* node) override;
    [[nodiscard]] virtual SharedLocationsMap walkTypeLiteral(TypeLiteral* node) override;
    [[nodiscard]] virtual SharedLocationsMap walkBooleanLiteralExpressionNode(BooleanLiteralExpressionNode* node) override;
    [[nodiscard]] virtual SharedLocationsMap walkStringLiteralExpressionNode(StringLiteralExpressionNode* node) override;
    [[nodiscard]] virtual SharedLocationsMap walkNumberLiteralExpressionNode(NumberLiteralExpressionNode* node) override;
    [[nodiscard]] virtual SharedLocationsMap walkEnumerationLiteralExpressionNode(EnumerationLiteralExpressionNode* node) override;
    [[nodiscard]] virtual SharedLocationsMap walkMapStatementNode(MapStatementNode* node) override;
    [[nodiscard]] virtual SharedLocationsMap walkMapNode(MapNode* node) override;
    [[nodiscard]] virtual SharedLocationsMap walkAssignExpressionNode(AssignExpressionNode* node) override;
    [[nodiscard]] virtual SharedLocationsMap walkVariableDeclarationNode(VariableDeclarationNode* node) override;
    [[nodiscard]] virtual SharedLocationsMap walkUninitializedVariableDeclarationNode(UninitializedVariableDeclarationNode* node) override;
    [[nodiscard]] virtual SharedLocationsMap walkUseNode(UseNode* node) override;
    [[nodiscard]] virtual SharedLocationsMap walkReturnStatementNode(ReturnStatementNode* node) override;
    [[nodiscard]] virtual SharedLocationsMap walkFunctionNode(FunctionNode* node) override;
    [[nodiscard]] virtual SharedLocationsMap walkConstructorNode(ConstructorNode* node) override;
    [[nodiscard]] virtual SharedLocationsMap walkTypeBodyNode(TypeBodyNode* node) override;
    [[nodiscard]] virtual SharedLocationsMap walkCallExpressionNode(CallExpressionNode* node) override;
    [[nodiscard]] virtual SharedLocationsMap walkDeferCallExpressionNode(DeferCallExpressionNode* node) override;
    [[nodiscard]] virtual SharedLocationsMap walkAndNode(AndNode* node) override;
    [[nodiscard]] virtual SharedLocationsMap walkOrNode(OrNode* node) override;
    [[nodiscard]] virtual SharedLocationsMap walkEqualsNode(EqualsNode* node) override;
    [[nodiscard]] virtual SharedLocationsMap walkNumericComparisonExpressionNode(NumericComparisonExpressionNode* node) override;
    [[nodiscard]] virtual SharedLocationsMap walkNotEqualsNode(NotEqualsNode* node) override;
    [[nodiscard]] virtual SharedLocationsMap walkAddNode(AddNode* node) override;
    [[nodiscard]] virtual SharedLocationsMap walkSubtractNode(SubtractNode* node) override;
    [[nodiscard]] virtual SharedLocationsMap walkMultiplyNode(MultiplyNode* node) override;
    [[nodiscard]] virtual SharedLocationsMap walkDivideNode(DivideNode* node) override;
    [[nodiscard]] virtual SharedLocationsMap walkModulusNode(ModulusNode* node) override;
    [[nodiscard]] virtual SharedLocationsMap walkPowerNode(PowerNode* node) override;
    [[nodiscard]] virtual SharedLocationsMap walkNthRootNode(NthRootNode* node) override;
    [[nodiscard]] virtual SharedLocationsMap walkNegativeExpressionNode(NegativeExpressionNode* node) override;
    [[nodiscard]] virtual SharedLocationsMap walkNotNode(NotNode* node) override;
    [[nodiscard]] virtual SharedLocationsMap walkEnumerationStatement(EnumerationStatement* node) override;
    [[nodiscard]] virtual SharedLocationsMap walkWithStatement(WithStatement* node) override;
    [[nodiscard]] virtual SharedLocationsMap walkIfStatement(IfStatement* node) override;
    [[nodiscard]] virtual SharedLocationsMap walkWhileStatement(WhileStatement* node) override;
    [[nodiscard]] virtual SharedLocationsMap walkContinueNode(ContinueNode* node) override;
    [[nodiscard]] virtual SharedLocationsMap walkBreakNode(BreakNode* node) override;
protected:
    SharedLocationsMap walkBinaryExpressionNode(BinaryExpressionNode* node);
    SharedLocationsMap walkUnaryExpressionNode(UnaryExpressionNode* node);
    void combine(SharedLocationsMap& first, SharedLocationsMap&& second);
};

}

#endif