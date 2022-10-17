#ifndef SWARMC_TYPE_ANALYSIS_WALK_H
#define SWARMC_TYPE_ANALYSIS_WALK_H

#include <stack>
#include "Walk.h"
#include "../TypeTable.h"
#include "../../Reporting.h"

namespace swarmc {
namespace Lang {
namespace Walk {

class TypeAnalysisWalk : public Walk<bool> {
public:
    TypeAnalysisWalk() : Walk<bool>(), _types(new TypeTable()), 
        _funcTypes(new std::stack<const Type::Type*>()), _funcArgs(new std::stack<int>()) {}
    ~TypeAnalysisWalk() {
        delete _types;
        delete _funcTypes;
        delete _funcArgs;
    }
protected:
    virtual bool walkProgramNode(ProgramNode* node) {
        for ( auto stmt : *node->body() ) {
            if ( !walk(stmt) ) {
                return false;
            }
        }

        _types->setTypeOf(node, Type::Primitive::of(Type::Intrinsic::UNIT));
        return true;
    }

    virtual bool walkExpressionStatementNode(ExpressionStatementNode* node) {
        bool expResult = walk(node->expression());

        if ( expResult ) {
            _types->setTypeOf(node, Type::Primitive::of(Type::Intrinsic::UNIT));
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

        const Type::Type* typeLVal = _types->getTypeOf(node->path());
        if ( typeLVal->intrinsic() != Type::Intrinsic::MAP ) {
            Reporting::typeError(
                node->position(),
                "Invalid map access: " + node->end()->name()
            );
            return false;
        }

        _types->setTypeOf(node, ((Type::Map*) typeLVal)->values());
        return true;
    }

    virtual bool walkEnumerableAccessNode(EnumerableAccessNode* node) {
        bool pathresult = walk(node->path());
        if ( !pathresult ) {
            return false;
        }

        const Type::Type* typeLVal = _types->getTypeOf(node->path());
        if ( typeLVal->intrinsic() != Type::Intrinsic::ENUMERABLE ) {
            Reporting::typeError(
                node->position(),
                "Invalid array access: " + std::to_string(node->index()->value())
            );
            return false;
        }

        _types->setTypeOf(node, ((Type::Enumerable*) typeLVal)->values());
        return true;
    }

    virtual bool walkTypeLiteral(swarmc::Lang::TypeLiteral *node) {
        _types->setTypeOf(node, Type::Primitive::of(Type::Intrinsic::TYPE));
        return true;
    }

    virtual bool walkBooleanLiteralExpressionNode(BooleanLiteralExpressionNode* node) {
        _types->setTypeOf(node, Type::Primitive::of(Type::Intrinsic::BOOLEAN));
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

        const Type::Type* typeOfValue = _types->getTypeOf(node->value());
        if ( !node->typeNode()->value()->isAssignableTo(typeOfValue) ) {
            
            Reporting::typeError(
                node->position(),
                "Attempted to initialize identifier of type " + node->typeNode()->value()->toString() + " with value of type " + typeOfValue->toString() + "."
            );

            return false;
        }

        _types->setTypeOf(node, Type::Primitive::of(Type::Intrinsic::UNIT));
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
        const Type::Type* baseTypeOfCallee = _types->getTypeOf(node->id());
        if ( baseTypeOfCallee->intrinsic() != Type::Intrinsic::LAMBDA0 && baseTypeOfCallee->intrinsic() != Type::Intrinsic::LAMBDA1 ) {
            Reporting::typeError(
                node->position(),
                "Attempted to call non-callable type " + baseTypeOfCallee->toString() + "."
            );
        }

        const Type::Lambda* typeOfCallee = (Type::Lambda*) baseTypeOfCallee;
        auto argTypes = typeOfCallee->params();

        // Make sure the # of arguments matches
        if ( argTypes.size() != node->args()->size() ) {
            Reporting::typeError(
                node->position(),
                "Invalid number of arguments for call (expected: " + std::to_string(argTypes.size()) + ")."
            );

            return false;
        }

        const Type::Type* nodeType = typeOfCallee;

        if ( nodeType->intrinsic() == Type::Intrinsic::LAMBDA0 ) {
            nodeType = ((Type::Lambda*) nodeType)->returns();
        }

        // Make sure the type of each argument matches
        for ( size_t i = 0; i < argTypes.size(); i += 1 ) {
            const Type::Type* expectedType = argTypes.at(i);
            const Type::Type* actualType = _types->getTypeOf(node->args()->at(i));

            if ( !actualType->isAssignableTo(expectedType) ) {
                Reporting::typeError(
                    node->position(),
                    "Invalid argument of type " + actualType->toString() + " in position " + std::to_string(i) + " (expected: " + expectedType->toString() + ")."
                );

                return false;
            }

            assert(nodeType->intrinsic() == Type::Intrinsic::LAMBDA0 || nodeType->intrinsic() == Type::Intrinsic::LAMBDA1);
            nodeType = ((Type::Lambda*) nodeType)->returns();
        }

        _types->setTypeOf(node, nodeType);
        return true;
    }

    virtual bool walkIIFExpressionNode(IIFExpressionNode* node) {
        if ( !walk(node->expression()) ) {
            return false;
        }

        // Perform type analysis on the arguments
        for ( auto arg : *node->args() ) {
            if ( !walk(arg) ) {
                return false;
            }
        }

        if ( !node->expression()->type()->isCallable() ) {
            return false;
        }

        const Type::Lambda* typeOfCallee = (Type::Lambda*) node->expression()->type();
        auto argTypes = typeOfCallee->params();

        const Type::Type* nodeType = typeOfCallee;

        if ( nodeType->intrinsic() == Type::Intrinsic::LAMBDA0 ) {
            nodeType = ((Type::Lambda*) nodeType)->returns();
        }

        // Make sure the type of each argument matches
        for ( size_t i = 0; i < argTypes.size(); i += 1 ) {
            const Type::Type* expectedType = argTypes.at(i);
            const Type::Type* actualType = _types->getTypeOf(node->args()->at(i));

            if ( !actualType->isAssignableTo(expectedType) ) {
                Reporting::typeError(
                    node->position(),
                    "Invalid argument of type " + actualType->toString() + " in position " + std::to_string(i) + " (expected: " + expectedType->toString() + ")."
                );

                return false;
            }

            assert(nodeType->intrinsic() == Type::Intrinsic::LAMBDA0 || nodeType->intrinsic() == Type::Intrinsic::LAMBDA1);
            nodeType = ((Type::Lambda*) nodeType)->returns();
        }

        _types->setTypeOf(node, nodeType);
        return true;
    }

    virtual bool walkPureBinaryExpression(PureBinaryExpressionNode* node) {
        if (
            !walk(node->left())
            || !walk(node->right())
        ) {
            return false;
        }

        const Type::Type* actualLeftType = _types->getTypeOf(node->left());
        const Type::Type* actualRightType = _types->getTypeOf(node->right());

        if ( !node->leftType()->isAssignableTo(actualLeftType) ) {
            Reporting::typeError(
                node->position(),
                "Invalid type " + actualLeftType->toString() + " of left-hand operand to expression (expected: " + node->leftType()->toString() + ")."
            );

            return false;
        }

        if ( !node->rightType()->isAssignableTo(actualRightType) ) {
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

        const Type::Type* actualLeftType = _types->getTypeOf(node->left());
        const Type::Type* actualRightType = _types->getTypeOf(node->right());
        if ( !actualLeftType->isAssignableTo(actualRightType) ) {
            Reporting::typeError(
                node->position(),
                "Invalid comparison between left-hand type " + actualLeftType->toString() + " and right-hand type " + actualRightType->toString() + "."
            );

            return false;
        }

        _types->setTypeOf(node, Type::Primitive::of(Type::Intrinsic::BOOLEAN));
        return true;
    }

    virtual bool walkNotEqualsNode(NotEqualsNode* node) {
        if (
            !walk(node->left())
            || !walk(node->right())
        ) {
            return false;
        }

        const Type::Type* actualLeftType = _types->getTypeOf(node->left());
        const Type::Type* actualRightType = _types->getTypeOf(node->right());
        if ( !actualLeftType->isAssignableTo(actualRightType) ) {
            Reporting::typeError(
                node->position(),
                "Invalid comparison between left-hand type " + actualLeftType->toString() + " and right-hand type " + actualRightType->toString() + "."
            );

            return false;
        }

        _types->setTypeOf(node, Type::Primitive::of(Type::Intrinsic::BOOLEAN));
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

        const Type::Type* numType = Type::Primitive::of(Type::Intrinsic::NUMBER);
        const Type::Type* expType = _types->getTypeOf(node->exp());
        if ( !expType->isAssignableTo(numType) ) {
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

        const Type::Type* boolType = Type::Primitive::of(Type::Intrinsic::BOOLEAN);
        const Type::Type* expType = _types->getTypeOf(node->exp());
        if ( !expType->isAssignableTo(boolType) ) {
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
        const Type::Type* innerType = nullptr;

        if ( node->_disambiguationType != nullptr ) {
            innerType = node->_disambiguationType->value();
            hadFirst = true;
        }

        for ( auto exp : *node->actuals() ) {
            if ( !walk(exp) ) {
                return false;
            }

            const Type::Type* expType = _types->getTypeOf(exp);
            if ( !hadFirst ) {
                innerType = expType;
            } else if ( !expType->isAssignableTo(innerType) ) {
                Reporting::typeError(
                    node->position(),
                    "Invalid entry in enumerable at position " + std::to_string(idx) + ". Expected: " + innerType->toString() + "; Found: " + expType->toString()
                );
                return false;
            }

            idx += 1;
        }

        auto type = new Type::Enumerable(innerType);
        _types->setTypeOf(node, type);
        return true;
    }

    virtual bool walkCapturedBlockStatementNode(CapturedBlockStatementNode* node) {
        for ( auto stmt : *node->body() ) {
            if ( !walk(stmt) ) {
                return false;
            }
        }

        _types->setTypeOf(node, Type::Primitive::of(Type::Intrinsic::UNIT));
        return true;
    }

    virtual bool walkEnumerationStatement(EnumerationStatement* node) {
        if ( !walk(node->enumerable()) ) {
            return false;
        }

        const Type::Type* enumType = node->enumerable()->type();
        if ( enumType->intrinsic() != Type::Intrinsic::ENUMERABLE ) {
            Reporting::typeError(
                node->position(),
                "Attempted to enumerate invalid value"
            );
        }

        auto genericType = (Type::Enumerable*) enumType;
        auto concreteType = genericType->values();
        _types->setTypeOf(node->local(), concreteType);

        for ( auto stmt : *node->body() ) {
            if ( !walk(stmt) ) {
                return false;
            }
        }

        _types->setTypeOf(node, Type::Primitive::of(Type::Intrinsic::UNIT));
        return true;
    }

    virtual bool walkWithStatement(WithStatement* node) {
        if ( !walk(node->resource()) ) {
            return false;
        }

        const Type::Type* type = _types->getTypeOf(node->resource());
        if ( type == nullptr || type->intrinsic() != Type::Intrinsic::RESOURCE ) {
            Reporting::typeError(
                node->position(),
                "Expected Intrinsic::RESOURCE, found: " + (type == nullptr ? "none" : type->toString())
            );

            return false;
        }

        auto localType = ((Type::Resource*) type)->yields()->copy();

        _types->setTypeOf(node->local(), localType);
        node->local()->symbol()->_type = localType;  // local is implicitly defined, so need to set its type

        for ( auto stmt : *node->body() ) {
            if ( !walk(stmt) ) {
                return false;
            }
        }

        _types->setTypeOf(node, Type::Primitive::of(Type::Intrinsic::UNIT));
        return true;
    }

    virtual bool walkIfStatement(IfStatement* node) {
        if (!walk(node->condition())) {
            return false;
        }

        const Type::Type* condType = _types->getTypeOf(node->condition());
        if ( !condType->isAssignableTo(Type::Primitive::of(Type::Intrinsic::BOOLEAN)) ) {
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

        _types->setTypeOf(node, Type::Primitive::of(Type::Intrinsic::UNIT));
        return true;
    }

    virtual bool walkWhileStatement(WhileStatement* node) {
        if (!walk(node->condition())) {
            return false;
        }

        const Type::Type* condType = _types->getTypeOf(node->condition());
        if ( !condType->isAssignableTo(Type::Primitive::of(Type::Intrinsic::BOOLEAN)) ) {
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

        _types->setTypeOf(node, Type::Primitive::of(Type::Intrinsic::UNIT));
        return true;
    }

    virtual bool walkContinueNode(ContinueNode* node) {
        return true;
    }

    virtual bool walkBreakNode(BreakNode* node) {
        return true;
    }

    virtual bool walkReturnStatementNode(ReturnStatementNode* node) {
        if ( _funcTypes->empty() ) {
            Reporting::syntaxError(
                node->position(),
                "Found return statement outside of a function"
            );
            return false;
        }

        const Type::Type* funcType = _funcTypes->top();

        if ( funcType->intrinsic() == Type::Intrinsic::LAMBDA0 ) {
            funcType = ((Type::Lambda*) funcType)->returns();
        } else {
            for ( int i = 0; i < _funcArgs->top(); i++ ) {
                assert(funcType->intrinsic() == Type::Intrinsic::LAMBDA0 || funcType->intrinsic() == Type::Intrinsic::LAMBDA1);
                funcType = ((Type::Lambda*) funcType)->returns();
            }
        }
        
        if ( node->value() == nullptr ) {
            if ( funcType->intrinsic() != Type::Intrinsic::VOID ) {
                Reporting::typeError(
                    node->position(),
                    "Invalid return type. Expected: " + funcType->toString() + "; Found: VOID"
                );
                return false;
            }

            return true;
        }

        if ( !walk(node->value()) ) {
            return false;
        }

        auto retType = node->value()->type();

        if ( !retType->isAssignableTo(funcType) ) {
            Reporting::typeError(
                node->position(),
                "Invalid return type. Expected: " + funcType->toString() + "; Found: " + retType->toString()
            );
            return false;
        }
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
        const Type::Type* innerType = nullptr;

        if ( node->_disambiguationType != nullptr ) {
            innerType = node->_disambiguationType->value();
            hadFirst = true;
        }

        for ( auto stmt : *node->body() ) {
            if ( !walk(stmt) ) {
                return false;
            }

            const Type::Type* stmtType = _types->getTypeOf(stmt);
            if ( !hadFirst ) {
                innerType = stmtType;
            } else if ( !stmtType->isAssignableTo(innerType) ) {
                Reporting::typeError(
                    node->position(),
                    "Invalid entry in map at position " + std::to_string(idx) + ". Expected: " + innerType->toString() + "; Found: " + stmtType->toString()
                );
                return false;
            }

            idx += 1;
        }

        auto type = new Type::Map(innerType);
        _types->setTypeOf(node, type);
        return true;
    }

    virtual bool walkStringLiteralExpressionNode(StringLiteralExpressionNode* node) {
        _types->setTypeOf(node, Type::Primitive::of(Type::Intrinsic::STRING));
        return true;
    }

    virtual bool walkNumberLiteralExpressionNode(NumberLiteralExpressionNode* node) {
        _types->setTypeOf(node, Type::Primitive::of(Type::Intrinsic::NUMBER));
        return true;
    }

    virtual bool walkAssignExpressionNode(AssignExpressionNode* node) {
        if (
            !walk(node->dest())
            || !walk(node->value())
        ) {
            return false;
        }

        const Type::Type* typeOfValue = _types->getTypeOf(node->value());
        const Type::Type* typeOfDest = _types->getTypeOf(node->dest());

        if ( !typeOfValue->isAssignableTo(typeOfDest) ) {
            Reporting::typeError(
                node->position(),
                "Attempted to assign value of type " + typeOfValue->toString() + " to lval of type " + typeOfDest->toString() + "."
            );

            return false;
        }

        _types->setTypeOf(node, Type::Primitive::of(Type::Intrinsic::UNIT));
        return true;
    }

    virtual bool walkUnitNode(UnitNode* node) {
        _types->setTypeOf(node, Type::Primitive::of(Type::Intrinsic::UNIT));
        return true;
    }

    virtual bool walkFunctionNode(FunctionNode* node) {
        _funcTypes->push(node->type());
        _funcArgs->push(node->formals()->size());
        for ( auto stmt : *node->body() ) {
            if ( !walk(stmt) ) {
                return false;
            }
        }
        _funcTypes->pop();
        _funcArgs->pop();

        _types->setTypeOf(node, node->type());
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
    std::stack<const Type::Type*>* _funcTypes;
    std::stack<int>* _funcArgs;
};

}
}
}

#endif