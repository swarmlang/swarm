#include <string>
#include "AST.h"
#include "SymbolTable.h"
#include "Walk/PrintWalk.h"

#ifndef SWARMC_SPACE
#define SWARMC_SPACE "  "
#endif

namespace swarmc::Lang {

    ProgramNode* ProgramNode::copy() const  {
        auto other = new ProgramNode;
        for ( auto stmt : *_body ) {
            other->pushStatement(stmt->copy());
        }

        return other;
    }

    size_t ConstructorNode::nameID = 0;

}

namespace nslib {
    [[nodiscard]] std::string s(swarmc::Lang::ASTNodeTag tag) {
        if ( tag == swarmc::Lang::ASTNodeTag::PROGRAM ) return "ProgramNode";
        if ( tag == swarmc::Lang::ASTNodeTag::EXPRESSIONSTATEMENT ) return "ExpressionStatementNode";
        if ( tag == swarmc::Lang::ASTNodeTag::IDENTIFIER ) return "IdentifierNode";
        if ( tag == swarmc::Lang::ASTNodeTag::ENUMERABLEACCESS ) return "EnumerableAccessNode";
        if ( tag == swarmc::Lang::ASTNodeTag::ENUMERABLEAPPEND ) return "EnumerableAppendNode";
        if ( tag == swarmc::Lang::ASTNodeTag::MAPACCESS ) return "MapAccessNode";
        if ( tag == swarmc::Lang::ASTNodeTag::CLASSACCESS ) return "ClassAccessNode";
        if ( tag == swarmc::Lang::ASTNodeTag::INCLUDE ) return "IncludeStatementNode";
        if ( tag == swarmc::Lang::ASTNodeTag::TYPELITERAL ) return "TypeLiteralNode";
        if ( tag == swarmc::Lang::ASTNodeTag::BOOLEANLITERAL ) return "BooleanLiteralNode";
        if ( tag == swarmc::Lang::ASTNodeTag::STRINGLITERAL ) return "StringLiteralNode";
        if ( tag == swarmc::Lang::ASTNodeTag::NUMBERLITERAL ) return "NumberLiteralNode";
        if ( tag == swarmc::Lang::ASTNodeTag::ENUMERATIONLITERAL ) return "EnumerationLiteralNode";
        if ( tag == swarmc::Lang::ASTNodeTag::MAPSTATEMENT ) return "MapStatementNode";
        if ( tag == swarmc::Lang::ASTNodeTag::MAPLITERAL ) return "MapLiteralNode";
        if ( tag == swarmc::Lang::ASTNodeTag::ASSIGN ) return "AssignNode";
        if ( tag == swarmc::Lang::ASTNodeTag::VARIABLEDECLARATION ) return "VariableDeclarationNode";
        if ( tag == swarmc::Lang::ASTNodeTag::UNINITIALIZEDVARIABLEDECLARATION ) return "UninitializedVariableDeclarationNode";
        if ( tag == swarmc::Lang::ASTNodeTag::RETURN ) return "ReturnNode";
        if ( tag == swarmc::Lang::ASTNodeTag::FUNCTION ) return "FunctionNode";
        if ( tag == swarmc::Lang::ASTNodeTag::CONSTRUCTOR ) return "ConstructorNode";
        if ( tag == swarmc::Lang::ASTNodeTag::TYPEBODY ) return "TypeBodyNode";
        if ( tag == swarmc::Lang::ASTNodeTag::CALL ) return "CallNode";
        if ( tag == swarmc::Lang::ASTNodeTag::DEFERCALL ) return "DeferCallNode";
        if ( tag == swarmc::Lang::ASTNodeTag::AND ) return "AndNode";
        if ( tag == swarmc::Lang::ASTNodeTag::OR ) return "OrNode";
        if ( tag == swarmc::Lang::ASTNodeTag::EQUALS ) return "EqualsNode";
        if ( tag == swarmc::Lang::ASTNodeTag::NUMERICCOMPARISON ) return "NumericComparisonNode";
        if ( tag == swarmc::Lang::ASTNodeTag::NOTEQUALS ) return "NotEqualsNode";
        if ( tag == swarmc::Lang::ASTNodeTag::ADD ) return "AddNode";
        if ( tag == swarmc::Lang::ASTNodeTag::SUBTRACT ) return "SubtractNode";
        if ( tag == swarmc::Lang::ASTNodeTag::MULTIPLY ) return "MultiplyNode";
        if ( tag == swarmc::Lang::ASTNodeTag::DIVIDE ) return "DivideNode";
        if ( tag == swarmc::Lang::ASTNodeTag::MODULUS ) return "ModulusNode";
        if ( tag == swarmc::Lang::ASTNodeTag::POWER ) return "PowerNode";
        if ( tag == swarmc::Lang::ASTNodeTag::NEGATIVE ) return "NegativeNode";
        if ( tag == swarmc::Lang::ASTNodeTag::SQUAREROOT ) return "SquareRootNode";
        if ( tag == swarmc::Lang::ASTNodeTag::NOT ) return "NotNode";
        if ( tag == swarmc::Lang::ASTNodeTag::ENUMERATE ) return "EnumerateNode";
        if ( tag == swarmc::Lang::ASTNodeTag::WITH ) return "WithNode";
        if ( tag == swarmc::Lang::ASTNodeTag::IF ) return "IfNode";
        if ( tag == swarmc::Lang::ASTNodeTag::WHILE ) return "WhileNode";
        if ( tag == swarmc::Lang::ASTNodeTag::CONTINUE ) return "ContinueNode";
        if ( tag == swarmc::Lang::ASTNodeTag::BREAK ) return "BreakNode";
        return "UnknownNode";
    }
}
