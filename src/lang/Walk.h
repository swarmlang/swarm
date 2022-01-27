#ifndef SWARMC_WALK_H
#define SWARMC_WALK_H

#include "../shared/IStringable.h"
#include "../errors/SwarmError.h"
#include "AST.h"

namespace swarmc {
namespace Lang {

    template <typename TReturn>
    class Walk : public IStringable {
    public:
        virtual TReturn walk(ASTNode* node) {
            if ( node->getName() == "ProgramNode" ) return walkProgramNode(node);
            if ( node->getName() == "ExpressionStatementNode" ) return walkExpressionStatementNode(node);
            if ( node->getName() == "IdentifierNode" ) return walkIdentifierNode(node);
            if ( node->getName() == "PrimitiveTypeNode" ) return walkPrimitiveTypeNode(node);
            if ( node->getName() == "EnumerableTypeNode" ) return walkEnumerableTypeNode(node);
            if ( node->getName() == "MapTypeNode" ) return walkMapTypeNode(node);
            if ( node->getName() == "BooleanLiteralExpressionNode" ) return walkBooleanLiteralExpressionNode(node);
            if ( node->getName() == "VariableDeclarationNode" ) return walkVariableDeclarationNode(node);
            if ( node->getName() == "CallExpressionNode" ) return walkCallExpressionNode(node);
            if ( node->getName() == "AndNode" ) return walkAndNode(node);
            if ( node->getName() == "OrNode" ) return walkOrNode(node);
            if ( node->getName() == "EqualsNode" ) return walkEqualsNode(node);
            if ( node->getName() == "NotEqualsNode" ) return walkNotEqualsNode(node);
            if ( node->getName() == "AddNode" ) return walkAddNode(node);
            if ( node->getName() == "AddAssignExpressionNode" ) return walkAddAssignExpressionNode(node);
            if ( node->getName() == "SubtractNode" ) return walkSubtractNode(node);
            if ( node->getName() == "MultiplyNode" ) return walkMultiplyNode(node);
            if ( node->getName() == "MultiplyAssignExpressionNode" ) return walkMultiplyAssignExpressionNode(node);
            if ( node->getName() == "DivideNode" ) return walkDivideNode(node);
            if ( node->getName() == "ModulusNode" ) return walkModulusNode(node);
            if ( node->getName() == "PowerNode" ) return walkPowerNode(node);
            if ( node->getName() == "ConcatenateNode" ) return walkConcatenateNode(node);
            if ( node->getName() == "NotNode" ) return walkNotNode(node);
            if ( node->getName() == "EnumerationLiteralExpressionNode" ) return walkEnumerationLiteralExpressionNode(node);
            if ( node->getName() == "EnumerationStatement" ) return walkEnumerationStatement(node);
            if ( node->getName() == "WithStatement" ) return walkWithStatement(node);
            if ( node->getName() == "IfStatement" ) return walkIfStatement(node);
            if ( node->getName() == "WhileStatement" ) return walkWhileStatement(node);
            if ( node->getName() == "MapStatementNode" ) return walkMapStatementNode(node);
            if ( node->getName() == "MapNode" ) return walkMapNode(node);
            if ( node->getName() == "StringLiteralExpressionNode" ) return walkStringLiteralExpressionNode(node);
            if ( node->getName() == "NumberLiteralExpressionNode" ) return walkNumberLiteralExpressionNode(node);
            if ( node->getName() == "AssignExpressionNode" ) return walkAssignExpressionNode(node);

            throw Errors::SwarmError("Invalid node type: " + node->getName());
        }
    protected:
        virtual TReturn walkProgramNode(ProgramNode* node) = 0;
        virtual TReturn walkExpressionStatementNode(ExpressionStatementNode* node) = 0;
        virtual TReturn walkIdentifierNode(IdentifierNode* node) = 0;
        virtual TReturn walkPrimitiveTypeNode(PrimitiveTypeNode* node) = 0;
        virtual TReturn walkEnumerableTypeNode(EnumerableTypeNode* node) = 0;
        virtual TReturn walkMapTypeNode(MapTypeNode* node) = 0;
        virtual TReturn walkBooleanLiteralExpressionNode(BooleanLiteralExpressionNode* node) = 0;
        virtual TReturn walkVariableDeclarationNode(VariableDeclarationNode* node) = 0;
        virtual TReturn walkCallExpressionNode(CallExpressionNode* node) = 0;
        virtual TReturn walkAndNode(AndNode* node) = 0;
        virtual TReturn walkOrNode(OrNode* node) = 0;
        virtual TReturn walkEqualsNode(EqualsNode* node) = 0;
        virtual TReturn walkNotEqualsNode(NotEqualsNode* node) = 0;
        virtual TReturn walkAddNode(AddNode* node) = 0;
        virtual TReturn walkAddAssignExpressionNode(AddAssignExpressionNode* node) = 0;
        virtual TReturn walkSubtractNode(SubtractNode* node) = 0;
        virtual TReturn walkMultiplyNode(MultiplyNode* node) = 0;
        virtual TReturn walkMultiplyAssignExpressionNode(MultiplyAssignExpressionNode* node) = 0;
        virtual TReturn walkDivideNode(DivideNode* node) = 0;
        virtual TReturn walkModulusNode(ModulusNode* node) = 0;
        virtual TReturn walkPowerNode(PowerNode* node) = 0;
        virtual TReturn walkConcatenateNode(ConcatenateNode* node) = 0;
        virtual TReturn walkNotNode(NotNode* node) = 0;
        virtual TReturn walkEnumerationLiteralExpressionNode(EnumerationLiteralExpressionNode* node) = 0;
        virtual TReturn walkEnumerationStatement(EnumerationStatement* node) = 0;
        virtual TReturn walkWithStatement(WithStatement* node) = 0;
        virtual TReturn walkIfStatement(IfStatement* node) = 0;
        virtual TReturn walkWhileStatement(WhileStatement* node) = 0;
        virtual TReturn walkMapStatementNode(MapStatementNode* node) = 0;
        virtual TReturn walkMapNode(MapNode* node) = 0;
        virtual TReturn walkStringLiteralExpressionNode(StringLiteralExpressionNode* node) = 0;
        virtual TReturn walkNumberLiteralExpressionNode(NumberLiteralExpressionNode* node) = 0;
        virtual TReturn walkAssignExpressionNode(AssignExpressionNode* node) = 0;

        virtual std::string toString() const = 0;
    };

}
}

#endif
