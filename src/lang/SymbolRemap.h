#ifndef SWARMC_SYMBOLREMAP_H
#define SWARMC_SYMBOLREMAP_H

#include <unordered_map>
#include "AST.h"
#include "../vm/ISA.h"

namespace swarmc::Lang {

// LVAL we see -> (replace, location of evaluation)
using SymbolMap = std::vector<LValNode*, std::pair<LValNode*, ISA::LocationReference*>>;

class SymbolRemapScope {
public:
    explicit SymbolRemapScope(SymbolRemapScope* parent) : _parent(parent) {}

    bool has(const LValNode* lval) const {

        if ( _parent == nullptr ) return false;
        return _parent->has(lval);
    }

    // ?? replace(const LValNode* lval) const {

    // }

    // void registerLVal(??) {

    // }

protected:
    SymbolRemapScope* _parent;
    SymbolMap _map;
};

}

#endif