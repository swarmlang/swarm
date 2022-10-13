#ifndef SWARMC_PRINT_WALK_H
#define SWARMC_PRINT_WALK_H

#ifndef SWARMC_SPACE
#define SWARMC_SPACE "  "
#endif

#include <fstream>
#include "Walk.h"

namespace swarmc {
namespace Lang {
namespace Walk {

class PrintWalk : Walk<void> {
public:
    PrintWalk(std::ostream& out, ASTNode* node) : Walk<void>(), _out(out), _prefix("") {
        walk(node);
    }
protected:
    virtual void walkProgramNode(ProgramNode* node) {
        _out << _prefix << node->toString() << std::endl;

        push_space();
        for ( auto stmt : *node->body() ) {
            walk(stmt);
        }
        pop_space();
    }

    virtual void walkExpressionStatementNode(ExpressionStatementNode* node) {
        _out << _prefix << node->toString() << std::endl;
        push_space();
        walk(node->expression());
        pop_space();
    }

    virtual void walkIdentifierNode(IdentifierNode* node) {
        _out << _prefix << node->toString() << std::endl;
    }

    virtual void walkMapAccessNode(MapAccessNode* node) {
        _out << _prefix << node->toString() << std::endl;
        push_space();
        walk(node->path());
        pop_space();
    }

    virtual void walkEnumerableAccessNode(EnumerableAccessNode* node) {
        _out << _prefix << node->toString() << std::endl;
        push_space();
        walk(node->path());
        pop_space();
    }

    virtual void walkPrimitiveTypeNode(PrimitiveTypeNode* node) {
        _out << _prefix << node->toString() << std::endl;
    }

    virtual void walkEnumerableTypeNode(EnumerableTypeNode* node) {
        _out << _prefix << node->toString() << std::endl;
    }

    virtual void walkMapTypeNode(MapTypeNode* node) {
        _out << _prefix << node->toString() << std::endl;
    }

    virtual void walkBooleanLiteralExpressionNode(BooleanLiteralExpressionNode* node) {
        _out << _prefix << node->toString() << std::endl;
    }

    virtual void walkVariableDeclarationNode(VariableDeclarationNode* node) {
        _out << _prefix << node->toString() << std::endl;
        push_space();
        walk(node->typeNode());
        walk(node->value());
        pop_space();
    }

    virtual void walkCallExpressionNode(CallExpressionNode* node) {
        _out << _prefix << node->toString() << std::endl;
        push_space();
        for ( auto exp : *node->args() ) {
            walk(exp);
        }
        pop_space();
    }

    virtual void walkAndNode(AndNode* node) {
        _out << _prefix << node->toString() << std::endl;
        push_space();
        walk(node->left());
        walk(node->right());
        pop_space();
    }

    virtual void walkOrNode(OrNode* node) {
        _out << _prefix << node->toString() << std::endl;
        push_space();
        walk(node->left());
        walk(node->right());
        pop_space();
    }

    virtual void walkEqualsNode(EqualsNode* node) {
        _out << _prefix << node->toString() << std::endl;
        push_space();
        walk(node->left());
        walk(node->right());
        pop_space();
    }

    virtual void walkNotEqualsNode(NotEqualsNode* node) {
        _out << _prefix << node->toString() << std::endl;
        push_space();
        walk(node->left());
        walk(node->right());
        pop_space();
    }

    virtual void walkAddNode(AddNode* node) {
        _out << _prefix << node->toString() << std::endl;
        push_space();
        walk(node->left());
        walk(node->right());
        pop_space();
    }

    virtual void walkSubtractNode(SubtractNode* node) {
        _out << _prefix << node->toString() << std::endl;
        push_space();
        walk(node->left());
        walk(node->right());
        pop_space();
    }

    virtual void walkMultiplyNode(MultiplyNode* node) {
        _out << _prefix << node->toString() << std::endl;
        push_space();
        walk(node->left());
        walk(node->right());
        pop_space();
    }

    virtual void walkDivideNode(DivideNode* node) {
        _out << _prefix << node->toString() << std::endl;
        push_space();
        walk(node->left());
        walk(node->right());
        pop_space();
    }

    virtual void walkModulusNode(ModulusNode* node) {
        _out << _prefix << node->toString() << std::endl;
        push_space();
        walk(node->left());
        walk(node->right());
        pop_space();
    }

    virtual void walkPowerNode(PowerNode* node) {
        _out << _prefix << node->toString() << std::endl;
        push_space();
        walk(node->left());
        walk(node->right());
        pop_space();
    }

    virtual void walkConcatenateNode(ConcatenateNode* node) {
        _out << _prefix << node->toString() << std::endl;
        push_space();
        walk(node->left());
        walk(node->right());
        pop_space();
    }

    virtual void walkNegativeExpressionNode(NegativeExpressionNode* node) {
        _out << _prefix << node->toString() << std::endl;
        push_space();
        walk(node->exp());
        pop_space();
    }

    virtual void walkNotNode(NotNode* node) {
        _out << _prefix << node->toString() << std::endl;
        push_space();
        walk(node->exp());
        pop_space();
    }

    virtual void walkEnumerationLiteralExpressionNode(EnumerationLiteralExpressionNode* node) {
        _out << _prefix << node->toString() << std::endl;
        push_space();
        for ( auto exp : *node->actuals() ) {
            walk(exp);
        }
        pop_space();
    }

    virtual void walkEnumerationStatement(EnumerationStatement* node) {
        _out << _prefix << node->toString() << std::endl;
        push_space();
        for ( auto stmt : *node->body() ) {
            walk(stmt);
        }
        pop_space();
    }

    virtual void walkCapturedBlockStatementNode(CapturedBlockStatementNode* node) {
        _out << _prefix << node->toString() << std::endl;
        push_space();
        for ( auto stmt : *node->body() ) {
            walk(stmt);
        }
        pop_space();
    }

    virtual void walkWithStatement(WithStatement* node) {
        _out << _prefix << node->toString() << std::endl;
        push_space();
        for ( auto stmt : *node->body() ) {
            walk(stmt);
        }
        pop_space();
    }

    virtual void walkIfStatement(IfStatement* node) {
        _out << _prefix << node->toString() << std::endl;
        push_space();
        for ( auto stmt : *node->body() ) {
            walk(stmt);
        }
        pop_space();
    }

    virtual void walkWhileStatement(WhileStatement* node) {
        _out << _prefix << node->toString() << std::endl;
        push_space();
        for ( auto stmt : *node->body() ) {
            walk(stmt);
        }
        pop_space();
    }

    virtual void walkMapStatementNode(MapStatementNode* node) {
        _out << _prefix << node->toString() << std::endl;
        push_space();
        walk(node->value());
        pop_space();
    }

    virtual void walkMapNode(MapNode* node) {
        _out << _prefix << node->toString() << std::endl;
        push_space();
        for ( auto entry : *node->body() ) {
            walk(entry);
        }
        pop_space();
    }

    virtual void walkStringLiteralExpressionNode(StringLiteralExpressionNode* node) {
        _out << _prefix << node->toString() << std::endl;
    }

    virtual void walkNumberLiteralExpressionNode(NumberLiteralExpressionNode* node) {
        _out << _prefix << node->toString() << std::endl;
    }

    virtual void walkAssignExpressionNode(AssignExpressionNode* node) {
        _out << _prefix << node->toString() << std::endl;
        push_space();
        walk(node->value());
        pop_space();
    }

    virtual void walkUnitNode(UnitNode* node) {
        _out << _prefix << node->toString() << std::endl;
    }

    virtual void walkFunctionNode(FunctionNode* node) {
        _out << _prefix << node->toString() << std::endl;
        push_space();
        for ( auto stmt : *node->body() ) {
            walk(stmt);
        }
        pop_space();
    }

    virtual void walkNumericComparisonExpressionNode(NumericComparisonExpressionNode* node) {
        _out << _prefix << node->toString() << std::endl;
    }

    virtual void walkTagResourceNode(TagResourceNode* node) {
        _out << _prefix << node->toString() << std::endl;
    }

    virtual std::string toString() const {
        return "PrintWalk<>";
    }

private:
    std::ostream& _out;
    std::string _prefix;

    void push_space() {
        _prefix += SWARMC_SPACE;
    }

    void pop_space() {
        if (_prefix == "") throw Errors::SwarmError("Attempt to pop empty prefix");
        std::string temp = SWARMC_SPACE;
        _prefix = _prefix.substr(temp.size());
    }
};

}
}
}

#endif