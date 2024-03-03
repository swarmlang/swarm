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
            if ( node->getTag() == ASTNodeTag::PROGRAM ) return walkProgramNode((ProgramNode*) node);
            if ( node->getTag() == ASTNodeTag::EXPRESSIONSTATEMENT ) return walkExpressionStatementNode((ExpressionStatementNode*) node);
            if ( node->getTag() == ASTNodeTag::IDENTIFIER ) return walkIdentifierNode((IdentifierNode*) node);
            if ( node->getTag() == ASTNodeTag::ENUMERABLEACCESS ) return walkEnumerableAccessNode((EnumerableAccessNode*) node);
            if ( node->getTag() == ASTNodeTag::MAPACCESS ) return walkMapAccessNode((MapAccessNode*) node);
            if ( node->getTag() == ASTNodeTag::CLASSACCESS ) return walkClassAccessNode((ClassAccessNode*) node);
            if ( node->getTag() == ASTNodeTag::INCLUDE ) return walkIncludeStatementNode((IncludeStatementNode*) node);
            if ( node->getTag() == ASTNodeTag::TYPELITERAL ) return walkTypeLiteral((TypeLiteral*) node);
            if ( node->getTag() == ASTNodeTag::BOOLEANLITERAL ) return walkBooleanLiteralExpressionNode((BooleanLiteralExpressionNode*) node);
            if ( node->getTag() == ASTNodeTag::STRINGLITERAL ) return walkStringLiteralExpressionNode((StringLiteralExpressionNode*) node);
            if ( node->getTag() == ASTNodeTag::NUMBERLITERAL ) return walkNumberLiteralExpressionNode((NumberLiteralExpressionNode*) node);
            if ( node->getTag() == ASTNodeTag::ENUMERATIONLITERAL ) return walkEnumerationLiteralExpressionNode((EnumerationLiteralExpressionNode*) node);
            if ( node->getTag() == ASTNodeTag::MAPSTATEMENT ) return walkMapStatementNode((MapStatementNode*) node);
            if ( node->getTag() == ASTNodeTag::MAPLITERAL ) return walkMapNode((MapNode*) node);
            if ( node->getTag() == ASTNodeTag::ASSIGN ) return walkAssignExpressionNode((AssignExpressionNode*) node);
            if ( node->getTag() == ASTNodeTag::VARIABLEDECLARATION ) return walkVariableDeclarationNode((VariableDeclarationNode*) node);
            if ( node->getTag() == ASTNodeTag::UNINITIALIZEDVARIABLEDECLARATION ) return walkUninitializedVariableDeclarationNode((UninitializedVariableDeclarationNode*)node);
            if ( node->getTag() == ASTNodeTag::RETURN ) return walkReturnStatementNode((ReturnStatementNode*) node);
            if ( node->getTag() == ASTNodeTag::FUNCTION ) return walkFunctionNode((FunctionNode*) node);
            if ( node->getTag() == ASTNodeTag::CONSTRUCTOR ) return walkConstructorNode((ConstructorNode*)node);
            if ( node->getTag() == ASTNodeTag::TYPEBODY ) return walkTypeBodyNode((TypeBodyNode*) node);
            if ( node->getTag() == ASTNodeTag::CALL ) return walkCallExpressionNode((CallExpressionNode*) node);
            if ( node->getTag() == ASTNodeTag::DEFERCALL ) return walkDeferCallExpressionNode((DeferCallExpressionNode*) node);
            if ( node->getTag() == ASTNodeTag::AND ) return walkAndNode((AndNode*) node);
            if ( node->getTag() == ASTNodeTag::OR ) return walkOrNode((OrNode*) node);
            if ( node->getTag() == ASTNodeTag::EQUALS ) return walkEqualsNode((EqualsNode*) node);
            if ( node->getTag() == ASTNodeTag::NUMERICCOMPARISON ) return walkNumericComparisonExpressionNode((NumericComparisonExpressionNode*) node);
            if ( node->getTag() == ASTNodeTag::NOTEQUALS ) return walkNotEqualsNode((NotEqualsNode*) node);
            if ( node->getTag() == ASTNodeTag::ADD ) return walkAddNode((AddNode*) node);
            if ( node->getTag() == ASTNodeTag::SUBTRACT ) return walkSubtractNode((SubtractNode*) node);
            if ( node->getTag() == ASTNodeTag::MULTIPLY ) return walkMultiplyNode((MultiplyNode*) node);
            if ( node->getTag() == ASTNodeTag::DIVIDE ) return walkDivideNode((DivideNode*) node);
            if ( node->getTag() == ASTNodeTag::MODULUS ) return walkModulusNode((ModulusNode*) node);
            if ( node->getTag() == ASTNodeTag::POWER ) return walkPowerNode((PowerNode*) node);
            if ( node->getTag() == ASTNodeTag::NEGATIVE ) return walkNegativeExpressionNode((NegativeExpressionNode*) node);
            if ( node->getTag() == ASTNodeTag::SQUAREROOT ) return walkSqrtNode((SqrtNode*) node);
            if ( node->getTag() == ASTNodeTag::NOT ) return walkNotNode((NotNode*) node);
            if ( node->getTag() == ASTNodeTag::ENUMERATE ) return walkEnumerationStatement((EnumerationStatement*) node);
            if ( node->getTag() == ASTNodeTag::WITH ) return walkWithStatement((WithStatement*) node);
            if ( node->getTag() == ASTNodeTag::IF ) return walkIfStatement((IfStatement*) node);
            if ( node->getTag() == ASTNodeTag::WHILE ) return walkWhileStatement((WhileStatement*) node);
            if ( node->getTag() == ASTNodeTag::CONTINUE ) return walkContinueNode((ContinueNode*) node);
            if ( node->getTag() == ASTNodeTag::BREAK ) return walkBreakNode((BreakNode*) node);
            
            throw Errors::SwarmError("Invalid node type: " + s(node->getTag()));
        }
    protected:
        virtual TReturn walkProgramNode(ProgramNode* node) = 0;
        virtual TReturn walkExpressionStatementNode(ExpressionStatementNode* node) = 0;
        virtual TReturn walkIdentifierNode(IdentifierNode* node) = 0;
        virtual TReturn walkEnumerableAccessNode(EnumerableAccessNode* node) = 0;
        virtual TReturn walkMapAccessNode(MapAccessNode* node) = 0;
        virtual TReturn walkClassAccessNode(ClassAccessNode* node) = 0;
        virtual TReturn walkIncludeStatementNode(IncludeStatementNode* node) = 0;
        virtual TReturn walkTypeLiteral(TypeLiteral* node) = 0;
        virtual TReturn walkBooleanLiteralExpressionNode(BooleanLiteralExpressionNode* node) = 0;
        virtual TReturn walkStringLiteralExpressionNode(StringLiteralExpressionNode* node) = 0;
        virtual TReturn walkNumberLiteralExpressionNode(NumberLiteralExpressionNode* node) = 0;
        virtual TReturn walkEnumerationLiteralExpressionNode(EnumerationLiteralExpressionNode* node) = 0;
        virtual TReturn walkMapStatementNode(MapStatementNode* node) = 0;
        virtual TReturn walkMapNode(MapNode* node) = 0;
        virtual TReturn walkAssignExpressionNode(AssignExpressionNode* node) = 0;
        virtual TReturn walkVariableDeclarationNode(VariableDeclarationNode* node) = 0;
        virtual TReturn walkUninitializedVariableDeclarationNode(UninitializedVariableDeclarationNode* node) = 0;
        virtual TReturn walkReturnStatementNode(ReturnStatementNode* node) = 0;
        virtual TReturn walkFunctionNode(FunctionNode* node) = 0;
        virtual TReturn walkConstructorNode(ConstructorNode* node) = 0;
        virtual TReturn walkTypeBodyNode(TypeBodyNode* node) = 0;
        virtual TReturn walkCallExpressionNode(CallExpressionNode* node) = 0;
        virtual TReturn walkDeferCallExpressionNode(DeferCallExpressionNode* node) = 0;
        virtual TReturn walkAndNode(AndNode* node) = 0;
        virtual TReturn walkOrNode(OrNode* node) = 0;
        virtual TReturn walkEqualsNode(EqualsNode* node) = 0;
        virtual TReturn walkNumericComparisonExpressionNode(NumericComparisonExpressionNode* node) = 0;
        virtual TReturn walkNotEqualsNode(NotEqualsNode* node) = 0;
        virtual TReturn walkAddNode(AddNode* node) = 0;
        virtual TReturn walkSubtractNode(SubtractNode* node) = 0;
        virtual TReturn walkMultiplyNode(MultiplyNode* node) = 0;
        virtual TReturn walkDivideNode(DivideNode* node) = 0;
        virtual TReturn walkModulusNode(ModulusNode* node) = 0;
        virtual TReturn walkPowerNode(PowerNode* node) = 0;
        virtual TReturn walkNegativeExpressionNode(NegativeExpressionNode* node) = 0;
        virtual TReturn walkSqrtNode(SqrtNode* node) = 0;
        virtual TReturn walkNotNode(NotNode* node) = 0;
        virtual TReturn walkEnumerationStatement(EnumerationStatement* node) = 0;
        virtual TReturn walkWithStatement(WithStatement* node) = 0;
        virtual TReturn walkIfStatement(IfStatement* node) = 0;
        virtual TReturn walkWhileStatement(WhileStatement* node) = 0;
        virtual TReturn walkContinueNode(ContinueNode* node) = 0;
        virtual TReturn walkBreakNode(BreakNode* node) = 0;
    };

}

#endif
