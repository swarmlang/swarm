#ifndef SWARMC_NAME_ANALYSIS_WALK_H
#define SWARMC_NAME_ANALYSIS_WALK_H

#include "Walk.h"
#include "../../Reporting.h"
#include "../SymbolTable.h"

namespace swarmc::Lang::Walk {

class NameAnalysisWalk : public Walk<bool> {
public:
    NameAnalysisWalk() : Walk<bool>(), _symbols(new SymbolTable()) {}
    ~NameAnalysisWalk() override {
        delete _symbols;
    }
protected:
    bool walkProgramNode(ProgramNode* node) override {
        bool flag = true;
        // Enter the global scope
        _symbols->enter();

        for ( auto stmt : *node->body() ) {
            flag = walk(stmt) && flag;
        }

        // Exit the global scope
        _symbols->leave();
        return flag;
    }

    bool walkExpressionStatementNode(ExpressionStatementNode* node) override {
        return walk(node->expression());
    }

    bool walkIdentifierNode(IdentifierNode* node) override {
        if ( node->_symbol != nullptr ) {
            return true;
        }

        std::string name = node->name();
        node->_symbol = _symbols->lookup(name);

        if ( node->_symbol == nullptr ) {
            Reporting::nameError(node->position(), "Use of free identifier \"" + name + "\".");
            return false;
        }

        return true;
    }

    bool walkMapAccessNode(MapAccessNode* node) override {
        return walk(node->path());
    }

    bool walkEnumerableAccessNode(EnumerableAccessNode* node) override {
        bool flag = walk(node->path());
        return walk(node->index()) && flag;
    }

    bool walkTypeLiteral(swarmc::Lang::TypeLiteral *node) override {
        return true;
    }

    bool walkBooleanLiteralExpressionNode(BooleanLiteralExpressionNode* node) override {
        return true;
    }

    bool walkVariableDeclarationNode(VariableDeclarationNode* node) override {
        if ( node->id()->symbol() != nullptr ) {
            return walk(node->value());
        }

        std::string name = node->id()->name();
        const Type::Type* type = node->typeNode()->value();

        // Make sure the name isn't already declared in this scope
        if ( _symbols->isClashing(name) ) {
            SemanticSymbol* existing = _symbols->lookup(name);
            Reporting::nameError(node->position(), "Redeclaration of identifier \"" + name + "\" first declared at " + existing->declaredAt()->start() + ".");
            return false;
        }

        bool valueResult;
        if ( node->value()->getName() == "FunctionNode" ) {
            // Add the declaration to the current scope
            _symbols->addVariable(name, type, node->position());

            // Check the RHS of the assignment
            valueResult = walk(node->value());
        } else {
            // Check the RHS of the assignment
            valueResult = walk(node->value());

            // Add the declaration to the current scope
            _symbols->addVariable(name, type, node->position());
        }

        // Call this to attach the Symbol to the IdentifierNode
        walk(node->id());
        return valueResult;
    }

    bool walkCallExpressionNode(CallExpressionNode* node) override {
        bool flag = walk(node->id());

        for ( auto arg : *node->args() ) {
            flag = walk(arg) && flag;
        }

        return flag;
    }

    bool walkIIFExpressionNode(IIFExpressionNode* node) override {
        bool flag = walk(node->expression());

        for ( auto arg : *node->args() ) {
            flag = walk(arg) && flag;
        }

        return flag;
    }

    bool walkAndNode(AndNode* node) override {
        bool leftResult = walk(node->left());
        bool rightResult = walk(node->right());
        return leftResult && rightResult;
    }

    bool walkOrNode(OrNode* node) override {
        bool leftResult = walk(node->left());
        bool rightResult = walk(node->right());
        return leftResult && rightResult;
    }

    bool walkEqualsNode(EqualsNode* node) override {
        bool leftResult = walk(node->left());
        bool rightResult = walk(node->right());
        return leftResult && rightResult;
    }

    bool walkNotEqualsNode(NotEqualsNode* node) override {
        bool leftResult = walk(node->left());
        bool rightResult = walk(node->right());
        return leftResult && rightResult;
    }

    bool walkAddNode(AddNode* node) override {
        bool leftResult = walk(node->left());
        bool rightResult = walk(node->right());
        return leftResult && rightResult;
    }

    bool walkSubtractNode(SubtractNode* node) override {
        bool leftResult = walk(node->left());
        bool rightResult = walk(node->right());
        return leftResult && rightResult;
    }

    bool walkMultiplyNode(MultiplyNode* node) override {
        bool leftResult = walk(node->left());
        bool rightResult = walk(node->right());
        return leftResult && rightResult;
    }

    bool walkDivideNode(DivideNode* node) override {
        bool leftResult = walk(node->left());
        bool rightResult = walk(node->right());
        return leftResult && rightResult;
    }

    bool walkModulusNode(ModulusNode* node) override {
        bool leftResult = walk(node->left());
        bool rightResult = walk(node->right());
        return leftResult && rightResult;
    }

    bool walkPowerNode(PowerNode* node) override {
        bool leftResult = walk(node->left());
        bool rightResult = walk(node->right());
        return leftResult && rightResult;
    }

    bool walkConcatenateNode(ConcatenateNode* node) override {
        bool leftResult = walk(node->left());
        bool rightResult = walk(node->right());
        return leftResult && rightResult;
    }

    bool walkNegativeExpressionNode(NegativeExpressionNode* node) override {
        return walk(node->exp());
    }

    bool walkNotNode(NotNode* node) override {
        return walk(node->exp());
    }

    bool walkEnumerationLiteralExpressionNode(EnumerationLiteralExpressionNode* node) override {
        bool flag = true;
        
        for ( auto actual : *node->actuals() ) {
            flag = walk(actual) && flag;
        }

        return flag;
    }

    virtual bool walkBlockStatementNode(BlockStatementNode* node) {
        bool flag = true;

        for ( auto stmt : *node->body() ) {
            flag = walk(stmt) && flag;
        }

        return flag;
    }

    bool walkEnumerationStatement(EnumerationStatement* node) override {
        bool flag = walk(node->enumerable());

        // Need to register the block-local variable
        // Its type is implicit as the generic type of the enumerable
        bool inScope = false;
        if ( node->local()->symbol() == nullptr ) {
            std::string name = node->local()->name();
            Position* pos = node->local()->position();
            const Type::Type* type = nullptr;

            // Try to look up the generic type of the enumerable
            const Type::Type* enumType = node->enumerable()->type();
            if ( enumType->intrinsic() == Type::Intrinsic::ENUMERABLE ) {
                auto enumGenericType = (Type::Enumerable*) enumType;
                type = enumGenericType->values()->copy(node->_shared);
            }

            // Start a new scope in the body and add the local
            _symbols->enter();
            inScope = true;
            _symbols->addVariable(name, type, pos);
        }

        flag = walk(node->local()) && flag;
        flag = walkBlockStatementNode(node) && flag;
        if ( inScope ) _symbols->leave();
        return flag;
    }

    bool walkWithStatement(WithStatement* node) override {
        bool flag = walk(node->resource());

        // need to register the block-local variable
        // Its type is implicit as the result of the expression
        bool inScope = false;
        if ( node->local()->symbol() == nullptr ) {
            std::string name = node->local()->name();
            Position* pos = node->local()->position();
            Type::Type* type = nullptr;

            // Note that the type of the local depends on the type of the expression,
            // since it is implicitly defined. This is handled in typeAnalysis.

            // Start a new scope in the body and add the local
            _symbols->enter();
            inScope = true;
            _symbols->addVariable(name, type, pos);
        }

        flag = walk(node->local()) && flag;
        flag = walkBlockStatementNode(node) && flag;
        if ( inScope ) _symbols->leave();
        return flag;
    }

    bool walkIfStatement(IfStatement* node) override {
        bool flag = walk(node->condition());
        return walkBlockStatementNode(node) && flag;
    }

    bool walkWhileStatement(WhileStatement* node) override {
        bool flag = walk(node->condition());
        return walkBlockStatementNode(node) && flag;
    }

    bool walkContinueNode(ContinueNode* node) override {
        return true;
    }

    bool walkBreakNode(BreakNode* node) override {
        return true;
    }

    bool walkReturnStatementNode(ReturnStatementNode* node) override {
        if ( node->value() == nullptr ) {
            return true;
        }

        return walk(node->value());
    }

    bool walkMapStatementNode(MapStatementNode* node) override {
        return walk(node->value());
    }

    bool walkMapNode(MapNode* node) override {
        bool flag = true;
        // Check each entry in the map
        for ( auto entry : *node->body() ) {
            // Check for duplicate name
            size_t nameCount = 0;
            for ( auto subentry : *node->body() ) {
                if ( subentry->id()->name() == entry->id()->name() && nameCount < 2) {
                    nameCount += 1;
                }
            }

            if ( nameCount > 1 ) {
                Reporting::nameError(
                    entry->position(),
                    "Duplicate map key: \"" + entry->id()->name() + "\""
                );

                flag = false;
            }

            // Check the value expression
            flag = walk(entry) && flag;
        }

        return flag;
    }

    bool walkStringLiteralExpressionNode(StringLiteralExpressionNode* node) override {
        return true;
    }

    bool walkNumberLiteralExpressionNode(NumberLiteralExpressionNode* node) override {
        return true;
    }

    bool walkAssignExpressionNode(AssignExpressionNode* node) override {
        bool lvalResult = walk(node->dest());
        bool rvalResult = walk(node->value());
        return lvalResult && rvalResult;
    }

    bool walkUnitNode(UnitNode* node) override {
        return true;
    }

    bool walkFunctionNode(FunctionNode* node) override {
        bool flag = true;
        _symbols->enter();
        for ( auto formal : *node->formals() ) {
            std::string name = formal.second->name();
            const Type::Type* type = formal.first->value();

            // Make sure the name isn't already declared in this scope
            if ( _symbols->isClashing(name) ) {
                SemanticSymbol* existing = _symbols->lookup(name);
                Reporting::nameError(node->position(), "Redeclaration of identifier \"" + name + "\" first declared at " + existing->declaredAt()->start() + ".");
                flag = false;
            }

            // Add the declaration to the current scope
            _symbols->addVariable(name, type, formal.second->position());

            // Call this to attach the Symbol to the IdentifierNode
            walk(formal.second);
        }

        for ( auto stmt : *node->body() ) {
            flag = walk(stmt) && flag;
        }

        _symbols->leave();

        return flag;
    }

    bool walkNumericComparisonExpressionNode(NumericComparisonExpressionNode* node) override {
        bool leftResult = walk(node->left());
        bool rightResult = walk(node->right());
        return leftResult && rightResult;
    }

    [[nodiscard]] std::string toString() const override {
        return "NameAnalysisWalk<>";
    }
private:
    SymbolTable* _symbols;
};

}

#endif