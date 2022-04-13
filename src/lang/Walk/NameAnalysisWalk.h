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
        // Enter the global scope
        _symbols->enter();

        for ( auto stmt : *node->body() ) {
            if ( !walk(stmt) ) {
                _symbols->leave();
                return false;
            }
        }

        // Exit the global scope
        _symbols->leave();
        return true;
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
        return walk(node->path());
    }

    virtual bool walkPrimitiveTypeNode(PrimitiveTypeNode* node) {
        return true;
    }

    virtual bool walkEnumerableTypeNode(EnumerableTypeNode* node) {
        return true;
    }

    virtual bool walkMapTypeNode(MapTypeNode* node) {
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
        Type* type = node->typeNode()->type();

        // Make sure the name isn't already declared in this scope
        if ( _symbols->isClashing(name) ) {
            SemanticSymbol* existing = _symbols->lookup(name);
            Reporting::nameError(node->position(), "Redeclaration of identifier \"" + name + "\" first declared at " + existing->declaredAt()->start() + ".");
            return false;
        }

        // Check the RHS of the assignment
        bool valueResult = walk(node->value());

        // Add the declaration to the current scope
        _symbols->addVariable(name, type, node->position());

        // Call this to attach the Symbol to the IdentifierNode
        walk(node->id());
        return valueResult;
    }

    virtual bool walkCallExpressionNode(CallExpressionNode* node) {
        if ( !walk(node->id()) ) {
            return false;
        }

        for ( auto arg : *node->args() ) {
            if ( !walk(arg) ) {
                return false;
            }
        }

        return true;
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

    virtual bool walkAddAssignExpressionNode(AddAssignExpressionNode* node) {
        if ( !walk(node->value()) ) return false;
        return walkAssignExpressionNode(node);
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

    virtual bool walkMultiplyAssignExpressionNode(MultiplyAssignExpressionNode* node) {
        if ( !walk(node->value()) ) return false;
        return walkAssignExpressionNode(node);
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
        for ( auto actual : *node->actuals() ) {
            if ( !walk(actual) ) {
                return false;
            }
        }

        return true;
    }

    virtual bool walkBlockStatementNode(BlockStatementNode* node) {
        for ( auto stmt : *node->body() ) {
            if ( !walk(stmt) ) {
                return false;
            }
        }

        return true;
    }

    virtual bool walkEnumerationStatement(EnumerationStatement* node) {
        if ( !walk(node->enumerable()) ) {
            return false;
        }

        // Need to register the block-local variable
        // Its type is implicit as the generic type of the enumerable
        bool inScope = false;
        if ( node->local()->symbol() == nullptr ) {
            std::string name = node->local()->name();
            Position* pos = node->local()->position();
            Type* type = nullptr;

            // Try to look up the generic type of the enumerable
            const Type* enumType = node->enumerable()->symbol()->type();
            if ( enumType->kind() == TypeKind::KGENERIC ) {
                auto enumGenericType = (GenericType*) enumType;
                type = enumGenericType->concrete();

                if ( type->isPrimitiveType() ) {
                    type = PrimitiveType::of(type->valueType(), node->_shared);
                } else if ( type->isGenericType() ) {
                    type = type->copy();
                    type->setShared(node->_shared);
                } else if ( type->isFunctionType() ) {
                    type = type->copy();
                    type->setShared(node->_shared);
                }
            }

            // Start a new scope in the body and add the local
            _symbols->enter();
            inScope = true;
            _symbols->addVariable(name, type, pos);

//            if ( !walk(node->local()) ) {
//                _symbols->leave();
//                return false;
//            }
        }

        if ( !walk(node->local()) ) {  // FIXME did this need to be moved?
            if ( inScope ) _symbols->leave();
            return false;
        }

        bool bodyResult = walkBlockStatementNode(node);
        if ( inScope ) _symbols->leave();
        return bodyResult;
    }

    virtual bool walkCapturedBlockStatementNode(CapturedBlockStatementNode* node) {
        return walkBlockStatementNode(node);
    }

    virtual bool walkWithStatement(WithStatement* node) {
        if ( !walk(node->resource()) ) {
            return false;
        }

        // need to register the block-local variable
        // Its type is implicit as the result of the expression
        bool inScope = false;
        if ( node->local()->symbol() == nullptr ) {
            std::string name = node->local()->name();
            Position* pos = node->local()->position();
            Type* type = nullptr;

            // Note that the type of the local depends on the type of the expression,
            // since it is implicitly defined. This is handled in typeAnalysis.

            // Start a new scope in the body and add the local
            _symbols->enter();
            inScope = true;
            _symbols->addVariable(name, type, pos);

//            if ( !walk(node->local()) ) {
//                _symbols->leave();
//                return false;
//            }
        }

        if ( !walk(node->local()) ) {  // FIXME did this need to be moved?
            if ( inScope ) _symbols->leave();
            return false;
        }

        bool bodyResult = walkBlockStatementNode(node);
        if ( inScope ) _symbols->leave();
        return bodyResult;
    }

    virtual bool walkIfStatement(IfStatement* node) {
        return walk(node->condition()) && walkBlockStatementNode(node);
    }

    virtual bool walkWhileStatement(WhileStatement* node) {
        return walk(node->condition()) && walkBlockStatementNode(node);
    }

    virtual bool walkMapStatementNode(MapStatementNode* node) {
        return walk(node->value());
    }

    virtual bool walkMapNode(MapNode* node) {
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

                return false;
            }

            // Check the value expression
            if ( !walk(entry) ) {
                return false;
            }
        }

        return true;
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

    virtual bool walkNumericComparisonExpressionNode(NumericComparisonExpressionNode* node) {
        bool leftResult = walk(node->left());
        bool rightResult = walk(node->right());
        return leftResult && rightResult;
    }

    virtual bool walkTagResourceNode(TagResourceNode* node) {
        return true;
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