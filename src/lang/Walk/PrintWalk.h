#ifndef SWARMC_PRINT_WALK_H
#define SWARMC_PRINT_WALK_H

#ifndef SWARMC_SPACE
#define SWARMC_SPACE "  "
#endif

#include <fstream>
#include "Walk.h"

namespace swarmc::Lang::Walk {

class PrintWalk : Walk<void> {
public:
    static void print(std::ostream& out, ASTNode* node) {
        PrintWalk p(out);
        p.walk(node);
    }

    explicit PrintWalk(std::ostream& out) : Walk<void>("Print Walk"), _out(out) {}
protected:
    void walkProgramNode(ProgramNode* node) override {
        _out << _prefix << node->toString() << std::endl;

        push_space();
        for ( auto stmt : *node->body() ) {
            walk(stmt);
        }
        pop_space();
    }

    void walkExpressionStatementNode(ExpressionStatementNode* node) override {
        _out << _prefix << node->toString() << std::endl;
        push_space();
        walk(node->expression());
        pop_space();
    }

    void walkIdentifierNode(IdentifierNode* node) override {
        _out << _prefix << node->toString() << std::endl;
    }

    void walkEnumerableAccessNode(EnumerableAccessNode* node) override {
        _out << _prefix << node->toString() << std::endl;
        push_space();
        walk(node->path());
        walk(node->index());
        pop_space();
    }

    void walkEnumerableAppendNode(EnumerableAppendNode* node) override {
        _out << _prefix << node->toString() << std::endl;
        push_space();
        walk(node->path());
        pop_space();
    }

    void walkMapAccessNode(MapAccessNode* node) override {
        _out << _prefix << node->toString() << std::endl;
        push_space();
        walk(node->path());
        pop_space();
    }

    void walkClassAccessNode(ClassAccessNode* node) override {
        _out << _prefix << node->toString() << std::endl;
        push_space();
        walk(node->path());
        pop_space();
    }

    void walkIncludeStatementNode(IncludeStatementNode* node) override {
        _out << _prefix << node->toString() << std::endl;
    }

    void walkTypeLiteral(swarmc::Lang::TypeLiteral *node) override {
        _out << _prefix << node->toString() << std::endl;
    }

    void walkBooleanLiteralExpressionNode(BooleanLiteralExpressionNode* node) override {
        _out << _prefix << node->toString() << std::endl;
    }

    void walkStringLiteralExpressionNode(StringLiteralExpressionNode* node) override {
        _out << _prefix << node->toString() << std::endl;
    }

    void walkNumberLiteralExpressionNode(NumberLiteralExpressionNode* node) override {
        _out << _prefix << node->toString() << std::endl;
    }

    void walkEnumerationLiteralExpressionNode(EnumerationLiteralExpressionNode* node) override {
        _out << _prefix << node->toString() << std::endl;
        push_space();
        for ( auto exp : *node->actuals() ) {
            walk(exp);
        }
        pop_space();
    }

    void walkMapStatementNode(MapStatementNode* node) override {
        _out << _prefix << node->toString() << std::endl;
        push_space();
        walk(node->value());
        pop_space();
    }

    void walkMapNode(MapNode* node) override {
        _out << _prefix << node->toString() << std::endl;
        push_space();
        for ( auto entry : *node->body() ) {
            walk(entry);
        }
        pop_space();
    }

    void walkAssignExpressionNode(AssignExpressionNode* node) override {
        _out << _prefix << node->toString() << std::endl;
        push_space();
        walk(node->value());
        pop_space();
    }

    void walkVariableDeclarationNode(VariableDeclarationNode* node) override {
        _out << _prefix << node->toString() << std::endl;
        push_space();
        walk(node->typeNode());
        walk(node->id());
        walk(node->value());
        pop_space();
    }

    void walkUninitializedVariableDeclarationNode(UninitializedVariableDeclarationNode* node) override {
        _out << _prefix << node->toString() << std::endl;
        push_space();
        walk(node->typeNode());
        pop_space();
    }

    void walkUseNode(UseNode* node) override {
        _out << _prefix << node->toString() << std::endl;
    }

    void walkReturnStatementNode(ReturnStatementNode* node) override {
        _out << _prefix << node->toString() << std::endl;
        if ( node->value() != nullptr ) {
            push_space();
            walk(node->value());
            pop_space();
        }
    }

    void walkFunctionNode(FunctionNode* node) override {
        _out << _prefix << node->toString() << std::endl;
        push_space();
        for ( auto stmt : *node->body() ) {
            walk(stmt);
        }
        pop_space();
    }

    void walkConstructorNode(ConstructorNode* node) override {
        _out << _prefix << node->toString() << std::endl;
        push_space();
        for ( auto pc : *node->parentConstructors() ) walk(pc);
        walk(node->func());
        pop_space();
    }

    void walkTypeBodyNode(TypeBodyNode* node) override {
        _out << _prefix << node->toString() << std::endl;
        push_space();
        for ( auto decl : *node->parents() ) {
            walk(decl);
        }
        for ( auto decl : *node->declarations() ) {
            walk(decl);
        }
        for ( auto cons : *node->constructors() ) {
            walk(cons);
        }
        pop_space();
    }

    void walkCallExpressionNode(CallExpressionNode* node) override {
        _out << _prefix << node->toString() << std::endl;
        push_space();
        walk(node->func());
        for ( auto exp : *node->args() ) {
            walk(exp);
        }
        pop_space();
    }

    void walkDeferCallExpressionNode(DeferCallExpressionNode* node) override {
        _out << _prefix << node->toString() << std::endl;
        push_space();
        walk(node->call());
        pop_space();
    }

    void walkAndNode(AndNode* node) override {
        _out << _prefix << node->toString() << std::endl;
        push_space();
        walk(node->left());
        walk(node->right());
        pop_space();
    }

    void walkOrNode(OrNode* node) override {
        _out << _prefix << node->toString() << std::endl;
        push_space();
        walk(node->left());
        walk(node->right());
        pop_space();
    }

    void walkEqualsNode(EqualsNode* node) override {
        _out << _prefix << node->toString() << std::endl;
        push_space();
        walk(node->left());
        walk(node->right());
        pop_space();
    }

    void walkNumericComparisonExpressionNode(NumericComparisonExpressionNode* node) override {
        _out << _prefix << node->toString() << std::endl;
    }

    void walkNotEqualsNode(NotEqualsNode* node) override {
        _out << _prefix << node->toString() << std::endl;
        push_space();
        walk(node->left());
        walk(node->right());
        pop_space();
    }

    void walkAddNode(AddNode* node) override {
        _out << _prefix << node->toString() << std::endl;
        push_space();
        walk(node->left());
        walk(node->right());
        pop_space();
    }

    void walkSubtractNode(SubtractNode* node) override {
        _out << _prefix << node->toString() << std::endl;
        push_space();
        walk(node->left());
        walk(node->right());
        pop_space();
    }

    void walkMultiplyNode(MultiplyNode* node) override {
        _out << _prefix << node->toString() << std::endl;
        push_space();
        walk(node->left());
        walk(node->right());
        pop_space();
    }

    void walkDivideNode(DivideNode* node) override {
        _out << _prefix << node->toString() << std::endl;
        push_space();
        walk(node->left());
        walk(node->right());
        pop_space();
    }

    void walkModulusNode(ModulusNode* node) override {
        _out << _prefix << node->toString() << std::endl;
        push_space();
        walk(node->left());
        walk(node->right());
        pop_space();
    }

    void walkPowerNode(PowerNode* node) override {
        _out << _prefix << node->toString() << std::endl;
        push_space();
        walk(node->left());
        walk(node->right());
        pop_space();
    }

    void walkNegativeExpressionNode(NegativeExpressionNode* node) override {
        _out << _prefix << node->toString() << std::endl;
        push_space();
        walk(node->exp());
        pop_space();
    }

    void walkSqrtNode(SqrtNode* node) override {
        _out << _prefix << node->toString() << std::endl;
        push_space();
        walk(node->exp());
        pop_space();
    }

    void walkNotNode(NotNode* node) override {
        _out << _prefix << node->toString() << std::endl;
        push_space();
        walk(node->exp());
        pop_space();
    }

    void walkEnumerationStatement(EnumerationStatement* node) override {
        _out << _prefix << node->toString() << std::endl;
        push_space();
        for ( auto stmt : *node->body() ) {
            walk(stmt);
        }
        pop_space();
    }

    void walkWithStatement(WithStatement* node) override {
        _out << _prefix << node->toString() << std::endl;
        push_space();
        for ( auto stmt : *node->body() ) {
            walk(stmt);
        }
        pop_space();
    }

    void walkIfStatement(IfStatement* node) override {
        _out << _prefix << node->toString() << std::endl;
        push_space();
        for ( auto stmt : *node->body() ) {
            walk(stmt);
        }
        pop_space();
    }

    void walkWhileStatement(WhileStatement* node) override {
        _out << _prefix << node->toString() << std::endl;
        push_space();
        for ( auto stmt : *node->body() ) {
            walk(stmt);
        }
        pop_space();
    }

    void walkContinueNode(ContinueNode* node) override {
        _out << _prefix << node->toString() << std::endl;
    }

    void walkBreakNode(BreakNode* node) override {
        _out << _prefix << node->toString() << std::endl;
    }
    [[nodiscard]] std::string toString() const override {
        return "PrintWalk<>";
    }

private:
    std::ostream& _out;
    std::string _prefix;

    void push_space() {
        _prefix += SWARMC_SPACE;
    }

    void pop_space() {
        if (_prefix.empty()) throw Errors::SwarmError("Attempt to pop empty prefix");
        std::string temp = SWARMC_SPACE;
        _prefix = _prefix.substr(temp.size());
    }
};

}

#endif
