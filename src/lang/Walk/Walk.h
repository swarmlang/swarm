#ifndef SWARMC_WALK_H
#define SWARMC_WALK_H

#include "../shared/nslib.h"
#include "../errors/SwarmError.h"
#include "AST.h"

using namespace nslib;

namespace swarmc::Lang::Walk {

    template <typename TReturn>
    class Walk : public IStringable, public IUsesConsole {
    public:
        Walk() : IUsesConsole() {}

        virtual TReturn walk(ASTNode* node) {
            if ( node->getName() == "ProgramNode" ) return walkProgramNode((ProgramNode*) node);
            if ( node->getName() == "ExpressionStatementNode" ) return walkExpressionStatementNode((ExpressionStatementNode*) node);
            if ( node->getName() == "IdentifierNode" ) return walkIdentifierNode((IdentifierNode*) node);
            if ( node->getName() == "MapAccessNode" ) return walkMapAccessNode((MapAccessNode*) node);
            if ( node->getName() == "EnumerableAccessNode" ) return walkEnumerableAccessNode((EnumerableAccessNode*) node);
            if ( node->getName() == "TypeLiteral" ) return walkTypeLiteral((TypeLiteral*) node);
            if ( node->getName() == "BooleanLiteralExpressionNode" ) return walkBooleanLiteralExpressionNode((BooleanLiteralExpressionNode*) node);
            if ( node->getName() == "VariableDeclarationNode" ) return walkVariableDeclarationNode((VariableDeclarationNode*) node);
            if ( node->getName() == "CallExpressionNode" ) return walkCallExpressionNode((CallExpressionNode*) node);
            if ( node->getName() == "IIFExpressionNode" ) return walkIIFExpressionNode((IIFExpressionNode*) node);
            if ( node->getName() == "AndNode" ) return walkAndNode((AndNode*) node);
            if ( node->getName() == "OrNode" ) return walkOrNode((OrNode*) node);
            if ( node->getName() == "EqualsNode" ) return walkEqualsNode((EqualsNode*) node);
            if ( node->getName() == "NotEqualsNode" ) return walkNotEqualsNode((NotEqualsNode*) node);
            if ( node->getName() == "AddNode" ) return walkAddNode((AddNode*) node);
            if ( node->getName() == "SubtractNode" ) return walkSubtractNode((SubtractNode*) node);
            if ( node->getName() == "MultiplyNode" ) return walkMultiplyNode((MultiplyNode*) node);
            if ( node->getName() == "DivideNode" ) return walkDivideNode((DivideNode*) node);
            if ( node->getName() == "ModulusNode" ) return walkModulusNode((ModulusNode*) node);
            if ( node->getName() == "PowerNode" ) return walkPowerNode((PowerNode*) node);
            if ( node->getName() == "NegativeExpressionNode" ) return walkNegativeExpressionNode((NegativeExpressionNode*) node);
            if ( node->getName() == "NotNode" ) return walkNotNode((NotNode*) node);
            if ( node->getName() == "EnumerationLiteralExpressionNode" ) return walkEnumerationLiteralExpressionNode((EnumerationLiteralExpressionNode*) node);
            if ( node->getName() == "EnumerationStatement" ) return walkEnumerationStatement((EnumerationStatement*) node);
            if ( node->getName() == "WithStatement" ) return walkWithStatement((WithStatement*) node);
            if ( node->getName() == "IfStatement" ) return walkIfStatement((IfStatement*) node);
            if ( node->getName() == "WhileStatement" ) return walkWhileStatement((WhileStatement*) node);
            if ( node->getName() == "ContinueNode" ) return walkContinueNode((ContinueNode*) node);
            if ( node->getName() == "BreakNode" ) return walkBreakNode((BreakNode*) node);
            if ( node->getName() == "ReturnStatementNode" ) return walkReturnStatementNode((ReturnStatementNode*) node);
            if ( node->getName() == "MapStatementNode" ) return walkMapStatementNode((MapStatementNode*) node);
            if ( node->getName() == "MapNode" ) return walkMapNode((MapNode*) node);
            if ( node->getName() == "StringLiteralExpressionNode" ) return walkStringLiteralExpressionNode((StringLiteralExpressionNode*) node);
            if ( node->getName() == "AssignExpressionNode" ) return walkAssignExpressionNode((AssignExpressionNode*) node);
            if ( node->getName() == "NumberLiteralExpressionNode" ) return walkNumberLiteralExpressionNode((NumberLiteralExpressionNode*) node);
            if ( node->getName() == "IntegerLiteralExpressionNode" ) return walkIntegerLiteralExpressionNode((IntegerLiteralExpressionNode*) node);
            if ( node->getName() == "UnitNode" ) return walkUnitNode((UnitNode*) node);
            if ( node->getName() == "FunctionNode" ) return walkFunctionNode((FunctionNode*) node);
            if ( node->getName() == "NumericComparisonExpressionNode" ) return walkNumericComparisonExpressionNode((NumericComparisonExpressionNode*) node);
            if ( node->getName() == "TypeBodyNode" ) return walkTypeBodyNode((TypeBodyNode*) node);
            if ( node->getName() == "ClassAccessNode" ) return walkClassAccessNode((ClassAccessNode*) node);
            if ( node->getName() == "IncludeStatementNode") return walkIncludeStatementNode((IncludeStatementNode*) node);
            if ( node->getName() == "ConstructorNode" ) return walkConstructorNode((ConstructorNode*)node);
            if ( node->getName() == "UninitializedVariableDeclarationNode" ) return walkUninitializedVariableDeclarationNode((UninitializedVariableDeclarationNode*)node);

            throw Errors::SwarmError("Invalid node type: " + node->getName());
        }
    protected:
        virtual TReturn walkProgramNode(ProgramNode* node) = 0;
        virtual TReturn walkExpressionStatementNode(ExpressionStatementNode* node) = 0;
        virtual TReturn walkIdentifierNode(IdentifierNode* node) = 0;
        virtual TReturn walkMapAccessNode(MapAccessNode* node) = 0;
        virtual TReturn walkEnumerableAccessNode(EnumerableAccessNode* node) = 0;
        virtual TReturn walkTypeLiteral(TypeLiteral* node) = 0;
        virtual TReturn walkBooleanLiteralExpressionNode(BooleanLiteralExpressionNode* node) = 0;
        virtual TReturn walkVariableDeclarationNode(VariableDeclarationNode* node) = 0;
        virtual TReturn walkCallExpressionNode(CallExpressionNode* node) = 0;
        virtual TReturn walkIIFExpressionNode(IIFExpressionNode* node) = 0;
        virtual TReturn walkAndNode(AndNode* node) = 0;
        virtual TReturn walkOrNode(OrNode* node) = 0;
        virtual TReturn walkEqualsNode(EqualsNode* node) = 0;
        virtual TReturn walkNotEqualsNode(NotEqualsNode* node) = 0;
        virtual TReturn walkAddNode(AddNode* node) = 0;
        virtual TReturn walkSubtractNode(SubtractNode* node) = 0;
        virtual TReturn walkMultiplyNode(MultiplyNode* node) = 0;
        virtual TReturn walkDivideNode(DivideNode* node) = 0;
        virtual TReturn walkModulusNode(ModulusNode* node) = 0;
        virtual TReturn walkPowerNode(PowerNode* node) = 0;
        virtual TReturn walkNegativeExpressionNode(NegativeExpressionNode* node) = 0;
        virtual TReturn walkNotNode(NotNode* node) = 0;
        virtual TReturn walkEnumerationLiteralExpressionNode(EnumerationLiteralExpressionNode* node) = 0;
        virtual TReturn walkEnumerationStatement(EnumerationStatement* node) = 0;
        virtual TReturn walkWithStatement(WithStatement* node) = 0;
        virtual TReturn walkIfStatement(IfStatement* node) = 0;
        virtual TReturn walkWhileStatement(WhileStatement* node) = 0;
        virtual TReturn walkContinueNode(ContinueNode* node) = 0;
        virtual TReturn walkBreakNode(BreakNode* node) = 0;
        virtual TReturn walkReturnStatementNode(ReturnStatementNode* node) = 0;
        virtual TReturn walkMapStatementNode(MapStatementNode* node) = 0;
        virtual TReturn walkMapNode(MapNode* node) = 0;
        virtual TReturn walkStringLiteralExpressionNode(StringLiteralExpressionNode* node) = 0;
        virtual TReturn walkNumberLiteralExpressionNode(NumberLiteralExpressionNode* node) = 0;
        virtual TReturn walkAssignExpressionNode(AssignExpressionNode* node) = 0;
        virtual TReturn walkUnitNode(UnitNode* node) = 0;
        virtual TReturn walkFunctionNode(FunctionNode* node) = 0;
        virtual TReturn walkNumericComparisonExpressionNode(NumericComparisonExpressionNode* node) = 0;
        virtual TReturn walkIntegerLiteralExpressionNode(IntegerLiteralExpressionNode* node) {
            return walkNumberLiteralExpressionNode(node);
        }
        virtual TReturn walkTypeBodyNode(TypeBodyNode* node) = 0;
        virtual TReturn walkClassAccessNode(ClassAccessNode* node) = 0;
        virtual TReturn walkIncludeStatementNode(IncludeStatementNode* node) = 0;
        virtual TReturn walkConstructorNode(ConstructorNode* node) = 0;
        virtual TReturn walkUninitializedVariableDeclarationNode(UninitializedVariableDeclarationNode* node) = 0;

        [[nodiscard]] std::string toString() const override = 0;
    };

}

#endif
