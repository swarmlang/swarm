#ifndef SWARMC_TYPE_ANALYSIS_WALK_H
#define SWARMC_TYPE_ANALYSIS_WALK_H

#include "Walk.h"
#include "../TypeTable.h"
#include "../../Reporting.h"

namespace swarmc {
namespace Lang {
namespace Walk {

class TypeAnalysisWalk : public Walk<bool> {
public:
    TypeAnalysisWalk() : Walk<bool>(), _types(new TypeTable()) {}
    ~TypeAnalysisWalk() {
        delete _types;
    }
protected:
    virtual bool walkProgramNode(ProgramNode* node) {
        for ( auto stmt : *node->body() ) {
            if ( !walk(stmt) ) {
                return false;
            }
        }

        _types->setTypeOf(node, PrimitiveType::of(ValueType::TUNIT, false));
        return true;
    }

    virtual bool walkExpressionStatementNode(ExpressionStatementNode* node) {
        bool expResult = walk(node->expression());

        if ( expResult ) {
            _types->setTypeOf(node, PrimitiveType::of(ValueType::TUNIT, false));
        }

        return expResult;
    }

    virtual bool walkIdentifierNode(IdentifierNode* node) {
        if ( node->_symbol->type() == nullptr ) {
            Reporting::typeError(
                node->position(),
                "Invalid type of free identifier: " + node->name()
            );

            return false;
        }

        _types->setTypeOf(node, node->_symbol->type());
        return true;
    }

    virtual bool walkMapAccessNode(MapAccessNode* node) {
        bool pathresult = walk(node->path());
        if ( !pathresult ) {
            return false;
        }

        const Type* typeLVal = _types->getTypeOf(node->path());
        if ( !typeLVal->isGenericType() ) {
            Reporting::typeError(
                node->position(),
                "Invalid map access: " + node->end()->name()
            );
            return false;
        }

        _types->setTypeOf(node, ((GenericType*) typeLVal)->concrete());
        return true;
    }

    virtual bool walkEnumerableAccessNode(EnumerableAccessNode* node) {
        bool pathresult = walk(node->path());
        if ( !pathresult ) {
            return false;
        }

        const Type* typeLVal = _types->getTypeOf(node->path());
        if ( !typeLVal->isGenericType() ) {
            Reporting::typeError(
                node->position(),
                "Invalid array access: " + std::to_string(node->index()->value())
            );
            return false;
        }

        _types->setTypeOf(node, ((GenericType*) typeLVal)->concrete());
        return true;
    }

    virtual bool walkPrimitiveTypeNode(PrimitiveTypeNode* node) {
        _types->setTypeOf(node, PrimitiveType::of(ValueType::TUNIT, false));
        return true;
    }

    virtual bool walkEnumerableTypeNode(EnumerableTypeNode* node) {
        _types->setTypeOf(node, PrimitiveType::of(ValueType::TUNIT, false));
        return true;
    }

    virtual bool walkMapTypeNode(MapTypeNode* node) {
        _types->setTypeOf(node, PrimitiveType::of(ValueType::TUNIT, false));
        return true;
    }

    virtual bool walkBooleanLiteralExpressionNode(BooleanLiteralExpressionNode* node) {
        _types->setTypeOf(node, PrimitiveType::of(ValueType::TBOOL, false));
        return true;
    }

    virtual bool walkVariableDeclarationNode(VariableDeclarationNode* node) {
        if (
            !walk(node->typeNode())
            || !walk(node->id())
            || !walk(node->value())
        ) {
            return false;
        }

        const Type* typeOfValue = _types->getTypeOf(node->value());
        if ( !node->typeNode()->type()->is(typeOfValue) ) {
            Reporting::typeError(
                node->position(),
                "Attempted to initialize identifier of type " + node->typeNode()->type()->toString() + " with value of type " + typeOfValue->toString() + "."
            );

            return false;
        }

        _types->setTypeOf(node, PrimitiveType::of(ValueType::TUNIT, false));
        return true;
    }

    virtual bool walkCallExpressionNode(CallExpressionNode* node) {
        // Perform type analysis on the callable
        if ( !walk(node->id()) ) {
            return false;
        }

        // Perform type analysis on the arguments
        for ( auto arg : *node->args() ) {
            if ( !walk(arg) ) {
                return false;
            }
        }

        // Make sure the callee is actually a function type
        const Type* baseTypeOfCallee = _types->getTypeOf(node->id());
        if ( baseTypeOfCallee->kind() != TypeKind::KFUNCTION ) {
            Reporting::typeError(
                node->position(),
                "Attempted to call non-callable type " + baseTypeOfCallee->toString() + "."
            );
        }

        const FunctionType* typeOfCallee = (FunctionType*) baseTypeOfCallee;
        std::vector<Type*>* argTypes = typeOfCallee->getArgumentTypes();

        // Make sure the # of arguments matches
        if ( argTypes->size() != node->args()->size() ) {
            Reporting::typeError(
                node->position(),
                "Invalid number of arguments for call (expected: " + std::to_string(argTypes->size()) + ")."
            );

            return false;
        }

        // Make sure the type of each argument matches
        for ( size_t i = 0; i < argTypes->size(); i += 1 ) {
            const Type* expectedType = argTypes->at(i);
            const Type* actualType = _types->getTypeOf(node->args()->at(i));

            if ( !actualType->is(expectedType) ) {
                Reporting::typeError(
                    node->position(),
                    "Invalid argument of type " + actualType->toString() + " in position " + std::to_string(i) + " (expected: " + expectedType->toString() + ")."
                );

                return false;
            }
        }

        _types->setTypeOf(node, typeOfCallee->returnType());
        return true;
    }

    virtual bool walkPureBinaryExpression(PureBinaryExpressionNode* node) {
        if (
            !walk(node->left())
            || !walk(node->right())
        ) {
            return false;
        }

        const Type* actualLeftType = _types->getTypeOf(node->left());
        const Type* actualRightType = _types->getTypeOf(node->right());

        if ( !node->leftType()->is(actualLeftType) ) {
            Reporting::typeError(
                node->position(),
                "Invalid type " + actualLeftType->toString() + " of left-hand operand to expression (expected: " + node->leftType()->toString() + ")."
            );

            return false;
        }

        if ( !node->rightType()->is(actualRightType) ) {
            Reporting::typeError(
                node->position(),
                "Invalid type " + actualRightType->toString() + " of right-hand operand to expression (expected: " + node->rightType()->toString() + ")."
            );

            return false;
        }

        _types->setTypeOf(node, node->resultType());
        return true;
    }

    virtual bool walkAndNode(AndNode* node) {
        return walkPureBinaryExpression(node);
    }

    virtual bool walkOrNode(OrNode* node) {
        return walkPureBinaryExpression(node);
    }

    virtual bool walkEqualsNode(EqualsNode* node) {
        if (
            !walk(node->left())
            || !walk(node->right())
        ) {
            return false;
        }

        const Type* actualLeftType = _types->getTypeOf(node->left());
        const Type* actualRightType = _types->getTypeOf(node->right());
        if ( !actualLeftType->is(actualRightType) ) {
            Reporting::typeError(
                node->position(),
                "Invalid comparison between left-hand type " + actualLeftType->toString() + " and right-hand type " + actualRightType->toString() + "."
            );

            return false;
        }

        _types->setTypeOf(node, PrimitiveType::of(ValueType::TBOOL, false));
        return true;
    }

    virtual bool walkNotEqualsNode(NotEqualsNode* node) {
        if (
            !walk(node->left())
            || !walk(node->right())
        ) {
            return false;
        }

        const Type* actualLeftType = _types->getTypeOf(node->left());
        const Type* actualRightType = _types->getTypeOf(node->right());
        if ( !actualLeftType->is(actualRightType) ) {
            Reporting::typeError(
                node->position(),
                "Invalid comparison between left-hand type " + actualLeftType->toString() + " and right-hand type " + actualRightType->toString() + "."
            );

            return false;
        }

        _types->setTypeOf(node, PrimitiveType::of(ValueType::TBOOL, false));
        return true;
    }

    virtual bool walkAddNode(AddNode* node) {
        return walkPureBinaryExpression(node);
    }

    virtual bool walkSubtractNode(SubtractNode* node) {
        return walkPureBinaryExpression(node);
    }

    virtual bool walkMultiplyNode(MultiplyNode* node) {
        return walkPureBinaryExpression(node);
    }

    virtual bool walkDivideNode(DivideNode* node) {
        return walkPureBinaryExpression(node);
    }

    virtual bool walkModulusNode(ModulusNode* node) {
        return walkPureBinaryExpression(node);
    }

    virtual bool walkPowerNode(PowerNode* node) {
        return walkPureBinaryExpression(node);
    }

    virtual bool walkConcatenateNode(ConcatenateNode* node) {
        return walkPureBinaryExpression(node);
    }

    virtual bool walkNegativeExpressionNode(NegativeExpressionNode* node) {
        if ( !walk(node->exp()) ) {
            return false;
        }

        const Type* numType = PrimitiveType::of(ValueType::TNUM);
        const Type* expType = _types->getTypeOf(node->exp());
        if ( !expType->is(numType) ) {
            Reporting::typeError(
                node->position(),
                "Attempted to perform numeric negation on invalid type. Expected: " + numType->toString() + "; actual: " + expType->toString()
            );
            return false;
        }

        _types->setTypeOf(node, numType);
        return true;
    }

    virtual bool walkNotNode(NotNode* node) {
        if ( !walk(node->exp()) ) {
            return false;
        }

        const Type* boolType = PrimitiveType::of(ValueType::TBOOL);
        const Type* expType = _types->getTypeOf(node->exp());
        if ( !expType->is(boolType) ) {
            Reporting::typeError(
                node->position(),
                "Attempted to perform boolean negation on invalid type. Expected: " + boolType->toString() + "; actual: " + expType->toString()
            );
            return false;
        }

        _types->setTypeOf(node, boolType);
        return true;
    }

    virtual bool walkEnumerationLiteralExpressionNode(EnumerationLiteralExpressionNode* node) {
        size_t idx = 0;
        bool hadFirst = false;
        const Type* innerType = nullptr;

        if ( node->_disambiguationType != nullptr ) {
            innerType = node->_disambiguationType->type();
            hadFirst = true;
        }

        for ( auto exp : *node->actuals() ) {
            if ( !walk(exp) ) {
                return false;
            }

            const Type* expType = _types->getTypeOf(exp);
            if ( !hadFirst ) {
                innerType = expType;
            } else if ( !expType->is(innerType) ) {
                Reporting::typeError(
                    node->position(),
                    "Invalid entry in enumerable at position " + std::to_string(idx) + ". Expected: " + innerType->toString() + "; Found: " + expType->toString()
                );
                return false;
            }

            idx += 1;
        }

        GenericType* type = GenericType::of(ValueType::TENUMERABLE, (Type*) innerType);
        _types->setTypeOf(node, type);
        return true;
    }

    virtual bool walkCapturedBlockStatementNode(CapturedBlockStatementNode* node) {
        for ( auto stmt : *node->body() ) {
            if ( !walk(stmt) ) {
                return false;
            }
        }

        _types->setTypeOf(node, PrimitiveType::of(ValueType::TUNIT, false));
        return true;
    }

    virtual bool walkEnumerationStatement(EnumerationStatement* node) {
        if ( !walk(node->enumerable()) ) {
            return false;
        }

        const Type* enumType = node->enumerable()->symbol()->type();
        if ( !enumType->isGenericType() ) {
            Reporting::typeError(
                node->position(),
                "Attempted to enumerate invalid value"
            );
        }

        GenericType* genericType = (GenericType*) enumType;
        Type* concreteType = genericType->concrete();
        _types->setTypeOf(node->local(), concreteType);

        for ( auto stmt : *node->body() ) {
            if ( !walk(stmt) ) {
                return false;
            }
        }

        _types->setTypeOf(node, PrimitiveType::of(ValueType::TUNIT, false));
        return true;
    }

    virtual bool walkWithStatement(WithStatement* node) {
        if ( !walk(node->resource()) ) {
            return false;
        }

        const Type* type = _types->getTypeOf(node->resource());
        if ( !type->isGenericType() ) console->debug("type is not generic");
        if ( type == nullptr || !type->isGenericType() || type->valueType() != ValueType::TRESOURCE ) {
            Reporting::typeError(
                node->position(),
                "Expected ValueType::TRESOURCE, found: " + (type == nullptr ? "none" : type->toString())
            );

            return false;
        }

        Type* localType = ((GenericType*) type)->concrete();

        if ( localType->isPrimitiveType() ) {
            localType = PrimitiveType::of(localType->valueType(), node->_shared);
        } else if ( localType->isGenericType() ) {
            localType = localType->copy();
            localType->setShared(node->_shared);
        } else if ( localType->isFunctionType() ) {
            localType = localType->copy();
            localType->setShared(node->_shared);
        }

        _types->setTypeOf(node->local(), localType);
        node->local()->symbol()->_type = localType;  // local is implicitly defined, so need to set its type

        for ( auto stmt : *node->body() ) {
            if ( !walk(stmt) ) {
                return false;
            }
        }

        _types->setTypeOf(node, PrimitiveType::of(ValueType::TUNIT, false));
        return true;
    }

    virtual bool walkIfStatement(IfStatement* node) {
        if (!walk(node->condition())) {
            return false;
        }

        const Type* condType = _types->getTypeOf(node->condition());
        if ( !condType->is(PrimitiveType::of(ValueType::TBOOL, false))) {
            Reporting::typeError(
                node->position(),
                "Condition for if statement not boolean " + condType->toString() + "."
            );

            return false;
        }

        for ( auto stmt : *node->body() ) {
            if ( !walk(stmt) ) {
                return false;
            }
        }

        _types->setTypeOf(node, PrimitiveType::of(ValueType::TUNIT, false));
        return true;
    }

    virtual bool walkWhileStatement(WhileStatement* node) {
        if (!walk(node->condition())) {
            return false;
        }

        const Type* condType = _types->getTypeOf(node->condition());
        if ( !condType->is(PrimitiveType::of(ValueType::TBOOL))) {
            Reporting::typeError(
                node->position(),
                "Condition for while loop not boolean " + condType->toString() + "."
            );

            return false;
        }

        for ( auto stmt : *node->body() ) {
            if ( !walk(stmt) ) {
                return false;
            }
        }

        _types->setTypeOf(node, PrimitiveType::of(ValueType::TUNIT, false));
        return true;
    }

    virtual bool walkMapStatementNode(MapStatementNode* node) {
        if ( !walk(node->value()) ) {
            return false;
        }

        _types->setTypeOf(node, _types->getTypeOf(node->value()));
        return true;
    }

    virtual bool walkMapNode(MapNode* node) {
        size_t idx = 0;
        bool hadFirst = false;
        const Type* innerType = nullptr;

        if ( node->_disambiguationType != nullptr ) {
            innerType = node->_disambiguationType->type();
            hadFirst = true;
        }

        for ( auto stmt : *node->body() ) {
            if ( !walk(stmt) ) {
                return false;
            }

            const Type* stmtType = _types->getTypeOf(stmt);
            if ( !hadFirst ) {
                innerType = stmtType;
            } else if ( !stmtType->is(innerType) ) {
                Reporting::typeError(
                    node->position(),
                    "Invalid entry in map at position " + std::to_string(idx) + ". Expected: " + innerType->toString() + "; Found: " + stmtType->toString()
                );
                return false;
            }

            idx += 1;
        }

        GenericType* type = GenericType::of(ValueType::TMAP, (Type*) innerType);
        _types->setTypeOf(node, type);
        return true;
    }

    virtual bool walkStringLiteralExpressionNode(StringLiteralExpressionNode* node) {
        _types->setTypeOf(node, PrimitiveType::of(ValueType::TSTRING));
        return true;
    }

    virtual bool walkNumberLiteralExpressionNode(NumberLiteralExpressionNode* node) {
        _types->setTypeOf(node, PrimitiveType::of(ValueType::TNUM));
        return true;
    }

    virtual bool walkAssignExpressionNode(AssignExpressionNode* node) {
        if (
            !walk(node->dest())
            || !walk(node->value())
        ) {
            return false;
        }

        const Type* typeOfValue = _types->getTypeOf(node->value());
        const Type* typeOfDest = _types->getTypeOf(node->dest());

        if ( !typeOfValue->is(typeOfDest) ) {
            Reporting::typeError(
                node->position(),
                "Attempted to assign value of type " + typeOfValue->toString() + " to lval of type " + typeOfDest->toString() + "."
            );

            return false;
        }

        _types->setTypeOf(node, PrimitiveType::of(ValueType::TUNIT, false));
        return true;
    }

    virtual bool walkUnitNode(UnitNode* node) {
        _types->setTypeOf(node, PrimitiveType::of(ValueType::TUNIT));
        return true;
    }

    virtual bool walkFunctionNode(FunctionNode* node) {
        // IMPLEMENT ME
        return true;
    }

    virtual bool walkNumericComparisonExpressionNode(NumericComparisonExpressionNode* node) {
        return walkPureBinaryExpression(node);
    }

    virtual bool walkTagResourceNode(TagResourceNode* node) {
        _types->setTypeOf(node, node->type());
        return true;
    }

    virtual std::string toString() const {
        return "TypeAnalysisWalk<>";
    }
private:
    TypeTable* _types;
};

}
}
}

#endif