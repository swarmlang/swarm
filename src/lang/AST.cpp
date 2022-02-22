#include <string>
#include <ostream>
#include <assert.h>
#include "AST.h"
#include "../runtime/ISymbolValueStore.h"
#include "SymbolTable.h"
#include "../Reporting.h"
#include "Type.h"
#include "Walk/PrintWalk.h"

#ifndef SWARMC_SPACE
#define SWARMC_SPACE "  "
#endif

namespace swarmc {
namespace Lang {

    ProgramNode* ProgramNode::copy() const  {
        auto other = new ProgramNode;
        for ( auto stmt : *_body ) {
            other->pushStatement(stmt->copy());
        }

        return other;
    }

    void ASTNode::printTree(std::ostream& out) {
        Walk::PrintWalk pw(out, this);
    }

    /************* VALUE ACCESSORS ********/
    void IdentifierNode::setValue(Runtime::ISymbolValueStore* store, ExpressionNode* value) {
        assert(value->isValue());
        store->setValue(_symbol, value);
    }

    ExpressionNode* IdentifierNode::getValue(Runtime::ISymbolValueStore* store) {
        auto value = store->getValue(_symbol);
        assert(value == nullptr || value->isValue());
        return value;
    }

    void MapAccessNode::setValue(Runtime::ISymbolValueStore* store, ExpressionNode* value) {
        // Get the lval we are keying into
        auto node = _path->getValue(store);

        // This should be caught by the type-checker, but anyway:
        assert(node->isValue() && node->getName() == "MapNode");
        assert(value->isValue());

        auto mapNode = (MapNode*) node;
        mapNode->setKey(_end, value);
    }

    ExpressionNode* MapAccessNode::getValue(Runtime::ISymbolValueStore* store) {
        // Get the lval we are keying into
        auto node = _path->getValue(store);

        // This should be caught by the type-checker, but anyway
        assert(node->isValue() && node->getName() == "MapNode");

        auto mapNode = (MapNode*) node;
        return mapNode->getKey(_end);
    }

    void EnumerableAccessNode::setValue(Runtime::ISymbolValueStore* store, ExpressionNode* value) {
        // Get the lval we are keying into
        auto node = _path->getValue(store);

        // This should be caught by the type-checker, but anyway
        assert(node->isValue() && node->getName() == "EnumerationLiteralExpressionNode");
        assert(value->isValue());

        auto enumNode = (EnumerationLiteralExpressionNode*) node;

        size_t idx = _index->value();  // fixme invalid cast?
        enumNode->setIndex(idx, value);  // todo handle exception here
    }

    ExpressionNode* EnumerableAccessNode::getValue(Runtime::ISymbolValueStore* store) {
        // Get the lval we are keying into
        auto node = _path->getValue(store);

        // This should be caught by the type-checker, but anyway
        assert(node->isValue() && node->getName() == "EnumerationLiteralExpressionNode");

        auto enumNode = (EnumerationLiteralExpressionNode*) node;
        size_t idx = _index->value();  // fixme invalid cast?
        return enumNode->getIndex(idx);
    }


    /************* TYPE ANALYSIS *************/

    bool ProgramNode::typeAnalysis(TypeTable* types) {
        for ( auto stmt : *_body ) {
            if ( !stmt->typeAnalysis(types) ) {
                return false;
            }
        }

        types->setTypeOf(this, PrimitiveType::of(ValueType::TUNIT, false));
        return true;
    }

    bool ExpressionStatementNode::typeAnalysis(TypeTable* types) {
        bool expResult = _exp->typeAnalysis(types);

        if ( expResult ) {
            types->setTypeOf(this, PrimitiveType::of(ValueType::TUNIT, false));
        }

        return expResult;
    }

    bool IdentifierNode::typeAnalysis(TypeTable* types) {
        if ( _symbol->type() == nullptr ) {
            Reporting::typeError(
                position(),
                "Invalid type of free identifier: " + _name
            );

            return false;
        }

        types->setTypeOf(this, _symbol->type());
        return true;
    }

    bool MapAccessNode::typeAnalysis(TypeTable* types) {
        bool pathresult = _path->typeAnalysis(types);
        if ( !pathresult ) {
            return false;
        }

        const Type* typeLVal = types->getTypeOf(_path);
        if ( !typeLVal->isGenericType() ) {
            Reporting::typeError(
                position(),
                "Invalid map access: " + _end->name()
            );
            return false;
        }

        types->setTypeOf(this, ((GenericType*) typeLVal)->concrete());
        return true;
    }

    bool EnumerableAccessNode::typeAnalysis(TypeTable* types) {
        bool pathresult = _path->typeAnalysis(types);
        if ( !pathresult ) {
            return false;
        }

        const Type* typeLVal = types->getTypeOf(_path);
        if ( !typeLVal->isGenericType() ) {
            Reporting::typeError(
                position(),
                "Invalid array access: " + std::to_string(_index->value())
            );
            return false;
        }

        types->setTypeOf(this, ((GenericType*) typeLVal)->concrete());
        return true;
    }

    bool TypeNode::typeAnalysis(TypeTable* types) {
        types->setTypeOf(this, PrimitiveType::of(ValueType::TUNIT, false));
        return true;
    }

    bool BooleanLiteralExpressionNode::typeAnalysis(TypeTable* types) {
        types->setTypeOf(this, PrimitiveType::of(ValueType::TBOOL, false));
        return true;
    }

    bool VariableDeclarationNode::typeAnalysis(TypeTable* types) {
        if (
            !_type->typeAnalysis(types)
            || !_id->typeAnalysis(types)
            || !_value->typeAnalysis(types)
        ) {
            return false;
        }

        const Type* typeOfValue = types->getTypeOf(_value);
        if ( !_type->type()->is(typeOfValue) ) {
            Reporting::typeError(
                position(),
                "Attempted to initialize identifier of type " + _type->type()->toString() + " with value of type " + typeOfValue->toString() + "."
            );

            return false;
        }

        types->setTypeOf(this, PrimitiveType::of(ValueType::TUNIT, false));
        return true;
    }

    bool AssignExpressionNode::typeAnalysis(TypeTable* types) {
        if (
            !_dest->typeAnalysis(types)
            || !_value->typeAnalysis(types)
        ) {
            return false;
        }

        const Type* typeOfValue = types->getTypeOf(_value);
        const Type* typeOfDest = types->getTypeOf(_dest);

        if ( !typeOfValue->is(typeOfDest) ) {
            Reporting::typeError(
                position(),
                "Attempted to assign value of type " + typeOfValue->toString() + " to lval of type " + typeOfDest->toString() + "."
            );

            return false;
        }

        types->setTypeOf(this, PrimitiveType::of(ValueType::TUNIT, false));
        return true;
    }

    bool CallExpressionNode::typeAnalysis(TypeTable* types) {
        // Perform type analysis on the callable
        if ( !_id->typeAnalysis(types) ) {
            return false;
        }

        // Perform type analysis on the arguments
        for ( auto arg : *_args ) {
            if ( !arg->typeAnalysis(types) ) {
                return false;
            }
        }

        // Make sure the callee is actually a function type
        const Type* baseTypeOfCallee = types->getTypeOf(_id);
        if ( baseTypeOfCallee->kind() != TypeKind::KFUNCTION ) {
            Reporting::typeError(
                position(),
                "Attempted to call non-callable type " + baseTypeOfCallee->toString() + "."
            );
        }

        const FunctionType* typeOfCallee = (FunctionType*) baseTypeOfCallee;
        std::vector<Type*>* argTypes = typeOfCallee->getArgumentTypes();

        // Make sure the # of arguments matches
        if ( argTypes->size() != _args->size() ) {
            Reporting::typeError(
                position(),
                "Invalid number of arguments for call (expected: " + std::to_string(argTypes->size()) + ")."
            );

            return false;
        }

        // Make sure the type of each argument matches
        for ( size_t i = 0; i < argTypes->size(); i += 1 ) {
            const Type* expectedType = argTypes->at(i);
            const Type* actualType = types->getTypeOf(_args->at(i));

            if ( !actualType->is(expectedType) ) {
                Reporting::typeError(
                    position(),
                    "Invalid argument of type " + actualType->toString() + " in position " + std::to_string(i) + " (expected: " + expectedType->toString() + ")."
                );

                return false;
            }
        }

        types->setTypeOf(this, typeOfCallee->returnType());
        return true;
    }

    bool PureBinaryExpressionNode::typeAnalysis(TypeTable* types) {
        if (
            !_left->typeAnalysis(types)
            || !_right->typeAnalysis(types)
        ) {
            return false;
        }

        const Type* actualLeftType = types->getTypeOf(_left);
        const Type* actualRightType = types->getTypeOf(_right);

        if ( !leftType()->is(actualLeftType) ) {
            Reporting::typeError(
                position(),
                "Invalid type " + actualLeftType->toString() + " of left-hand operand to expression (expected: " + leftType()->toString() + ")."
            );

            return false;
        }

        if ( !rightType()->is(actualRightType) ) {
            Reporting::typeError(
                position(),
                "Invalid type " + actualRightType->toString() + " of right-hand operand to expression (expected: " + rightType()->toString() + ")."
            );

            return false;
        }

        types->setTypeOf(this, resultType());
        return true;
    }

    bool EqualsNode::typeAnalysis(TypeTable* types) {
        if (
            !_left->typeAnalysis(types)
            || !_right->typeAnalysis(types)
        ) {
            return false;
        }

        const Type* actualLeftType = types->getTypeOf(_left);
        const Type* actualRightType = types->getTypeOf(_right);
        if ( !actualLeftType->is(actualRightType) ) {
            Reporting::typeError(
                position(),
                "Invalid comparison between left-hand type " + actualLeftType->toString() + " and right-hand type " + actualRightType->toString() + "."
            );

            return false;
        }

        types->setTypeOf(this, PrimitiveType::of(ValueType::TBOOL, false));
        return true;
    }

    bool NotEqualsNode::typeAnalysis(TypeTable* types) {
        if (
            !_left->typeAnalysis(types)
            || !_right->typeAnalysis(types)
        ) {
            return false;
        }

        const Type* actualLeftType = types->getTypeOf(_left);
        const Type* actualRightType = types->getTypeOf(_right);
        if ( !actualLeftType->is(actualRightType) ) {
            Reporting::typeError(
                position(),
                "Invalid comparison between left-hand type " + actualLeftType->toString() + " and right-hand type " + actualRightType->toString() + "."
            );

            return false;
        }

        types->setTypeOf(this, PrimitiveType::of(ValueType::TBOOL, false));
        return true;
    }

    bool IfStatement::typeAnalysis(TypeTable* types) {
        if (!_condition->typeAnalysis(types)) {
            return false;
        }

        const Type* condType = types->getTypeOf(_condition);
        if ( !condType->is(PrimitiveType::of(ValueType::TBOOL, false))) {
            Reporting::typeError(
                position(),
                "Condition for if statement not boolean " + condType->toString() + "."
            );

            return false;
        }

        for ( auto stmt : *_body ) {
            if ( !stmt->typeAnalysis(types) ) {
                return false;
            }
        }

        types->setTypeOf(this, PrimitiveType::of(ValueType::TUNIT, false));
        return true;
    }

    bool WhileStatement::typeAnalysis(TypeTable* types) {
        if (!_condition->typeAnalysis(types)) {
            return false;
        }

        const Type* condType = types->getTypeOf(_condition);
        if ( !condType->is(PrimitiveType::of(ValueType::TBOOL))) {
            Reporting::typeError(
                position(),
                "Condition for while loop not boolean " + condType->toString() + "."
            );

            return false;
        }

        for ( auto stmt : *_body ) {
            if ( !stmt->typeAnalysis(types) ) {
                return false;
            }
        }

        types->setTypeOf(this, PrimitiveType::of(ValueType::TUNIT, false));
        return true;
    }

    bool StringLiteralExpressionNode::typeAnalysis(TypeTable* types) {
        types->setTypeOf(this, PrimitiveType::of(ValueType::TSTRING));
        return true;
    }

    bool NumberLiteralExpressionNode::typeAnalysis(TypeTable* types) {
        types->setTypeOf(this, PrimitiveType::of(ValueType::TNUM));
        return true;
    }

    bool EnumerationStatement::typeAnalysis(TypeTable* types) {
        if ( !_enumerable->typeAnalysis(types) ) {
            return false;
        }

        const Type* enumType = _enumerable->symbol()->type();
        if ( !enumType->isGenericType() ) {
            Reporting::typeError(
                position(),
                "Attempted to enumerate invalid value"
            );
        }

        GenericType* genericType = (GenericType*) enumType;
        Type* concreteType = genericType->concrete();
        types->setTypeOf(_local, concreteType);

        if ( !_enumerable->typeAnalysis(types) ) {
            return false;
        }

        for ( auto stmt : *_body ) {
            if ( !stmt->typeAnalysis(types) ) {
                return false;
            }
        }

        types->setTypeOf(this, PrimitiveType::of(ValueType::TUNIT, false));
        return true;
    }

    bool WithStatement::typeAnalysis(TypeTable* types) {
        if ( !_resource->typeAnalysis(types) ) {
            return false;
        }

        const Type* type = types->getTypeOf(_resource);
        if ( type == nullptr || !type->isGenericType() || type->valueType() != ValueType::TRESOURCE ) {
            Reporting::typeError(
                position(),
                "Expected ValueType::TRESOURCE, found: " + (type == nullptr ? "none" : type->toString())
            );

            return false;
        }

        Type* localType = ((GenericType*) type)->concrete();

        if ( localType->isPrimitiveType() ) {
            localType = PrimitiveType::of(localType->valueType(), _shared);
        } else if ( localType->isGenericType() ) {
            localType = localType->copy();
            localType->setShared(_shared);
        } else if ( localType->isFunctionType() ) {
            localType = localType->copy();
            localType->setShared(_shared);
        }

        types->setTypeOf(_local, localType);
        _local->symbol()->_type = localType;  // local is implicitly defined, so need to set its type

        for ( auto stmt : *_body ) {
            if ( !stmt->typeAnalysis(types) ) {
                return false;
            }
        }

        types->setTypeOf(this, PrimitiveType::of(ValueType::TUNIT, false));
        return true;
    }

    bool EnumerationLiteralExpressionNode::typeAnalysis(TypeTable* types) {
        size_t idx = 0;
        bool hadFirst = false;
        const Type* innerType = nullptr;

        if ( _disambiguationType != nullptr ) {
            innerType = _disambiguationType->type();
            hadFirst = true;
        }

        for ( auto exp : *_actuals ) {
            if ( !exp->typeAnalysis(types) ) {
                return false;
            }

            const Type* expType = types->getTypeOf(exp);
            if ( !hadFirst ) {
                innerType = expType;
            } else if ( !expType->is(innerType) ) {
                Reporting::typeError(
                    position(),
                    "Invalid entry in enumerable at position " + std::to_string(idx) + ". Expected: " + innerType->toString() + "; Found: " + expType->toString()
                );
                return false;
            }

            idx += 1;
        }

        GenericType* type = GenericType::of(ValueType::TENUMERABLE, (Type*) innerType);
        types->setTypeOf(this, type);
        return true;
    }

    bool MapNode::typeAnalysis(TypeTable* types) {
        size_t idx = 0;
        bool hadFirst = false;
        const Type* innerType = nullptr;

        if ( _disambiguationType != nullptr ) {
            innerType = _disambiguationType->type();
            hadFirst = true;
        }

        for ( auto stmt : *_body ) {
            if ( !stmt->typeAnalysis(types) ) {
                return false;
            }

            const Type* stmtType = types->getTypeOf(stmt);
            if ( !hadFirst ) {
                innerType = stmtType;
            } else if ( !stmtType->is(innerType) ) {
                Reporting::typeError(
                    position(),
                    "Invalid entry in map at position " + std::to_string(idx) + ". Expected: " + innerType->toString() + "; Found: " + stmtType->toString()
                );
                return false;
            }

            idx += 1;
        }

        GenericType* type = GenericType::of(ValueType::TMAP, (Type*) innerType);
        types->setTypeOf(this, type);
        return true;
    }

    bool MapStatementNode::typeAnalysis(TypeTable* types) {
        if ( !_value->typeAnalysis(types) ) {
            return false;
        }

        types->setTypeOf(this, types->getTypeOf(_value));
        return true;
    }

    bool NegativeExpressionNode::typeAnalysis(TypeTable* types) {
        if ( !_exp->typeAnalysis(types) ) {
            return false;
        }

        const Type* numType = PrimitiveType::of(ValueType::TNUM);
        const Type* expType = types->getTypeOf(_exp);
        if ( !expType->is(numType) ) {
            Reporting::typeError(
                position(),
                "Attempted to perform numeric negation on invalid type. Expected: " + numType->toString() + "; actual: " + expType->toString()
            );
            return false;
        }

        types->setTypeOf(this, numType);
        return true;
    }

    bool NotNode::typeAnalysis(TypeTable* types) {
        if ( !_exp->typeAnalysis(types) ) {
            return false;
        }

        const Type* boolType = PrimitiveType::of(ValueType::TBOOL);
        const Type* expType = types->getTypeOf(_exp);
        if ( !expType->is(boolType) ) {
            Reporting::typeError(
                position(),
                "Attempted to perform boolean negation on invalid type. Expected: " + boolType->toString() + "; actual: " + expType->toString()
            );
            return false;
        }

        types->setTypeOf(this, boolType);
        return true;
    }
}
}
