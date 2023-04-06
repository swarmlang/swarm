#include <string>
#include "AST.h"
#include "SymbolTable.h"
#include "Walk/PrintWalk.h"

#ifndef SWARMC_SPACE
#define SWARMC_SPACE "  "
#endif

namespace swarmc::Lang {

    ProgramNode* ProgramNode::copy() const  {
        auto other = new ProgramNode;
        for ( auto stmt : *_body ) {
            other->pushStatement(stmt->copy());
        }

        return other;
    }

    SemanticSymbol* MapAccessNode::lockable() const {
        return _path->lockable();
    }

    SemanticSymbol* EnumerableAccessNode::lockable() const {
        return _path->lockable();
    }

    SemanticSymbol* ClassAccessNode::lockable() const {
        return _path->lockable();
    }

    size_t ConstructorNode::nameID = 0;

}
