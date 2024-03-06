#include "ValidConstructorWalk.h"

namespace swarmc::Lang::Walk {

bool ValidConstructorWalk::isValidConstructor(TypeBodyNode* type, ConstructorNode* constructor) {
    ValidConstructorWalk vcw;
    vcw.logger->debug("Validating constructor " + s(constructor));
    for ( auto d : *type->declarations() ) {
        if ( d->getTag() == ASTNodeTag::UNINITIALIZEDVARIABLEDECLARATION ) {
            // get symbols that need to be initialized
            auto uninit = ((UninitializedVariableDeclarationNode*)d);
            vcw.logger->debug("Adding " + uninit->id()->name() + " to list of required initializations.");
            vcw._symbols.insert(uninit->id()->symbol());
        } else if ( d->getTag() == ASTNodeTag::VARIABLEDECLARATION ) {
            // get default values of functions
            auto v = ((VariableDeclarationNode*)d);
            if ( v->typeNode()->value()->isCallable() ) {
                vcw.setPossibleFunctions(v->id()->symbol(), v->value());
            }
        }
    }
    vcw.walk(constructor->func());
    if (!vcw._symbols.empty()) {
        std::string ss = "";
        for (auto sym : vcw._symbols) ss += sym->name() + ", ";
        vcw.logger->error(
            s(constructor->position()) +
            " Unable to determine value of { " + ss.substr(0, ss.size() - 2) + " } in type constructor."
        );
    }
    return vcw._symbols.empty() && vcw._goodReturns;
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
}

void ValidConstructorWalk::walkVariableDeclarationNode(VariableDeclarationNode* node) {
    if ( node->typeNode()->value()->isCallable() ) {
        setPossibleFunctions(node->id()->symbol(), node->value());
    }
}

void ValidConstructorWalk::walkReturnStatementNode(ReturnStatementNode* node) {
    if ( node->value() != nullptr ) walk(node->value());

    if ( !_symbols.empty() ) {
        std::string symstr = "";
        for (auto sym : _symbols) symstr += sym->name() + ", ";
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
    if ( value->getTag() == ASTNodeTag::FUNCTION ) {
        logger->debug("Giving " + destSym->name() + " the possible value of " + value->toString());
        _possibleFunctions.at(destSym).push_back((FunctionNode*)value);
    } else if ( value->getTag() == ASTNodeTag::IDENTIFIER
            && _possibleFunctions.count(((IdentifierNode*) value)->symbol()) )
    {
        for (auto f : _possibleFunctions.at(((IdentifierNode*) value)->symbol())) {
            logger->debug("Giving " + destSym->name() + " the possible value of " + f->toString());
            _possibleFunctions.at(destSym).push_back(f);
        }
    } else {
        logger->debug(value->toString() + " is too ambiguous for constructor validation. " + destSym->name() + " is now ambiguous.");
        _possibleFunctions.erase(destSym);
    }
}

}