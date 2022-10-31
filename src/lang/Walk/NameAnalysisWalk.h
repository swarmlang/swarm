#ifndef SWARMC_NAME_ANALYSIS_WALK_H
#define SWARMC_NAME_ANALYSIS_WALK_H

#include "Walk.h"
#include "../../Reporting.h"
#include "../SymbolTable.h"

namespace swarmc {
namespace Lang {
namespace Walk {

class NameAnalysisWalk : public Walk<bool> {
public:
    NameAnalysisWalk() : Walk<bool>(), _symbols(new SymbolTable()) {}
    ~NameAnalysisWalk() {
        delete _symbols;
    }
protected:
    virtual bool walkProgramNode(ProgramNode* node) {
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

    virtual bool walkExpressionStatementNode(ExpressionStatementNode* node) {
        return walk(node->expression());
    }

    virtual bool walkIdentifierNode(IdentifierNode* node) {
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

    virtual bool walkMapAccessNode(MapAccessNode* node) {
        return walk(node->path());
    }

    virtual bool walkEnumerableAccessNode(EnumerableAccessNode* node) {
        bool flag = walk(node->path());
        return walk(node->index()) && flag;
    }

    virtual bool walkTypeLiteral(swarmc::Lang::TypeLiteral *node) {
        return true;
    }

    virtual bool walkBooleanLiteralExpressionNode(BooleanLiteralExpressionNode* node) {
        return true;
    }

    virtual bool walkVariableDeclarationNode(VariableDeclarationNode* node) {
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

        bool valueResult = true;
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

    virtual bool walkCallExpressionNode(CallExpressionNode* node) {
        bool flag = walk(node->id());

        for ( auto arg : *node->args() ) {
            flag = walk(arg) && flag;
        }

        return flag;
    }

    virtual bool walkIIFExpressionNode(IIFExpressionNode* node) {
        bool flag = walk(node->expression());

        for ( auto arg : *node->args() ) {
            flag = walk(arg) && flag;
        }

        return flag;
    }

    virtual bool walkAndNode(AndNode* node) {
        bool leftResult = walk(node->left());
        bool rightResult = walk(node->right());
        return leftResult && rightResult;
    }

    virtual bool walkOrNode(OrNode* node) {
        bool leftResult = walk(node->left());
        bool rightResult = walk(node->right());
        return leftResult && rightResult;
    }

    virtual bool walkEqualsNode(EqualsNode* node) {
        bool leftResult = walk(node->left());
        bool rightResult = walk(node->right());
        return leftResult && rightResult;
    }

    virtual bool walkNotEqualsNode(NotEqualsNode* node) {
        bool leftResult = walk(node->left());
        bool rightResult = walk(node->right());
        return leftResult && rightResult;
    }

    virtual bool walkAddNode(AddNode* node) {
        bool leftResult = walk(node->left());
        bool rightResult = walk(node->right());
        return leftResult && rightResult;
    }

    virtual bool walkSubtractNode(SubtractNode* node) {
        bool leftResult = walk(node->left());
        bool rightResult = walk(node->right());
        return leftResult && rightResult;
    }

    virtual bool walkMultiplyNode(MultiplyNode* node) {
        bool leftResult = walk(node->left());
        bool rightResult = walk(node->right());
        return leftResult && rightResult;
    }

    virtual bool walkDivideNode(DivideNode* node) {
        bool leftResult = walk(node->left());
        bool rightResult = walk(node->right());
        return leftResult && rightResult;
    }

    virtual bool walkModulusNode(ModulusNode* node) {
        bool leftResult = walk(node->left());
        bool rightResult = walk(node->right());
        return leftResult && rightResult;
    }

    virtual bool walkPowerNode(PowerNode* node) {
        bool leftResult = walk(node->left());
        bool rightResult = walk(node->right());
        return leftResult && rightResult;
    }

    virtual bool walkConcatenateNode(ConcatenateNode* node) {
        bool leftResult = walk(node->left());
        bool rightResult = walk(node->right());
        return leftResult && rightResult;
    }

    virtual bool walkNegativeExpressionNode(NegativeExpressionNode* node) {
        return walk(node->exp());
    }

    virtual bool walkNotNode(NotNode* node) {
        return walk(node->exp());
    }

    virtual bool walkEnumerationLiteralExpressionNode(EnumerationLiteralExpressionNode* node) {
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

    virtual bool walkEnumerationStatement(EnumerationStatement* node) {
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
                type = enumGenericType->values()->copy();
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

    virtual bool walkCapturedBlockStatementNode(CapturedBlockStatementNode* node) {
        return walkBlockStatementNode(node);
    }

    virtual bool walkWithStatement(WithStatement* node) {
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

    virtual bool walkIfStatement(IfStatement* node) {
        bool flag = walk(node->condition());
        return walkBlockStatementNode(node) && flag;
    }

    virtual bool walkWhileStatement(WhileStatement* node) {
        bool flag = walk(node->condition());
        return walkBlockStatementNode(node) && flag;
    }

    virtual bool walkContinueNode(ContinueNode* node) {
        return true;
    }

    virtual bool walkBreakNode(BreakNode* node) {
        return true;
    }

    virtual bool walkReturnStatementNode(ReturnStatementNode* node) {
        if ( node->value() == nullptr ) {
            return true;
        }

        return walk(node->value());
    }

    virtual bool walkMapStatementNode(MapStatementNode* node) {
        return walk(node->value());
    }

    virtual bool walkMapNode(MapNode* node) {
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

    virtual bool walkStringLiteralExpressionNode(StringLiteralExpressionNode* node) {
        return true;
    }

    virtual bool walkNumberLiteralExpressionNode(NumberLiteralExpressionNode* node) {
        return true;
    }

    virtual bool walkAssignExpressionNode(AssignExpressionNode* node) {
        bool lvalResult = walk(node->dest());
        bool rvalResult = walk(node->value());
        return lvalResult && rvalResult;
    }

    virtual bool walkUnitNode(UnitNode* node) {
        return true;
    }

    virtual bool walkFunctionNode(FunctionNode* node) {
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

    virtual bool walkNumericComparisonExpressionNode(NumericComparisonExpressionNode* node) {
        bool leftResult = walk(node->left());
        bool rightResult = walk(node->right());
        return leftResult && rightResult;
    }

    virtual std::string toString() const {
        return "NameAnalysisWalk<>";
    }
private:
    SymbolTable* _symbols;
};

}
}
}

#endif