#include "TypeAnalysisWalk.h"
#include "../TypeTable.h"
#include "ValidConstructorWalk.h"
#include "ASTMapReduce.h"
#include <stack>

namespace swarmc::Lang::Walk {

TypeAnalysisWalk::TypeAnalysisWalk() : Walk<bool>("Type Analysis"), _whileCount(0), 
    _funcCount(0), _types(new TypeTable()), _funcTypes(new std::stack<const Type::Type*>()), 
    _funcArgs(new std::stack<size_t>()) {}

TypeAnalysisWalk::~TypeAnalysisWalk() {
    delete _types;
    delete _funcTypes;
    delete _funcArgs;
}

bool TypeAnalysisWalk::walkProgramNode(ProgramNode* node) {
    bool flag = true;
    for ( auto stmt : *node->body() ) {
        flag = walk(stmt) && flag;
        logger->debug(s(stmt->position()) + " Finished: " + s(stmt));
    }

    auto type = flag ? Type::Primitive::of(Type::Intrinsic::UNIT) : Type::Primitive::of(Type::Intrinsic::ERROR);
    _types->setTypeOf(node, type);
    return flag;
}

bool TypeAnalysisWalk::walkExpressionStatementNode(ExpressionStatementNode* node) {
    bool flag = walk(node->expression());

    auto type = flag ? Type::Primitive::of(Type::Intrinsic::UNIT) : Type::Primitive::of(Type::Intrinsic::ERROR);
    _types->setTypeOf(node, type);

    return flag;
}

bool TypeAnalysisWalk::walkIdentifierNode(IdentifierNode* node) {
    if ( node->_symbol->type() == nullptr ) {
        logger->error(
            s(node->position()) +
            " Invalid type of free identifier: " + node->name()
        );

        _types->setTypeOf(node, Type::Primitive::of(Type::Intrinsic::ERROR));
        return false;
    }

    _types->setTypeOf(node, node->_symbol->type());
    return true;
}

bool TypeAnalysisWalk::walkEnumerableAppendNode(EnumerableAppendNode* node) {
    bool pathresult = walk(node->path());
    if ( !pathresult ) {
        _types->setTypeOf(node, Type::Primitive::of(Type::Intrinsic::ERROR));
        return false;
    }

    const Type::Type* typeLVal = _types->getTypeOf(node->path());
    if ( typeLVal->intrinsic() != Type::Intrinsic::ENUMERABLE ) {
        logger->error(
            s(node->position()) +
            " Invalid array access: " + s(node->path())
        );
        _types->setTypeOf(node, Type::Primitive::of(Type::Intrinsic::ERROR));
        return false;
    }

    _types->setTypeOf(node, ((Type::Enumerable*) typeLVal)->values());
    return true;
}

bool TypeAnalysisWalk::walkEnumerableAccessNode(EnumerableAccessNode* node) {
    bool pathresult = walk(node->path());
    if ( !pathresult ) {
        _types->setTypeOf(node, Type::Primitive::of(Type::Intrinsic::ERROR));
        return false;
    }

    const Type::Type* typeLVal = _types->getTypeOf(node->path());
    if ( typeLVal->intrinsic() != Type::Intrinsic::ENUMERABLE ) {
        logger->error(
            s(node->position()) +
            " Invalid array access: " + s(node->path())
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
        logger->error(
            s(node->position()) +
            " Invalid index type: " + s(node->index()->type())
        );
        _types->setTypeOf(node, Type::Primitive::of(Type::Intrinsic::ERROR));
        return false;
    }

    _types->setTypeOf(node, ((Type::Enumerable*) typeLVal)->values());
    return true;
}

bool TypeAnalysisWalk::walkMapAccessNode(MapAccessNode* node) {
    bool pathresult = walk(node->path());
    if ( !pathresult ) {
        _types->setTypeOf(node, Type::Primitive::of(Type::Intrinsic::ERROR));
        return false;
    }

    const Type::Type* typeLVal = _types->getTypeOf(node->path());
    if ( typeLVal->intrinsic() != Type::Intrinsic::MAP ) {
        logger->error(
            s(node->position()) +
            " Invalid map access: " + node->end()->name()
        );
        _types->setTypeOf(node, Type::Primitive::of(Type::Intrinsic::ERROR));
        return false;
    }

    _types->setTypeOf(node, ((Type::Map*) typeLVal)->values());
    return true;
}

bool TypeAnalysisWalk::walkClassAccessNode(ClassAccessNode* node) {
    bool pathresult = walk(node->path());
    if ( !pathresult ) {
        _types->setTypeOf(node, Type::Primitive::of(Type::Intrinsic::ERROR));
        return false;
    }

    const Type::Type* parsedPathType = _types->getTypeOf(node->path());
    const Type::Type* actualPathType;
    if ( parsedPathType->intrinsic() == Type::Intrinsic::AMBIGUOUS && ((Type::Ambiguous*)parsedPathType)->id() != nullptr ) {
        auto id = ((Type::Ambiguous*)parsedPathType)->id();
        assert(id->symbol() != nullptr);
        auto t = id->symbol()->ensureVariable()->getObjectType();

        assert(t->getTag() == ASTNodeTag::TYPELITERAL || t->getTag() == ASTNodeTag::TYPEBODY);
        actualPathType = ((TypeLiteral*)t)->value();
    } else {
        actualPathType = parsedPathType;
    }

    if ( actualPathType->intrinsic() != Type::Intrinsic::OBJECT ) {
        logger->error(
            s(node->position()) +
            " Attempt to access property of variable of type " + Type::Type::intrinsicString(actualPathType->intrinsic())
            + ": " + node->end()->name()
        );
        _types->setTypeOf(node, Type::Primitive::of(Type::Intrinsic::ERROR));
        return false;
    }

    auto endType = ((Type::Object*) actualPathType)->getProperty(node->end()->name());
    if ( endType == nullptr ) {
        logger->error(
            s(node->end()->position()) + " " +
            node->end()->name() + " is not a member of type " + s(actualPathType)
        );
        _types->setTypeOf(node, Type::Primitive::of(Type::Intrinsic::ERROR));
        return false;
    }

    _types->setTypeOf(node, endType);
    return true;
}

bool TypeAnalysisWalk::walkIncludeStatementNode(IncludeStatementNode* node) {
    return true; // these should be removed by this point, should throw
}

bool TypeAnalysisWalk::walkTypeLiteral(swarmc::Lang::TypeLiteral *node) {
    _types->setTypeOf(node, Type::Primitive::of(Type::Intrinsic::TYPE));
    return true;
}

bool TypeAnalysisWalk::walkBooleanLiteralExpressionNode(BooleanLiteralExpressionNode* node) {
    _types->setTypeOf(node, Type::Primitive::of(Type::Intrinsic::BOOLEAN));
    return true;
}

bool TypeAnalysisWalk::walkStringLiteralExpressionNode(StringLiteralExpressionNode* node) {
    _types->setTypeOf(node, Type::Primitive::of(Type::Intrinsic::STRING));
    return true;
}

bool TypeAnalysisWalk::walkNumberLiteralExpressionNode(NumberLiteralExpressionNode* node) {
    _types->setTypeOf(node, Type::Primitive::of(Type::Intrinsic::NUMBER));
    return true;
}

bool TypeAnalysisWalk::walkEnumerationLiteralExpressionNode(EnumerationLiteralExpressionNode* node) {
    size_t idx = 0;
    bool hadFirst = false;
    Type::Type* innerType = nullptr;
    if (node->_type != nullptr) {
        assert(node->type()->intrinsic() == Type::Intrinsic::ENUMERABLE);
        innerType = ((Type::Enumerable*)node->type())->values();
        hadFirst = true;
    }

    bool flag = true;
    for ( auto exp : *node->actuals() ) {
        flag = walk(exp) && flag;

        Type::Type* expType = _types->getTypeOf(exp);
        if ( !hadFirst ) {
            innerType = expType;
            node->_type = useref(new TypeLiteral(node->position(), new Type::Enumerable(innerType)));
            hadFirst = false;
        } else if ( !expType->isAssignableTo(innerType) ) {
            logger->error(
                s(node->position()) +
                " Invalid entry in enumerable at position " + s(idx) + ". Expected: " + s(innerType) + "; Found: " + s(expType)
            );
            flag = false;
        }

        idx += 1;
    }

    _types->setTypeOf(node, flag ? node->type() : Type::Primitive::of(Type::Intrinsic::ERROR));
    return flag;
}

bool TypeAnalysisWalk::walkMapStatementNode(MapStatementNode* node) {
    if ( !walk(node->value()) ) {
        _types->setTypeOf(node, Type::Primitive::of(Type::Intrinsic::ERROR));
        return false;
    }

    _types->setTypeOf(node, _types->getTypeOf(node->value()));
    return true;
}

bool TypeAnalysisWalk::walkMapNode(MapNode* node) {
    size_t idx = 0;
    bool hadFirst = false;
    Type::Type* innerType = nullptr;
    if ( node->_type != nullptr ) {
        assert(node->type()->intrinsic() == Type::Intrinsic::MAP);
        innerType = ((Type::Map*)node->type())->values();
        hadFirst = true;
    }

    bool flag = true;
    for ( auto stmt : *node->body() ) {
        flag = walk(stmt) && flag;

        Type::Type* stmtType = _types->getTypeOf(stmt);
        if ( !hadFirst ) {
            innerType = stmtType;
            // FIXME: Doesn't accurately type check this statement because type of map is not known
            node->_type = useref(new TypeLiteral(node->position(), new Type::Map(innerType)));
            hadFirst = false;
        } else if ( !stmtType->isAssignableTo(innerType) ) {
            logger->error(
                s(node->position()) +
                " Invalid entry in map at position " + s(idx) + ". Expected: " + s(innerType) + "; Found: " + s(stmtType)
            );
            flag = false;
        }

        idx += 1;
    }

    _types->setTypeOf(node, flag ? node->type() : Type::Primitive::of(Type::Intrinsic::ERROR));
    return flag;
}

bool TypeAnalysisWalk::walkAssignExpressionNode(AssignExpressionNode* node) {
    bool flag = walk(node->dest());
    flag = walk(node->value()) && flag;

    const Type::Type* typeOfValue = _types->getTypeOf(node->value());
    const Type::Type* typeOfDest = _types->getTypeOf(node->dest());

    if ( typeOfDest->intrinsic() == Type::Intrinsic::TYPE ) {
        logger->error(
            s(node->position()) +
            " Attempted to reassign variable of type " + Type::Type::intrinsicString(typeOfDest->intrinsic())
        );
        flag = false;
    } else if ( !typeOfValue->isAssignableTo(typeOfDest)
        && typeOfValue->intrinsic() != Type::Intrinsic::ERROR
        && typeOfDest->intrinsic() != Type::Intrinsic::ERROR
    ) {
        logger->error(
            s(node->position()) +
            " Attempted to assign value of type " + s(typeOfValue) + " to lval of type " + s(typeOfDest) + "."
        );

        flag = false;
    }

    auto type = flag ? Type::Primitive::of(Type::Intrinsic::UNIT) : Type::Primitive::of(Type::Intrinsic::ERROR);
    _types->setTypeOf(node, type);
    return flag;
}

bool TypeAnalysisWalk::walkVariableDeclarationNode(VariableDeclarationNode* node) {
    bool flag = walk(node->typeNode());
    flag = walk(node->id()) && flag;
    flag = walk(node->value()) && flag;

    const Type::Type* typeOfValue = _types->getTypeOf(node->value());
    if ( !typeOfValue->isAssignableTo(node->id()->type())
        && typeOfValue->intrinsic() != Type::Intrinsic::ERROR
    ) {

        logger->error(
            s(node->position()) +
            " Attempted to initialize identifier of type " + s(node->id()->type()) + " with value of type " + s(typeOfValue) + "."
        );

        flag = false;
    }

    auto type = flag ? Type::Primitive::of(Type::Intrinsic::UNIT) : Type::Primitive::of(Type::Intrinsic::ERROR);
    _types->setTypeOf(node, type);
    return flag;
}

bool TypeAnalysisWalk::walkUninitializedVariableDeclarationNode(UninitializedVariableDeclarationNode* node) {
    bool flag = walk(node->typeNode());
    flag = walk(node->id()) && flag;

    auto type = flag ? Type::Primitive::of(Type::Intrinsic::UNIT) : Type::Primitive::of(Type::Intrinsic::ERROR);
    _types->setTypeOf(node, type);
    return flag;
}

bool TypeAnalysisWalk::walkUseNode(UseNode* node) {
    throw Errors::SwarmError("Unresolved UseNode in Type Analysis");
}

bool TypeAnalysisWalk::walkReturnStatementNode(ReturnStatementNode* node) {
    if ( _funcCount == 0 ) {
        Logging::get()->get("Syntax")->error(
            s(node->position()) +
            " Found return statement outside of a function"
        );
        _types->setTypeOf(node, Type::Primitive::of(Type::Intrinsic::ERROR));
        return false;
    }

    const Type::Type* funcType = _funcTypes->top();

    if ( funcType->intrinsic() == Type::Intrinsic::LAMBDA0 ) {
        funcType = ((Type::Lambda*) funcType)->returns();
    } else {
        for ( size_t i = 0; i < _funcArgs->top(); i++ ) {
            assert(funcType->intrinsic() == Type::Intrinsic::LAMBDA0 || funcType->intrinsic() == Type::Intrinsic::LAMBDA1);
            funcType = ((Type::Lambda*) funcType)->returns();
        }
    }

    if ( node->value() == nullptr ) {
        if ( funcType->intrinsic() != Type::Intrinsic::VOID ) {
            logger->error(
                s(node->position()) +
                " Invalid return type. Expected: " + s(funcType) + "; Found: Primitive<VOID>"
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
        logger->error(
            s(node->position()) +
            " Invalid return type. Expected: " + s(funcType) + "; Found: " + s(retType)
        );
        _types->setTypeOf(node, Type::Primitive::of(Type::Intrinsic::ERROR));
        return false;
    }
    _types->setTypeOf(node, Type::Primitive::of(Type::Intrinsic::UNIT));
    return true;
}

bool TypeAnalysisWalk::walkFunctionNode(FunctionNode* node) {
    bool flag = walk(node->typeNode());
    _funcTypes->push(node->type());
    _funcArgs->push(node->formals()->size());
    _funcCount++;
    auto temp = _whileCount;
    _whileCount = 0;
    for ( auto stmt : *node->body() ) {
        flag = walk(stmt) && flag;
    }
    _funcTypes->pop();
    _funcArgs->pop();
    _funcCount--;
    _whileCount = temp;

    // for modifying the type later, to preserve the static scoping scheme
    auto syms = unscopedLocations()->walkStatementList(node);
    if ( syms.has_value() ) {
        node->setUsedSymbols(syms.value(), true);
    }

    _types->setTypeOf(node, flag ? node->type() : Type::Primitive::of(Type::Intrinsic::ERROR));
    return flag;
}

bool TypeAnalysisWalk::walkConstructorNode(ConstructorNode* node) {
    bool flag = walk(node->func());

    for ( auto pc : *node->parentConstructors() ) {
        flag = walk(pc) && flag;
        assert(pc->getTag() == ASTNodeTag::CALL);
        for ( auto arg : *((CallExpressionNode*)pc)->args() ) {
            auto syms = unscopedLocations()->walk(arg);
            if ( syms.has_value() && syms.value().second.size() > 0 ) {
                node->func()->setUsedSymbols(syms.value(), false);
            }
        }
    }

    _types->setTypeOf(node, flag ? Type::Primitive::of(Type::Intrinsic::UNIT) : Type::Primitive::of(Type::Intrinsic::ERROR));
    return flag;
}

bool TypeAnalysisWalk::walkTypeBodyNode(TypeBodyNode* node) {
    bool flag = true;
    for (auto d : *node->declarations()) {
        flag = walk(d) && flag;
    }
    std::vector<Type::Type*> constructorTypes;
    for (auto c : *node->constructors()) {
        flag = walk(c) && flag;
        flag = ValidConstructorWalk::isValidConstructor(node, c) && flag;
        auto type = _types->getTypeOf(c);
        if ( type->intrinsic() != Type::Intrinsic::ERROR ) {
            for ( auto t : constructorTypes ) {
                if ( type->isAssignableTo(t) ) {
                    logger->warn(s(c->position()) + " Constructors with duplicate type signatures will be removed in compilation.");
                }
            }
            constructorTypes.push_back(type);
        }
    }
    if ( flag ) {
        _types->setTypeOf(node, Type::Primitive::of(Type::Intrinsic::TYPE));
    } else {
        _types->setTypeOf(node, Type::Primitive::of(Type::Intrinsic::ERROR));
    }
    return flag;
}

bool TypeAnalysisWalk::walkCallExpressionNode(CallExpressionNode* node) {
    // Perform type analysis on the callable
    bool flag = walk(node->func());

    // Perform type analysis on the arguments
    for ( auto arg : *node->args() ) {
        flag = walk(arg) && flag;
    }

    // Make sure the callee is actually a function type
    const Type::Type* baseTypeOfCallee = _types->getTypeOf(node->func());
    if ( !baseTypeOfCallee->isCallable() ) {
        if ( node->func()->getTag() == ASTNodeTag::IDENTIFIER && baseTypeOfCallee->intrinsic() == Type::Intrinsic::TYPE ) {
            auto sym = ((IdentifierNode*)node->func())->symbol()->ensureVariable();
            if ( sym->getObjectType() == nullptr ) {
                logger->error(
                    s(node->position()) +
                    " Attempted to create object instance of type " + ((IdentifierNode*)node->func())->name() + "."
                );

                _types->setTypeOf(node, Type::Primitive::of(Type::Intrinsic::ERROR));
                node->_type = useref(Type::Primitive::of(Type::Intrinsic::ERROR));
                return false;
            }

            if ( sym->getObjectType()->getTag() != ASTNodeTag::TYPEBODY ) {
                logger->error(
                    s(node->position()) +
                    " Attempted to create object of type " + s(sym->getObjectType()->value()) + "."
                );

                _types->setTypeOf(node, Type::Primitive::of(Type::Intrinsic::ERROR));
                node->_type = useref(Type::Primitive::of(Type::Intrinsic::ERROR));
                return false;
            }
            auto tLit = (TypeBodyNode*)sym->getObjectType();

            std::vector<const Type::Type*> paramtypes;
            std::string typeString;
            for (size_t i = 0; i < node->args()->size(); i++) {
                paramtypes.push_back(_types->getTypeOf(node->args()->at(i)));
                if (typeString != "") typeString += " -> ";
                typeString += s(paramtypes.back());
            }
            typeString += " -> " + s(tLit->value());

            ConstructorNode* theconstructor = nullptr;
            for (auto c : *((TypeBodyNode*)tLit)->constructors()) {
                // no partial applications of constructors
                if (paramtypes.size() != c->func()->formals()->size()) continue;
                for (size_t i = 0; i < paramtypes.size(); i++) {
                    if (!paramtypes.at(i)->isAssignableTo(c->func()->formals()->at(i).first->value())) continue;
                }
                theconstructor = c;
                break;
            }

            if (theconstructor == nullptr) {
                logger->error(
                    s(node->position()) +
                    " No " + ((IdentifierNode*)node->func())->name() + " constructor matches the arguments: " + typeString
                );

                _types->setTypeOf(node, Type::Primitive::of(Type::Intrinsic::ERROR));
                node->_type = useref(Type::Primitive::of(Type::Intrinsic::ERROR));
                return false;
            }

            if (flag) {
                _types->setTypeOf(node, theconstructor->partOf());
                node->_type = useref(theconstructor->partOf());
            } else {
                _types->setTypeOf(node, Type::Primitive::of(Type::Intrinsic::ERROR));
                node->_type = useref(Type::Primitive::of(Type::Intrinsic::ERROR));
            }
            node->_constructor = useref(theconstructor);
            return flag;

        } else {
            logger->error(
                s(node->position()) +
                " Attempted to call non-callable type " + s(baseTypeOfCallee) + "."
            );

            _types->setTypeOf(node, Type::Primitive::of(Type::Intrinsic::ERROR));
            node->_type = useref(Type::Primitive::of(Type::Intrinsic::ERROR));
            return false;
        }
    }

    Type::Lambda* typeOfCallee = (Type::Lambda*) baseTypeOfCallee;
    auto argTypes = typeOfCallee->params();

    // Make sure the # of arguments matches
    if ( argTypes.size() < node->args()->size() || (node->args()->size() == 0 && typeOfCallee->intrinsic() == Type::Intrinsic::LAMBDA1) ) {
        logger->error(
            s(node->position()) +
            " Invalid number of arguments for call (expected: " + s(argTypes.size()) + ")."
        );

        _types->setTypeOf(node, Type::Primitive::of(Type::Intrinsic::ERROR));
        node->_type = useref(Type::Primitive::of(Type::Intrinsic::ERROR));
        return false;
    }

    Type::Type* nodeType = typeOfCallee;

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
            logger->error(
                s(node->position()) +
                " Invalid argument of type " + s(actualType) + " in position " + s(i) + " (expected: " + s(expectedType) + ")."
            );

            flag = false;
        }

        assert(nodeType->isCallable());
        nodeType = ((Type::Lambda*) nodeType)->returns();
    }

    _types->setTypeOf(node, flag ? nodeType : Type::Primitive::of(Type::Intrinsic::ERROR));
    node->_type = useref(flag ? nodeType : Type::Primitive::of(Type::Intrinsic::ERROR));
    return flag;
}

bool TypeAnalysisWalk::walkDeferCallExpressionNode(DeferCallExpressionNode* node) {
    auto flag = walkCallExpressionNode(node->call());
    auto callType = _types->getTypeOf(node->call());
    _types->setTypeOf(node, callType);
    return flag;
}

bool TypeAnalysisWalk::walkAndNode(AndNode* node) {
    return walkPureBinaryExpression(node);
}

bool TypeAnalysisWalk::walkOrNode(OrNode* node) {
    return walkPureBinaryExpression(node);
}

bool TypeAnalysisWalk::walkEqualsNode(EqualsNode* node) {
    bool flag = walk(node->left());
    flag = walk(node->right()) && flag;

    const Type::Type* actualLeftType = _types->getTypeOf(node->left());
    const Type::Type* actualRightType = _types->getTypeOf(node->right());
    if ( !actualLeftType->isAssignableTo(actualRightType)
        && actualLeftType->intrinsic() != Type::Intrinsic::ERROR
        && actualRightType->intrinsic() != Type::Intrinsic::ERROR
    ) {
        logger->error(
            s(node->position()) +
            " Invalid comparison between left-hand type " + s(actualLeftType) + " and right-hand type " + s(actualRightType) + "."
        );

        flag = false;
    }

    auto type = flag ? Type::Primitive::of(Type::Intrinsic::BOOLEAN) : Type::Primitive::of(Type::Intrinsic::ERROR);
    _types->setTypeOf(node, type);
    return flag;
}

bool TypeAnalysisWalk::walkNumericComparisonExpressionNode(NumericComparisonExpressionNode* node) {
    return walkPureBinaryExpression(node);
}

bool TypeAnalysisWalk::walkNotEqualsNode(NotEqualsNode* node) {
    bool flag = walk(node->left());
    flag = walk(node->right()) && flag;

    const Type::Type* actualLeftType = _types->getTypeOf(node->left());
    const Type::Type* actualRightType = _types->getTypeOf(node->right());
    if ( !actualLeftType->isAssignableTo(actualRightType)
        && actualLeftType->intrinsic() != Type::Intrinsic::ERROR
        && actualRightType->intrinsic() != Type::Intrinsic::ERROR
    ) {
        logger->error(
            s(node->position()) +
            " Invalid comparison between left-hand type " + s(actualLeftType) + " and right-hand type " + s(actualRightType) + "."
        );

        flag = false;
    }

    auto type = flag ? Type::Primitive::of(Type::Intrinsic::BOOLEAN) : Type::Primitive::of(Type::Intrinsic::ERROR);
    _types->setTypeOf(node, type);
    return flag;
}

bool TypeAnalysisWalk::walkAddNode(AddNode* node) {
    bool flag = walk(node->left());
    flag = walk(node->right()) && flag;

    const Type::Type* actualLeftType = _types->getTypeOf(node->left());
    const Type::Type* actualRightType = _types->getTypeOf(node->right());

    if ( actualLeftType->isAssignableTo(Type::Primitive::of(Type::Intrinsic::NUMBER)) ) {
        if ( !actualRightType->isAssignableTo(Type::Primitive::of(Type::Intrinsic::NUMBER)) && actualRightType->intrinsic() != Type::Intrinsic::ERROR ) {
            logger->error(
                s(node->position()) +
                " Invalid type " + s(actualRightType) + " of right-hand operand to expression (expected: Primitive<NUMBER>)."
            );

            flag = false;
        }
    } else if ( actualLeftType->isAssignableTo(Type::Primitive::of(Type::Intrinsic::STRING)) ) {
        if ( !actualRightType->isAssignableTo(Type::Primitive::of(Type::Intrinsic::STRING)) && actualRightType->intrinsic() != Type::Intrinsic::ERROR ) {
            logger->error(
                s(node->position()) +
                " Invalid type " + s(actualRightType) + " of right-hand operand to expression (expected: Primitive<STRING>)."
            );

            flag = false;
        }
        node->setConcat(true);
    } else if ( actualLeftType->intrinsic() != Type::Intrinsic::ERROR ) {
        logger->error(
            s(node->position()) +
            " Invalid type " + s(actualLeftType) + " of left-hand operand to expression (expected: Primitive<NUMBER> or Primitive<STRING>)."
        );

        flag = false;
    }

    _types->setTypeOf(node, flag ? node->resultType() : Type::Primitive::of(Type::Intrinsic::ERROR));
    return flag;
}

bool TypeAnalysisWalk::walkSubtractNode(SubtractNode* node) {
    return walkPureBinaryExpression(node);
}

bool TypeAnalysisWalk::walkMultiplyNode(MultiplyNode* node) {
    return walkPureBinaryExpression(node);
}

bool TypeAnalysisWalk::walkDivideNode(DivideNode* node) {
    return walkPureBinaryExpression(node);
}

bool TypeAnalysisWalk::walkModulusNode(ModulusNode* node) {
    return walkPureBinaryExpression(node);
}

bool TypeAnalysisWalk::walkPowerNode(PowerNode* node) {
    return walkPureBinaryExpression(node);
}

bool TypeAnalysisWalk::walkNthRootNode(NthRootNode* node) {
    return walkPureBinaryExpression(node);
}

bool TypeAnalysisWalk::walkNegativeExpressionNode(NegativeExpressionNode* node) {
    bool flag = walk(node->exp());

    Type::Type* numType = Type::Primitive::of(Type::Intrinsic::NUMBER);
    Type::Type* expType = _types->getTypeOf(node->exp());
    if ( !expType->isAssignableTo(numType) && expType->intrinsic() != Type::Intrinsic::ERROR ) {
        logger->error(
            s(node->position()) +
            " Attempted to perform numeric negation on invalid type. Expected: " + s(numType) + "; actual: " + s(expType)
        );
        flag = false;
    }

    _types->setTypeOf(node, flag ? numType : Type::Primitive::of(Type::Intrinsic::ERROR));
    return flag;
}

bool TypeAnalysisWalk::walkNotNode(NotNode* node) {
    bool flag = walk(node->exp());

    Type::Type* boolType = Type::Primitive::of(Type::Intrinsic::BOOLEAN);
    Type::Type* expType = _types->getTypeOf(node->exp());
    if ( !expType->isAssignableTo(boolType) && expType->intrinsic() != Type::Intrinsic::ERROR ) {
        logger->error(
            s(node->position()) +
            " Attempted to perform boolean negation on invalid type. Expected: " + s(boolType) + "; actual: " + s(expType)
        );
        flag = false;
    }

    _types->setTypeOf(node, flag ? boolType : Type::Primitive::of(Type::Intrinsic::ERROR));
    return flag;
}

bool TypeAnalysisWalk::walkEnumerationStatement(EnumerationStatement* node) {
    bool flag = walk(node->enumerable());

    const Type::Type* enumType = node->enumerable()->type();
    if ( enumType->intrinsic() != Type::Intrinsic::ENUMERABLE &&
        enumType->intrinsic() != Type::Intrinsic::ERROR ) {
        logger->error(
            s(node->position()) +
            " Attempted to enumerate invalid value"
        );
        flag = false;
    }

    auto genericType = (Type::Enumerable*) enumType;
    auto concreteType = genericType->values();
    node->local()->_symbol->_type = useref(concreteType);
    _types->setTypeOf(node->local(), concreteType);

    int tempf = _funcCount;
    _funcCount = 0;
    int tempw = _whileCount;
    _whileCount = 0;
    for ( auto stmt : *node->body() ) {
        flag = walk(stmt) && flag;
    }
    _funcCount = tempf;
    _whileCount = tempw;

    auto type = flag ? Type::Primitive::of(Type::Intrinsic::UNIT) : Type::Primitive::of(Type::Intrinsic::ERROR);
    _types->setTypeOf(node, type);
    return flag;
}

bool TypeAnalysisWalk::walkWithStatement(WithStatement* node) {
    bool flag = walk(node->resource());
    Type::Type* localType = nullptr;

    Type::Type* type = _types->getTypeOf(node->resource());
    if (
        type == nullptr ||
        ( type->intrinsic() != Type::Intrinsic::RESOURCE && type->intrinsic() != Type::Intrinsic::ERROR )
    ) {
        logger->error(
            s(node->position()) +
            " Expected Intrinsic::RESOURCE, found: " + (type == nullptr ? "none" : s(type))
        );

        flag = false;
        localType = Type::Primitive::of(Type::Intrinsic::ERROR);
    } else {
        localType = ((Type::Resource*) type)->yields();

        bool lt = localType->intrinsic() != Type::Intrinsic::OPAQUE && localType->intrinsic() != Type::Intrinsic::ERROR;

        if ( lt ) {
            logger->error(
                s(node->local()->position()) +
                " Resource does not contain an opaque type."
            );
            flag = false;
            localType = Type::Primitive::of(Type::Intrinsic::ERROR);
        }
    }

    _types->setTypeOf(node->local(), type);
    if ( localType->intrinsic() != Type::Intrinsic::ERROR ) {
        node->local()->symbol()->_type = useref(type);  // local is implicitly defined, so need to set its type
    }

    for ( auto stmt : *node->body() ) {
        flag = walk(stmt) && flag;
    }

    auto t = flag ? Type::Primitive::of(Type::Intrinsic::UNIT) : Type::Primitive::of(Type::Intrinsic::ERROR);
    _types->setTypeOf(node, t);
    return flag;
}

bool TypeAnalysisWalk::walkIfStatement(IfStatement* node) {
    bool flag = walk(node->condition());

    const Type::Type* condType = _types->getTypeOf(node->condition());
    if ( !condType->isAssignableTo(Type::Primitive::of(Type::Intrinsic::BOOLEAN))
        && condType->intrinsic() != Type::Intrinsic::ERROR
    ) {
        logger->error(
            s(node->position()) +
            " Condition for if statement not boolean " + s(condType) + "."
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

bool TypeAnalysisWalk::walkWhileStatement(WhileStatement* node) {
    bool flag = walk(node->condition());

    const Type::Type* condType = _types->getTypeOf(node->condition());
    if ( !condType->isAssignableTo(Type::Primitive::of(Type::Intrinsic::BOOLEAN))
        && condType->intrinsic() != Type::Intrinsic::ERROR
    ) {
        logger->error(
            s(node->position()) +
            " Condition for while loop not boolean " + s(condType) + "."
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

bool TypeAnalysisWalk::walkContinueNode(ContinueNode* node) {
    if ( _whileCount == 0 ) {
        Logging::get()->get("Syntax")->error(
            s(node->position()) +
            " Found continue statement outside of a while statement"
        );
        _types->setTypeOf(node, Type::Primitive::of(Type::Intrinsic::ERROR));
        return false;
    }
    _types->setTypeOf(node, Type::Primitive::of(Type::Intrinsic::UNIT));
    return true;
}

bool TypeAnalysisWalk::walkBreakNode(BreakNode* node) {
    if ( _whileCount == 0 ) {
        Logging::get()->get("Syntax")->error(
            s(node->position()) +
            " Found break statement outside of a while statement"
        );
        _types->setTypeOf(node, Type::Primitive::of(Type::Intrinsic::ERROR));
        return false;
    }
    _types->setTypeOf(node, Type::Primitive::of(Type::Intrinsic::UNIT));
    return true;
}

bool TypeAnalysisWalk::walkPureBinaryExpression(PureBinaryExpressionNode* node) {
    bool flag = walk(node->left());
    flag = walk(node->right()) && flag;

    const Type::Type* actualLeftType = _types->getTypeOf(node->left());
    const Type::Type* actualRightType = _types->getTypeOf(node->right());

    if ( !node->leftType()->isAssignableTo(actualLeftType) && actualLeftType->intrinsic() != Type::Intrinsic::ERROR ) {
        logger->error(
            s(node->position()) +
            " Invalid type " + s(actualLeftType) + " of left-hand operand to expression (expected: " + s(node->leftType()) + ")."
        );

        flag = false;
    }

    if ( !node->rightType()->isAssignableTo(actualRightType) && actualRightType->intrinsic() != Type::Intrinsic::ERROR ) {
        logger->error(
            s(node->position()) +
            " Invalid type " + s(actualRightType) + " of right-hand operand to expression (expected: " + s(node->rightType()) + ")."
        );

        flag = false;
    }

    _types->setTypeOf(node, flag ? node->resultType() : Type::Primitive::of(Type::Intrinsic::ERROR));
    return flag;
}

}