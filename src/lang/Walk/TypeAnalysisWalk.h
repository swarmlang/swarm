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
    TypeAnalysisWalk() : Walk<bool>(), _whileCount(0), _funcCount(0), _types(new TypeTable()), 
        _funcTypes(new std::stack<const Type::Type*>()), _funcArgs(new std::stack<int>()) {}
    ~TypeAnalysisWalk() {
        delete _types;
        delete _funcTypes;
        delete _funcArgs;
    }
protected:
    virtual bool walkProgramNode(ProgramNode* node) {
        bool flag = true;
        for ( auto stmt : *node->body() ) {
            flag = walk(stmt) && flag;
        }

        auto type = flag ? Type::Primitive::of(Type::Intrinsic::UNIT) : Type::Primitive::of(Type::Intrinsic::ERROR);
        _types->setTypeOf(node, type);
        return flag;
    }

    virtual bool walkExpressionStatementNode(ExpressionStatementNode* node) {
        bool flag = walk(node->expression());

        auto type = flag ? Type::Primitive::of(Type::Intrinsic::UNIT) : Type::Primitive::of(Type::Intrinsic::ERROR);
        _types->setTypeOf(node, type);

        return flag;
    }

    virtual bool walkIdentifierNode(IdentifierNode* node) {
        if ( node->_symbol->type() == nullptr ) {
            Reporting::typeError(
                node->position(),
                "Invalid type of free identifier: " + node->name()
            );

            _types->setTypeOf(node, Type::Primitive::of(Type::Intrinsic::ERROR));
            return false;
        }

        _types->setTypeOf(node, node->_symbol->type());
        return true;
    }

    virtual bool walkMapAccessNode(MapAccessNode* node) {
        bool pathresult = walk(node->path());
        if ( !pathresult ) {
            _types->setTypeOf(node, Type::Primitive::of(Type::Intrinsic::ERROR));
            return false;
        }

        const Type::Type* typeLVal = _types->getTypeOf(node->path());
        if ( typeLVal->intrinsic() != Type::Intrinsic::MAP ) {
            Reporting::typeError(
                node->position(),
                "Invalid map access: " + node->end()->name()
            );
            _types->setTypeOf(node, Type::Primitive::of(Type::Intrinsic::ERROR));
            return false;
        }

        _types->setTypeOf(node, ((Type::Map*) typeLVal)->values());
        return true;
    }

    virtual bool walkEnumerableAccessNode(EnumerableAccessNode* node) {
        bool pathresult = walk(node->path());
        if ( !pathresult ) {
            _types->setTypeOf(node, Type::Primitive::of(Type::Intrinsic::ERROR));
            return false;
        }

        const Type::Type* typeLVal = _types->getTypeOf(node->path());
        if ( typeLVal->intrinsic() != Type::Intrinsic::ENUMERABLE ) {
            Reporting::typeError(
                node->position(),
                "Invalid array access: " + node->path()->toString()
            );
            _types->setTypeOf(node, Type::Primitive::of(Type::Intrinsic::ERROR));
            return false;
        }

        bool indexresult = walk(node->index());
        if ( !indexresult ) {
            _types->setTypeOf(node, Type::Primitive::of(Type::Intrinsic::ERROR));
            return false;
        }

        const Type::Type* typeIndex = _types->getTypeOf(node->index());
        if ( typeIndex->intrinsic() != Type::Intrinsic::NUMBER ) {
            Reporting::typeError(
                node->position(),
                "Invalid index type: " + node->index()->type()->toString()
            );
            _types->setTypeOf(node, Type::Primitive::of(Type::Intrinsic::ERROR));
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
        bool flag = walk(node->typeNode());
        flag = walk(node->id()) && flag;
        flag = walk(node->value()) && flag;

        const Type::Type* typeOfValue = _types->getTypeOf(node->value());
        if ( !node->typeNode()->value()->isAssignableTo(typeOfValue)
            && typeOfValue->intrinsic() != Type::Intrinsic::ERROR
        ) {
            
            Reporting::typeError(
                node->position(),
                "Attempted to initialize identifier of type " + node->typeNode()->value()->toString() + " with value of type " + typeOfValue->toString() + "."
            );

            flag = false;
        }

        auto type = flag ? Type::Primitive::of(Type::Intrinsic::UNIT) : Type::Primitive::of(Type::Intrinsic::ERROR);
        _types->setTypeOf(node, type);
        return flag;
    }

    virtual bool walkCallExpressionNode(CallExpressionNode* node) {
        // Perform type analysis on the callable
        bool flag = walk(node->id());

        // Perform type analysis on the arguments
        for ( auto arg : *node->args() ) {
            flag = walk(arg) && flag;
        }

        // Make sure the callee is actually a function type
        const Type::Type* baseTypeOfCallee = _types->getTypeOf(node->id());
        if ( !baseTypeOfCallee->isCallable() ) {
            Reporting::typeError(
                node->position(),
                "Attempted to call non-callable type " + baseTypeOfCallee->toString() + "."
            );
        
            _types->setTypeOf(node, Type::Primitive::of(Type::Intrinsic::ERROR));
            return false;
        }

        const Type::Lambda* typeOfCallee = (Type::Lambda*) baseTypeOfCallee;
        auto argTypes = typeOfCallee->params();

        // Make sure the # of arguments matches
        if ( argTypes.size() < node->args()->size() ) {
            Reporting::typeError(
                node->position(),
                "Invalid number of arguments for call (expected: " + std::to_string(argTypes.size()) + ")."
            );

            flag = false;
        }

        const Type::Type* nodeType = typeOfCallee;

        if ( nodeType->intrinsic() == Type::Intrinsic::LAMBDA0 ) {
            nodeType = ((Type::Lambda*) nodeType)->returns();
        }

        // Make sure the type of each argument matches
        for ( size_t i = 0; i < node->args()->size(); i += 1 ) {
            const Type::Type* expectedType = argTypes.at(i);
            const Type::Type* actualType = _types->getTypeOf(node->args()->at(i));

            if ( !actualType->isAssignableTo(expectedType) 
                && actualType->intrinsic() != Type::Intrinsic::ERROR
            ) {
                Reporting::typeError(
                    node->position(),
                    "Invalid argument of type " + actualType->toString() + " in position " + std::to_string(i) + " (expected: " + expectedType->toString() + ")."
                );

                flag = false;
            }

            assert(nodeType->isCallable());
            nodeType = ((Type::Lambda*) nodeType)->returns();
        }

        _types->setTypeOf(node, flag ? nodeType : Type::Primitive::of(Type::Intrinsic::ERROR));
        return flag;
    }

    virtual bool walkIIFExpressionNode(IIFExpressionNode* node) {
        bool flag = walk(node->expression());

        // Perform type analysis on the arguments
        for ( auto arg : *node->args() ) {
            flag = walk(arg) && flag;
        }

        if ( !node->expression()->type()->isCallable() ) {
            Reporting::typeError(
                node->position(),
                "Attempted to call non-callable type " + node->expression()->type()->toString() + "."
            );
        
            _types->setTypeOf(node, Type::Primitive::of(Type::Intrinsic::ERROR));
            return false;
        }

        const Type::Lambda* typeOfCallee = (Type::Lambda*) node->expression()->type();
        auto argTypes = typeOfCallee->params();

        // Make sure the # of arguments matches
        if ( argTypes.size() < node->args()->size() ) {
            Reporting::typeError(
                node->position(),
                "Invalid number of arguments for call (expected: " + std::to_string(argTypes.size()) + ")."
            );

            flag = false;
        }

        const Type::Type* nodeType = typeOfCallee;

        if ( nodeType->intrinsic() == Type::Intrinsic::LAMBDA0 ) {
            nodeType = ((Type::Lambda*) nodeType)->returns();
        }

        // Make sure the type of each argument matches
        for ( size_t i = 0; i < node->args()->size(); i += 1 ) {
            const Type::Type* expectedType = argTypes.at(i);
            const Type::Type* actualType = _types->getTypeOf(node->args()->at(i));

            if ( !actualType->isAssignableTo(expectedType) 
                && actualType->intrinsic() != Type::Intrinsic::ERROR
            ) {
                Reporting::typeError(
                    node->position(),
                    "Invalid argument of type " + actualType->toString() + " in position " + std::to_string(i) + " (expected: " + expectedType->toString() + ")."
                );

                flag = false;
            }

            assert(nodeType->isCallable());
            nodeType = ((Type::Lambda*) nodeType)->returns();
        }

        _types->setTypeOf(node, flag ? nodeType : Type::Primitive::of(Type::Intrinsic::ERROR));
        return flag;
    }

    virtual bool walkPureBinaryExpression(PureBinaryExpressionNode* node) {
        bool flag = walk(node->left());
        flag = walk(node->right()) && flag;

        const Type::Type* actualLeftType = _types->getTypeOf(node->left());
        const Type::Type* actualRightType = _types->getTypeOf(node->right());

        if ( !node->leftType()->isAssignableTo(actualLeftType) && actualLeftType->intrinsic() != Type::Intrinsic::ERROR ) {
            Reporting::typeError(
                node->position(),
                "Invalid type " + actualLeftType->toString() + " of left-hand operand to expression (expected: " + node->leftType()->toString() + ")."
            );

            flag = false;
        }

        if ( !node->rightType()->isAssignableTo(actualRightType) && actualRightType->intrinsic() != Type::Intrinsic::ERROR ) {
            Reporting::typeError(
                node->position(),
                "Invalid type " + actualRightType->toString() + " of right-hand operand to expression (expected: " + node->rightType()->toString() + ")."
            );

            flag = false;
        }

        _types->setTypeOf(node, flag ? node->resultType() : Type::Primitive::of(Type::Intrinsic::ERROR));
        return flag;
    }

    virtual bool walkAndNode(AndNode* node) {
        return walkPureBinaryExpression(node);
    }

    virtual bool walkOrNode(OrNode* node) {
        return walkPureBinaryExpression(node);
    }

    virtual bool walkEqualsNode(EqualsNode* node) {
        bool flag = walk(node->left());
        flag = walk(node->right()) && flag;

        const Type::Type* actualLeftType = _types->getTypeOf(node->left());
        const Type::Type* actualRightType = _types->getTypeOf(node->right());
        if ( !actualLeftType->isAssignableTo(actualRightType) 
            && actualLeftType->intrinsic() != Type::Intrinsic::ERROR
            && actualRightType->intrinsic() != Type::Intrinsic::ERROR
        ) {
            Reporting::typeError(
                node->position(),
                "Invalid comparison between left-hand type " + actualLeftType->toString() + " and right-hand type " + actualRightType->toString() + "."
            );

            flag = false;
        }

        auto type = flag ? Type::Primitive::of(Type::Intrinsic::BOOLEAN) : Type::Primitive::of(Type::Intrinsic::ERROR);
        _types->setTypeOf(node, type);
        return flag;
    }

    virtual bool walkNotEqualsNode(NotEqualsNode* node) {
        bool flag = walk(node->left());
        flag = walk(node->right()) && flag;

        const Type::Type* actualLeftType = _types->getTypeOf(node->left());
        const Type::Type* actualRightType = _types->getTypeOf(node->right());
        if ( !actualLeftType->isAssignableTo(actualRightType)
            && actualLeftType->intrinsic() != Type::Intrinsic::ERROR
            && actualRightType->intrinsic() != Type::Intrinsic::ERROR
        ) {
            Reporting::typeError(
                node->position(),
                "Invalid comparison between left-hand type " + actualLeftType->toString() + " and right-hand type " + actualRightType->toString() + "."
            );

            flag = false;
        }

        auto type = flag ? Type::Primitive::of(Type::Intrinsic::BOOLEAN) : Type::Primitive::of(Type::Intrinsic::ERROR);
        _types->setTypeOf(node, type);
        return flag;
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
        bool flag = walk(node->exp());

        const Type::Type* numType = Type::Primitive::of(Type::Intrinsic::NUMBER);
        const Type::Type* expType = _types->getTypeOf(node->exp());
        if ( !expType->isAssignableTo(numType) && expType->intrinsic() != Type::Intrinsic::ERROR ) {
            Reporting::typeError(
                node->position(),
                "Attempted to perform numeric negation on invalid type. Expected: " + numType->toString() + "; actual: " + expType->toString()
            );
            flag = false;
        }

        _types->setTypeOf(node, flag ? numType : Type::Primitive::of(Type::Intrinsic::ERROR));
        return flag;
    }

    virtual bool walkNotNode(NotNode* node) {
        bool flag = walk(node->exp());

        const Type::Type* boolType = Type::Primitive::of(Type::Intrinsic::BOOLEAN);
        const Type::Type* expType = _types->getTypeOf(node->exp());
        if ( !expType->isAssignableTo(boolType) && expType->intrinsic() != Type::Intrinsic::ERROR ) {
            Reporting::typeError(
                node->position(),
                "Attempted to perform boolean negation on invalid type. Expected: " + boolType->toString() + "; actual: " + expType->toString()
            );
            flag = false;
        }

        _types->setTypeOf(node, flag ? boolType : Type::Primitive::of(Type::Intrinsic::ERROR));
        return flag;
    }

    virtual bool walkEnumerationLiteralExpressionNode(EnumerationLiteralExpressionNode* node) {
        size_t idx = 0;
        bool hadFirst = false;
        const Type::Type* innerType = nullptr;
        if (node->_type != nullptr) {
            assert(node->type()->intrinsic() == Type::Intrinsic::ENUMERABLE);
            innerType = ((Type::Enumerable*)node->type())->values();
            hadFirst = true;
        }

        bool flag = true;
        for ( auto exp : *node->actuals() ) {
            flag = walk(exp) && flag;

            const Type::Type* expType = _types->getTypeOf(exp);
            if ( !hadFirst ) {
                innerType = expType;
                node->_type = new TypeLiteral(node->position()->copy(), new Type::Enumerable(innerType));
                hadFirst = false;
            } else if ( !expType->isAssignableTo(innerType) ) {
                Reporting::typeError(
                    node->position(),
                    "Invalid entry in enumerable at position " + std::to_string(idx) + ". Expected: " + innerType->toString() + "; Found: " + expType->toString()
                );
                flag = false;
            }

            idx += 1;
        }

        _types->setTypeOf(node, flag ? node->type() : Type::Primitive::of(Type::Intrinsic::ERROR));
        return flag;
    }

    virtual bool walkEnumerationStatement(EnumerationStatement* node) {
        bool flag = walk(node->enumerable());

        const Type::Type* enumType = node->enumerable()->type();
        if ( enumType->intrinsic() != Type::Intrinsic::ENUMERABLE &&
            enumType->intrinsic() != Type::Intrinsic::ERROR ) {
            Reporting::typeError(
                node->position(),
                "Attempted to enumerate invalid value"
            );
            flag = false;
        }

        auto genericType = (Type::Enumerable*) enumType;
        auto concreteType = genericType->values();
        _types->setTypeOf(node->local(), concreteType);

        int temp = _funcCount;
        _funcCount = 0;
        for ( auto stmt : *node->body() ) {
            flag = walk(stmt) && flag;
        }
        _funcCount = temp;

        auto type = flag ? Type::Primitive::of(Type::Intrinsic::UNIT) : Type::Primitive::of(Type::Intrinsic::ERROR);
        _types->setTypeOf(node, type);
        return flag;
    }

    virtual bool walkWithStatement(WithStatement* node) {
        bool flag = walk(node->resource());

        const Type::Type* type = _types->getTypeOf(node->resource());
        if ( (type == nullptr || type->intrinsic() != Type::Intrinsic::RESOURCE)
            && type->intrinsic() != Type::Intrinsic::ERROR ) {
            Reporting::typeError(
                node->position(),
                "Expected Intrinsic::RESOURCE, found: " + (type == nullptr ? "none" : type->toString())
            );

            flag = false;
        }

        auto localType = ((Type::Resource*) type)->yields()->copy(node->_shared);

        _types->setTypeOf(node->local(), localType);
        node->local()->symbol()->_type = localType;  // local is implicitly defined, so need to set its type

        for ( auto stmt : *node->body() ) {
            flag = walk(stmt) && flag;
        }

        auto t = flag ? Type::Primitive::of(Type::Intrinsic::UNIT) : Type::Primitive::of(Type::Intrinsic::ERROR);
        _types->setTypeOf(node, t);
        return flag;
    }

    virtual bool walkIfStatement(IfStatement* node) {
        bool flag = walk(node->condition());

        const Type::Type* condType = _types->getTypeOf(node->condition());
        if ( !condType->isAssignableTo(Type::Primitive::of(Type::Intrinsic::BOOLEAN)) 
            && condType->intrinsic() != Type::Intrinsic::ERROR
        ) {
            Reporting::typeError(
                node->position(),
                "Condition for if statement not boolean " + condType->toString() + "."
            );

            flag = false;
        }

        for ( auto stmt : *node->body() ) {
            flag = walk(stmt) && flag;
        }

        auto type = flag ? Type::Primitive::of(Type::Intrinsic::UNIT) : Type::Primitive::of(Type::Intrinsic::ERROR);
        _types->setTypeOf(node, type);
        return flag;
    }

    virtual bool walkWhileStatement(WhileStatement* node) {
        bool flag = walk(node->condition());

        const Type::Type* condType = _types->getTypeOf(node->condition());
        if ( !condType->isAssignableTo(Type::Primitive::of(Type::Intrinsic::BOOLEAN)) 
            && condType->intrinsic() != Type::Intrinsic::ERROR
        ) {
            Reporting::typeError(
                node->position(),
                "Condition for while loop not boolean " + condType->toString() + "."
            );

            flag = false;
        }

        _whileCount++;
        for ( auto stmt : *node->body() ) {
            flag = walk(stmt) && flag;
        }
        _whileCount--;

        auto type = flag ? Type::Primitive::of(Type::Intrinsic::UNIT) : Type::Primitive::of(Type::Intrinsic::ERROR);
        _types->setTypeOf(node, type);
        return flag;
    }

    virtual bool walkContinueNode(ContinueNode* node) {
        if ( _whileCount == 0 ) {
            Reporting::syntaxError(
                node->position(),
                "Found continue statement outside of a while statement"
            );
            _types->setTypeOf(node, Type::Primitive::of(Type::Intrinsic::ERROR));
            return false;
        }
        return true;
    }

    virtual bool walkBreakNode(BreakNode* node) {
        if ( _whileCount == 0 ) {
            Reporting::syntaxError(
                node->position(),
                "Found break statement outside of a while statement"
            );
            _types->setTypeOf(node, Type::Primitive::of(Type::Intrinsic::ERROR));
            return false;
        }
        return true;
    }

    virtual bool walkReturnStatementNode(ReturnStatementNode* node) {
        if ( _funcCount == 0 ) {
            Reporting::syntaxError(
                node->position(),
                "Found return statement outside of a function"
            );
            _types->setTypeOf(node, Type::Primitive::of(Type::Intrinsic::ERROR));
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
                    "Invalid return type. Expected: " + funcType->toString() + "; Found: Primitive<VOID>"
                );
                _types->setTypeOf(node, Type::Primitive::of(Type::Intrinsic::ERROR));
                return false;
            }
            _types->setTypeOf(node, Type::Primitive::of(Type::Intrinsic::UNIT));
            return true;
        }

        if ( !walk(node->value()) ) {
            _types->setTypeOf(node, Type::Primitive::of(Type::Intrinsic::ERROR));
            return false;
        }

        auto retType = node->value()->type();

        if ( !retType->isAssignableTo(funcType) ) {
            Reporting::typeError(
                node->position(),
                "Invalid return type. Expected: " + funcType->toString() + "; Found: " + retType->toString()
            );
            _types->setTypeOf(node, Type::Primitive::of(Type::Intrinsic::ERROR));
            return false;
        }
        _types->setTypeOf(node, Type::Primitive::of(Type::Intrinsic::UNIT));
        return true;
    }

    virtual bool walkMapStatementNode(MapStatementNode* node) {
        if ( !walk(node->value()) ) {
            _types->setTypeOf(node, Type::Primitive::of(Type::Intrinsic::ERROR));
            return false;
        }

        _types->setTypeOf(node, _types->getTypeOf(node->value()));
        return true;
    }

    virtual bool walkMapNode(MapNode* node) {
        size_t idx = 0;
        bool hadFirst = false;
        const Type::Type* innerType = nullptr;
        if ( node->_type != nullptr ) {
            assert(node->type()->intrinsic() == Type::Intrinsic::MAP);
            innerType = ((Type::Map*)node->type())->values();
            hadFirst = true;
        }

        bool flag = true;
        for ( auto stmt : *node->body() ) {
            flag = walk(stmt) && flag;

            const Type::Type* stmtType = _types->getTypeOf(stmt);
            if ( !hadFirst ) {
                innerType = stmtType;
                // FIXME: Doesn't accurately type check this statement because type of map is not known
                node->_type = new TypeLiteral(node->position()->copy(), new Type::Map(innerType));
                hadFirst = false;
            } else if ( !stmtType->isAssignableTo(innerType) ) {
                Reporting::typeError(
                    node->position(),
                    "Invalid entry in map at position " + std::to_string(idx) + ". Expected: " + innerType->toString() + "; Found: " + stmtType->toString()
                );
                flag = false;
            }

            idx += 1;
        }

        _types->setTypeOf(node, flag ? node->type() : Type::Primitive::of(Type::Intrinsic::ERROR));
        return flag;
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
        bool flag = walk(node->dest());
        flag = walk(node->value()) && flag;

        const Type::Type* typeOfValue = _types->getTypeOf(node->value());
        const Type::Type* typeOfDest = _types->getTypeOf(node->dest());

        if ( !typeOfValue->isAssignableTo(typeOfDest) 
            && typeOfValue->intrinsic() != Type::Intrinsic::ERROR
            && typeOfDest->intrinsic() != Type::Intrinsic::ERROR
        ) {
            Reporting::typeError(
                node->position(),
                "Attempted to assign value of type " + typeOfValue->toString() + " to lval of type " + typeOfDest->toString() + "."
            );

            flag = false;
        }

        auto type = flag ? Type::Primitive::of(Type::Intrinsic::UNIT) : Type::Primitive::of(Type::Intrinsic::ERROR);
        _types->setTypeOf(node, type);
        return flag;
    }

    virtual bool walkUnitNode(UnitNode* node) {
        _types->setTypeOf(node, Type::Primitive::of(Type::Intrinsic::UNIT));
        return true;
    }

    virtual bool walkFunctionNode(FunctionNode* node) {
        _funcTypes->push(node->type());
        _funcArgs->push(node->formals()->size());
        _funcCount++;
        bool flag = true;
        for ( auto stmt : *node->body() ) {
            flag = walk(stmt) && flag;
        }
        _funcTypes->pop();
        _funcArgs->pop();
        _funcCount--;

        _types->setTypeOf(node, flag ? node->type() : Type::Primitive::of(Type::Intrinsic::ERROR));
        return flag;
    }

    virtual bool walkNumericComparisonExpressionNode(NumericComparisonExpressionNode* node) {
        return walkPureBinaryExpression(node);
    }

    virtual std::string toString() const {
        return "TypeAnalysisWalk<>";
    }
private:
    int _whileCount, _funcCount;
    TypeTable* _types;
    std::stack<const Type::Type*>* _funcTypes;
    std::stack<int>* _funcArgs;
};

}
}
}

#endif