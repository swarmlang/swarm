#include "NameAnalysisWalk.h"
#include "../SymbolTable.h"

namespace swarmc::Lang::Walk {

NameAnalysisWalk::NameAnalysisWalk() : Walk<bool>("Name Analysis"), _symbols(new SymbolTable()) {}

NameAnalysisWalk::~NameAnalysisWalk() {
    delete _symbols;
}

bool NameAnalysisWalk::walkProgramNode(ProgramNode* node) {
    bool flag = true;
    // Enter the global scope
    _symbols->enter();

    for ( auto stmt : *node->body() ) {
        flag = walk(stmt) && flag;
        logger->debug(s(stmt->position()) + " Finished " + s(stmt));
    }

    // Exit the global scope
    _symbols->leave();
    return flag;
}

bool NameAnalysisWalk::walkExpressionStatementNode(ExpressionStatementNode* node) {
    return walk(node->expression());
}

bool NameAnalysisWalk::walkIdentifierNode(IdentifierNode* node) {
    if ( node->_symbol != nullptr ) {
        return true;
    }

    std::string name = node->name();
    node->_symbol = useref(_symbols->lookup(name));

    if ( node->_symbol == nullptr ) {
        logger->error(s(node->position()) + " Use of free identifier \"" + name + "\".");
        return false;
    }

    return true;
}

bool NameAnalysisWalk::walkEnumerableAccessNode(EnumerableAccessNode* node) {
    bool flag = walk(node->path());
    return walk(node->index()) && flag;
}

bool NameAnalysisWalk::walkEnumerableAppendNode(EnumerableAppendNode* node) {
    return walk(node->path());
}

bool NameAnalysisWalk::walkMapAccessNode(MapAccessNode* node) {
    return walk(node->path());
}

bool NameAnalysisWalk::walkClassAccessNode(ClassAccessNode* node) {
    return walk(node->path()); // property names cant be checked until type analysis
}

bool NameAnalysisWalk::walkIncludeStatementNode(IncludeStatementNode* node) {
    return true;
}

bool NameAnalysisWalk::walkTypeLiteral(swarmc::Lang::TypeLiteral *node) {
    return walkType(node->value());
}

bool NameAnalysisWalk::walkBooleanLiteralExpressionNode(BooleanLiteralExpressionNode* node) {
    return true;
}

bool NameAnalysisWalk::walkStringLiteralExpressionNode(StringLiteralExpressionNode* node) {
    return true;
}

bool NameAnalysisWalk::walkNumberLiteralExpressionNode(NumberLiteralExpressionNode* node) {
    return true;
}

bool NameAnalysisWalk::walkEnumerationLiteralExpressionNode(EnumerationLiteralExpressionNode* node) {
    bool flag = true;

    for ( auto actual : *node->actuals() ) {
        flag = walk(actual) && flag;
    }

    return flag;
}

bool NameAnalysisWalk::walkMapStatementNode(MapStatementNode* node) {
    return walk(node->value());
}

bool NameAnalysisWalk::walkMapNode(MapNode* node) {
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
            logger->error(
                s(entry->position()) +
                " Duplicate map key: \"" + entry->id()->name() + "\""
            );

            flag = false;
        }

        // Check the value expression
        flag = walk(entry) && flag;
    }

    return flag;
}

bool NameAnalysisWalk::walkAssignExpressionNode(AssignExpressionNode* node) {
    bool lvalResult = walk(node->dest());
    bool rvalResult = walk(node->value());
    return lvalResult && rvalResult;
}

bool NameAnalysisWalk::walkVariableDeclarationNode(VariableDeclarationNode* node) {
    if ( node->id()->symbol() != nullptr ) {
        return walk(node->value());
    }
    bool flag = walk(node->typeNode());
    flag = node->typeNode()->disambiguateValue() && flag;
    std::string name = node->id()->name();

    Type::Type* type = node->typeNode()->value();

    // Make sure the name isn't already declared in this scope
    if ( _symbols->isClashing(name) ) {
        SemanticSymbol* existing = _symbols->lookup(name);
        logger->error(s(node->position()) + " Redeclaration of identifier \"" + name + "\" first declared at " + existing->declaredAt()->start() + ".");
        return false;
    }

    bool valueResult;
    if ( node->value()->getTag() == ASTNodeTag::FUNCTION || node->typeNode()->value()->intrinsic() == Type::Intrinsic::TYPE ) {
        // Add the declaration to the current scope
        _symbols->addVariable(name, type, node->position(), node->shared(), false);

        if ( node->typeNode()->value()->intrinsic() == Type::Intrinsic::TYPE ) {
            // attach actual value of type to the symbol, for disambiguation of instances of this type
            TypeLiteral* objtype = nullptr;
            if ( node->value()->getTag() == ASTNodeTag::TYPELITERAL || node->value()->getTag() == ASTNodeTag::TYPEBODY ) {
                objtype = (TypeLiteral*)node->value();
            } else if ( node->value()->getTag() == ASTNodeTag::IDENTIFIER ) {
                assert(((IdentifierNode*)node->value())->symbol()->kind() == SemanticSymbolKind::VARIABLE);
                objtype = ((VariableSymbol*)((IdentifierNode*)node->value())->symbol())->getObjectType();
            } else {
                logger->error(
                    s(node->value()->position()) +
                    " Attempt to assign nontrivial value to a type variable."
                );
                return false;
            }
            auto sym = (VariableSymbol*)_symbols->lookup(name);
            logger->debug("Assigned " + s(objtype) + " to symbol " + s(sym));
            sym->setObjectType(objtype);
        }

        // Call this to attach the Symbol to the IdentifierNode
        walk(node->id());
        // Check the RHS of the assignment
        valueResult = walk(node->value());
    } else {
        // Check the RHS of the assignment
        valueResult = walk(node->value());
        // Add the declaration to the current scope
        _symbols->addVariable(name, type, node->position(), node->shared(), node->value()->getTag() == ASTNodeTag::DEFERCALL);
        // Call this to attach the Symbol to the IdentifierNode
        walk(node->id());
    }

    return flag && valueResult;
}

bool NameAnalysisWalk::walkUninitializedVariableDeclarationNode(UninitializedVariableDeclarationNode* node) {
    return true; // symbols should be added to scope during type body
}

bool NameAnalysisWalk::walkReturnStatementNode(ReturnStatementNode* node) {
    if ( node->value() == nullptr ) {
        return true;
    }

    return walk(node->value());
}

bool NameAnalysisWalk::walkFunctionNode(FunctionNode* node) {
    bool flag = true;
    _symbols->enter();
    for ( auto formal : *node->formals() ) {
        bool t = formal.first->disambiguateValue() && flag;
        if ( !t ) {
            logger->error(s(formal.first->position()) + " Identifier at " + formal.first->position()->toString() + " does not refer to a type!");
            flag = false;
        }
        flag = walk(formal.first) && flag;
        std::string name = formal.second->name();
        Type::Type* type = formal.first->value();

        // Make sure the name isn't already declared in this scope
        if ( _symbols->isClashing(name) ) {
            SemanticSymbol* existing = _symbols->lookup(name);
            logger->error(s(node->position()) + " Redeclaration of identifier \"" + name + "\" first declared at " + existing->declaredAt()->start() + ".");
            flag = false;
        }

        // Add the declaration to the current scope
        _symbols->addVariable(name, type, formal.second->position(), false, false);

        // Call this to attach the Symbol to the IdentifierNode
        walk(formal.second);
    }
    walk(node->typeNode());
    flag = node->typeNode()->disambiguateValue() && flag;

    for ( auto stmt : *node->body() ) {
        flag = walk(stmt) && flag;
    }

    _symbols->leave();

    return flag;
}

bool NameAnalysisWalk::walkConstructorNode(ConstructorNode* node) {
    return walk(node->func());
}

bool NameAnalysisWalk::walkTypeBodyNode(TypeBodyNode* node) {
    walkType(node->value());
    node->value()->transform([](Type::Type* v) -> Type::Type* {
        return v->disambiguateStatically();
    });
    _symbols->enter();
    bool flag = true;
    for (auto decl : *node->declarations()) {
        if ( decl->getTag() == ASTNodeTag::VARIABLEDECLARATION ) {
            auto d = (VariableDeclarationNode*)decl;
            if (d->shared()) {
                logger->error(
                    s(d->position()) +
                    " Attempted to create a shared variable " + d->id()->name() + " within type body."
                );
                flag = false;
            }
            flag = walk(d->typeNode()) && flag;
            flag = d->typeNode()->disambiguateValue() && flag;
            _symbols->addObjectProperty(d->id()->name(), d->typeNode()->value(), d->position(), d->value()->getTag() == ASTNodeTag::DEFERCALL);
            walk(d->id());
        } else if ( decl->getTag() == ASTNodeTag::UNINITIALIZEDVARIABLEDECLARATION ) {
            auto d = (UninitializedVariableDeclarationNode*)decl;
            flag = walk(d->typeNode()) && flag;
            flag = d->typeNode()->disambiguateValue() && flag;
            _symbols->addObjectProperty(d->id()->name(), d->typeNode()->value(), d->position(), false);
            walk(d->id());
        }
    }
    for (auto c : *node->declarations()) {
        if (c->getTag() == ASTNodeTag::VARIABLEDECLARATION) {
            flag = walk(((VariableDeclarationNode*)c)->value()) && flag;
        } else {
            flag = walk(c) && flag;
        }
    }
    for (auto c : *node->constructors()) {
        flag = walk(c) && flag;
    }
    _symbols->leave();
    return flag;
}

bool NameAnalysisWalk::walkCallExpressionNode(CallExpressionNode* node) {
    bool flag = walk(node->func());

    for ( auto arg : *node->args() ) {
        flag = walk(arg) && flag;
    }

    return flag;
}

bool NameAnalysisWalk::walkDeferCallExpressionNode(DeferCallExpressionNode* node) {
    return walkCallExpressionNode(node->call());
}

bool NameAnalysisWalk::walkAndNode(AndNode* node) {
    bool leftResult = walk(node->left());
    bool rightResult = walk(node->right());
    return leftResult && rightResult;
}

bool NameAnalysisWalk::walkOrNode(OrNode* node) {
    bool leftResult = walk(node->left());
    bool rightResult = walk(node->right());
    return leftResult && rightResult;
}

bool NameAnalysisWalk::walkEqualsNode(EqualsNode* node) {
    bool leftResult = walk(node->left());
    bool rightResult = walk(node->right());
    return leftResult && rightResult;
}

bool NameAnalysisWalk::walkNumericComparisonExpressionNode(NumericComparisonExpressionNode* node) {
    bool leftResult = walk(node->left());
    bool rightResult = walk(node->right());
    return leftResult && rightResult;
}

bool NameAnalysisWalk::walkNotEqualsNode(NotEqualsNode* node) {
    bool leftResult = walk(node->left());
    bool rightResult = walk(node->right());
    return leftResult && rightResult;
}

bool NameAnalysisWalk::walkAddNode(AddNode* node) {
    bool leftResult = walk(node->left());
    bool rightResult = walk(node->right());
    return leftResult && rightResult;
}

bool NameAnalysisWalk::walkSubtractNode(SubtractNode* node) {
    bool leftResult = walk(node->left());
    bool rightResult = walk(node->right());
    return leftResult && rightResult;
}

bool NameAnalysisWalk::walkMultiplyNode(MultiplyNode* node) {
    bool leftResult = walk(node->left());
    bool rightResult = walk(node->right());
    return leftResult && rightResult;
}

bool NameAnalysisWalk::walkDivideNode(DivideNode* node) {
    bool leftResult = walk(node->left());
    bool rightResult = walk(node->right());
    return leftResult && rightResult;
}

bool NameAnalysisWalk::walkModulusNode(ModulusNode* node) {
    bool leftResult = walk(node->left());
    bool rightResult = walk(node->right());
    return leftResult && rightResult;
}

bool NameAnalysisWalk::walkPowerNode(PowerNode* node) {
    bool leftResult = walk(node->left());
    bool rightResult = walk(node->right());
    return leftResult && rightResult;
}

bool NameAnalysisWalk::walkNegativeExpressionNode(NegativeExpressionNode* node) {
    return walk(node->exp());
}

bool NameAnalysisWalk::walkSqrtNode(SqrtNode* node) {
    return walk(node->exp());
}

bool NameAnalysisWalk::walkNotNode(NotNode* node) {
    return walk(node->exp());
}

bool NameAnalysisWalk::walkEnumerationStatement(EnumerationStatement* node) {
    bool flag = walk(node->enumerable());

    // Need to register the block-local variable
    // Its type is implicit as the generic type of the enumerable,
    // but we don't know it until type analysis
    std::string name = node->local()->name();
    Position* pos = node->local()->position();

    // Start a new scope in the body and add the local
    _symbols->enter();
    _symbols->addVariable(name, nullptr, pos, node->shared(), false);

    // Add the index symbol if it exists. Symbol was created during parsing
    auto i = node->index();
    if (i != nullptr) {
        _symbols->insert(i->symbol());
    }

    flag = walk(node->local()) && flag;
    flag = walkBlockStatementNode(node) && flag;
    _symbols->leave();
    return flag;
}

bool NameAnalysisWalk::walkWithStatement(WithStatement* node) {
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
        _symbols->addVariable(name, type, pos, node->shared(), false);
    }

    flag = walk(node->local()) && flag;
    flag = walkBlockStatementNode(node) && flag;
    if ( inScope ) _symbols->leave();
    return flag;
}

bool NameAnalysisWalk::walkIfStatement(IfStatement* node) {
    bool flag = walk(node->condition());
    _symbols->enter();
    flag = walkBlockStatementNode(node) && flag;
    _symbols->leave();
    return flag;
}

bool NameAnalysisWalk::walkWhileStatement(WhileStatement* node) {
    bool flag = walk(node->condition());
    _symbols->enter();
    flag = walkBlockStatementNode(node) && flag;
    _symbols->leave();
    return flag;
}

bool NameAnalysisWalk::walkContinueNode(ContinueNode* node) {
    return true;
}

bool NameAnalysisWalk::walkBreakNode(BreakNode* node) {
    return true;
}

bool NameAnalysisWalk::walkBlockStatementNode(BlockStatementNode* node) {
    bool flag = true;

    for ( auto stmt : *node->body() ) {
        flag = walk(stmt) && flag;
    }

    return flag;
}

bool NameAnalysisWalk::walkType(Type::Type* type) {
    std::set<std::size_t> visited;
    return walkTypeRec(type, visited);
}

bool NameAnalysisWalk::walkTypeRec(Type::Type* type, std::set<std::size_t>& visited) {
    if ( stl::contains(visited, type->getId()) ) return true;
    visited.insert(type->getId());
    // attach symbol to ambiguous types
    if ( type->isAmbiguous() ) {
        auto a = (Type::Ambiguous*)type;
        assert(a->id() != nullptr);
        return walk(a->id());
    }
    if ( type->intrinsic() == Type::Intrinsic::ENUMERABLE ) {
        return walkTypeRec(((Type::Enumerable*)type)->values(), visited);
    }
    if ( type->intrinsic() == Type::Intrinsic::MAP ) {
        return walkTypeRec(((Type::Map*)type)->values(), visited);
    }
    if ( type->intrinsic() == Type::Intrinsic::LAMBDA0 ) {
        return walkTypeRec(((Type::Lambda0*)type)->returns(), visited);
    }
    if ( type->intrinsic() == Type::Intrinsic::LAMBDA1 ) {
        auto l = (Type::Lambda1*)type;
        return walkTypeRec(l->param(), visited) && walkTypeRec(l->returns(), visited);
    }
    if ( type->intrinsic() == Type::Intrinsic::OBJECT ) {
        auto obj = (Type::Object*)type;
        bool flag = true;
        for ( auto p : obj->getProperties() ) {
            flag = walkTypeRec(p.second, visited) && flag;
        }
        return flag;
    }
    return true;
}

}