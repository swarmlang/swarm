#include "ValidConstructorWalk.h"

namespace swarmc::Lang::Walk {

bool ValidConstructorWalk::isValidConstructor(TypeBodyNode* type, ConstructorNode* constructor) {
    ValidConstructorWalk vcw;
    vcw.logger->debug("Validating constructor " + s(constructor));
    populate(vcw, type, constructor, "");
    vcw.walk(constructor->func());
    if (!vcw._symbols.empty()) {
        std::string ss = "";
        for (auto sym : vcw._symbols) ss += vcw._errNameMap[sym] + ", ";
        vcw.logger->error(
            s(constructor->position()) +
            " Unable to determine value of { " + ss.substr(0, ss.size() - 2) + " } in type constructor."
        );
    }
    return vcw._symbols.empty() && vcw._goodReturns;
}

void ValidConstructorWalk::populate(ValidConstructorWalk& vcw, TypeBodyNode* type, ConstructorNode* constructor, std::string name) {
    // recursively get uninit locations of parent types, if the parent constructor isnt explicitly called
    for ( auto p : *type->parents() ) {
        auto u = (UseNode*)p;

        for ( auto id : *u->ids() ) {
            auto sym = id->symbol()->ensureVariable();
            assert(sym->getObjectType()->getTag() == ASTNodeTag::TYPEBODY);
            bool flag = true;
            for ( auto pc : *constructor->parentConstructors() ) {
                auto call = (CallExpressionNode*)pc;
                if ( ((IdentifierNode*)call->func())->symbol() == sym ) {
                    flag = false;
                    break;
                }
            }
            if (flag) populate(vcw, (TypeBodyNode*)sym->getObjectType(), constructor, id->name() + "::");
        }
    }
    // get uninit locations for this type
    for ( auto d : *type->declarations() ) {
        if ( d->getTag() == ASTNodeTag::UNINITIALIZEDVARIABLEDECLARATION ) {
            // get symbols that need to be initialized
            auto uninit = ((UninitializedVariableDeclarationNode*)d);
            vcw.logger->debug("Adding " + name + uninit->id()->name() + " to list of required initializations.");
            vcw._symbols.insert(uninit->id()->symbol());
            vcw._errNameMap.insert({ uninit->id()->symbol(), name + uninit->id()->name() });
        } else if ( d->getTag() == ASTNodeTag::VARIABLEDECLARATION ) {
            // get default values of functions
            auto v = (VariableDeclarationNode*)d;
            if ( v->typeNode()->value()->isCallable() ) {
                vcw.setPossibleFunctions(v->id()->symbol(), v->value());
            }
            // remove parent instance of uninit value if it has a default value in child
            for ( auto sym : vcw._symbols ) {
                if ( sym->name() == v->id()->name() ) {
                    vcw.logger->debug("Removing " + vcw._errNameMap[sym] + " from list of required initializtaions.");
                    vcw._symbols.erase(sym);
                    vcw._errNameMap.erase(sym);
                    break;
                }
            }
        }
    }
}

void ValidConstructorWalk::walkProgramNode(ProgramNode* node) {
    for ( auto stmt : *node->body() ) {
        walk(stmt);
    }
}

void ValidConstructorWalk::walkExpressionStatementNode(ExpressionStatementNode* node) {
    walk(node->expression());
}

void ValidConstructorWalk::walkEnumerableAccessNode(EnumerableAccessNode* node) {
    return walk(node->path());
}

void ValidConstructorWalk::walkEnumerableAppendNode(EnumerableAppendNode* node) {
    return walk(node->path());
}

void ValidConstructorWalk::walkMapAccessNode(MapAccessNode* node) {
    return walk(node->path());
}

void ValidConstructorWalk::walkClassAccessNode(ClassAccessNode* node) {
    return walk(node->path());
}

void ValidConstructorWalk::walkEnumerationLiteralExpressionNode(EnumerationLiteralExpressionNode* node) {
    for ( auto i : *node->actuals() ) {
        walk(i);
    }
}

void ValidConstructorWalk::walkMapStatementNode(MapStatementNode* node) {
    walk(node->value());
}

void ValidConstructorWalk::walkMapNode(MapNode* node) {
    for ( auto i : *node->body() ) {
        walk(i);
    }
}

void ValidConstructorWalk::walkAssignExpressionNode(AssignExpressionNode* node) {
    if ( node->dest()->getTag() == ASTNodeTag::IDENTIFIER ) {
        auto dest = (IdentifierNode*)node->dest();
        if ( _symbols.count(dest->symbol()) ) {
            _symbols.erase(dest->symbol());
        }

        if ( dest->symbol()->type()->isCallable() ) {
            setPossibleFunctions(dest->symbol(), node->value());
        }
    }
    walk(node->value());
}

void ValidConstructorWalk::walkVariableDeclarationNode(VariableDeclarationNode* node) {
    if ( node->typeNode()->value()->isCallable() ) {
        setPossibleFunctions(node->id()->symbol(), node->value());
    }
    walk(node->assignment()->value());
}

void ValidConstructorWalk::walkReturnStatementNode(ReturnStatementNode* node) {
    if ( node->value() != nullptr ) walk(node->value());

    if ( !_symbols.empty() ) {
        std::string symstr = "";
        for (auto sym : _symbols) symstr += _errNameMap[sym] + ", ";
        logger->error(
            s(node->position()) +
            " Unable to determine value of { " + symstr.substr(0, symstr.size() - 2) + " } in type constructor."
        );
        _goodReturns = false;
    }
}

void ValidConstructorWalk::walkFunctionNode(FunctionNode* node) {
    for ( auto stmt : *node->body() ) {
        walk(stmt);
    }
}

void ValidConstructorWalk::walkCallExpressionNode(CallExpressionNode* node) {
    for ( auto arg : *node->args() ) {
        walk(arg);
    }
    if ( node->func()->getTag() == ASTNodeTag::IDENTIFIER ) {
        auto id = (IdentifierNode*)node->func();
        if ( node->constructor() ) {
            // walk the constructor being called
            walk(node->constructor());
        } else if ( _possibleFunctions.count(id->symbol()) ) {
            auto prewalk = _symbols;
            std::vector<std::set<SemanticSymbol*>> postwalks;

            // walk every possible function
            for (auto f : _possibleFunctions.at(id->symbol())) {
                walk(f);
                postwalks.push_back(_symbols);
                _symbols = prewalk;
            }

            // get symbols that were not assigned in at least 1 possible function
            _symbols = std::set<SemanticSymbol*>();
            for (auto& p : postwalks) {
                _symbols.insert(p.begin(), p.end());
            }
        } else {
            logger->debug("Function " + id->name() + " is too ambiguous for constructor validation.");
        }
    }
}

void ValidConstructorWalk::walkDeferCallExpressionNode(DeferCallExpressionNode* node) {
    walkCallExpressionNode(node->call());
}

void ValidConstructorWalk::walkAndNode(AndNode* node) {
    walkBinaryExpressionNode(node);
}

void ValidConstructorWalk::walkOrNode(OrNode* node) {
    walkBinaryExpressionNode(node);
}

void ValidConstructorWalk::walkEqualsNode(EqualsNode* node) {
    walkBinaryExpressionNode(node);
}

void ValidConstructorWalk::walkNumericComparisonExpressionNode(NumericComparisonExpressionNode* node) {
    walkBinaryExpressionNode(node);
}

void ValidConstructorWalk::walkNotEqualsNode(NotEqualsNode* node) {
    walkBinaryExpressionNode(node);
}

void ValidConstructorWalk::walkAddNode(AddNode* node) {
    walkBinaryExpressionNode(node);
}

void ValidConstructorWalk::walkSubtractNode(SubtractNode* node) {
    walkBinaryExpressionNode(node);
}

void ValidConstructorWalk::walkMultiplyNode(MultiplyNode* node) {
    walkBinaryExpressionNode(node);
}

void ValidConstructorWalk::walkDivideNode(DivideNode* node) {
    walkBinaryExpressionNode(node);
}

void ValidConstructorWalk::walkModulusNode(ModulusNode* node) {
    walkBinaryExpressionNode(node);
}

void ValidConstructorWalk::walkPowerNode(PowerNode* node) {
    walkBinaryExpressionNode(node);
}

void ValidConstructorWalk::walkNthRootNode(NthRootNode* node) {
    walkBinaryExpressionNode(node);
}

void ValidConstructorWalk::walkNegativeExpressionNode(NegativeExpressionNode* node) {
    walkUnaryExpressionNode(node);
}

void ValidConstructorWalk::walkNotNode(NotNode* node) {
    walkUnaryExpressionNode(node);
}

void ValidConstructorWalk::walkEnumerationConcatNode(EnumerationConcatNode* node) {
    walkBinaryExpressionNode(node);
}


void ValidConstructorWalk::walkWithStatement(WithStatement* node) {
    // this remains in the top layer because its body executes unconditionally
    for ( auto stmt : *node->body() ) {
        walk(stmt);
    }
}

void ValidConstructorWalk::walkIfStatement(IfStatement* node) {
    walkBlockStatementNode(node);
}

void ValidConstructorWalk::walkWhileStatement(WhileStatement* node) {
    walkBlockStatementNode(node);
}

void ValidConstructorWalk::walkBinaryExpressionNode(BinaryExpressionNode* node) {
    walk(node->left());
    walk(node->right());
}

void ValidConstructorWalk::walkUnaryExpressionNode(UnaryExpressionNode* node) {
    walk(node->exp());
}

void ValidConstructorWalk::walkBlockStatementNode(BlockStatementNode* node) {
    _inTopLayer = false;
    auto before = _symbols;
    for (auto stmt : *node->body()) {
        walk(stmt);
    }
    _symbols = before;
    _inTopLayer = true;
}

void ValidConstructorWalk::setPossibleFunctions(SemanticSymbol* destSym, ExpressionNode* value) {
    if ( !_possibleFunctions.count(destSym) ) {
        _possibleFunctions.insert({ destSym, std::vector<FunctionNode*>() });
    }
    if ( _inTopLayer ) {
        _possibleFunctions.at(destSym).clear();
    }
    std::string name = _errNameMap.count(destSym) ? _errNameMap[destSym] : destSym->name();
    if ( value->getTag() == ASTNodeTag::FUNCTION ) {
        logger->debug("Giving " + name + " the possible value of " + value->toString());
        _possibleFunctions.at(destSym).push_back((FunctionNode*)value);
    } else if ( value->getTag() == ASTNodeTag::IDENTIFIER
            && _possibleFunctions.count(((IdentifierNode*) value)->symbol()) )
    {
        for (auto f : _possibleFunctions.at(((IdentifierNode*) value)->symbol())) {
            logger->debug("Giving " + name + " the possible value of " + f->toString());
            _possibleFunctions.at(destSym).push_back(f);
        }
    } else {
        logger->debug(value->toString() + " is too ambiguous for constructor validation. " + name + " is now ambiguous.");
        _possibleFunctions.erase(destSym);
    }
}

}