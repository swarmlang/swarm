#ifndef SWARMC_SYMBOLREMAP_H
#define SWARMC_SYMBOLREMAP_H

#include <map>
#include "AST.h"
#include "../vm/ISA.h"

namespace swarmc::Lang {

// LVAL we see -> (replace, location of evaluation)
using SymbolReplaceMap = std::map<SemanticSymbol*, ISA::LocationReference*>;

class SymbolRemapScope {
public:
    explicit SymbolRemapScope(SymbolRemapScope* parent) : _parent(parent) {}

    ~SymbolRemapScope() {
        for ( const auto& p: _map ) {
            freeref(p.first);
            freeref(p.second);
        }
    }

    [[nodiscard]] SymbolRemapScope* enter() {
        return new SymbolRemapScope(this);
    }

    [[nodiscard]] SymbolRemapScope* leave() {
        auto parent = _parent;
        delete this;
        return parent;
    }

    [[nodiscard]] bool has(const SemanticSymbol* sym) const {
        for ( const auto& v : _map ) {
            if ( v.first == sym ) return true;
        }
        if ( _parent == nullptr ) return false;
        return _parent->has(sym);
    }

    [[nodiscard]] ISA::LocationReference* replace(SemanticSymbol* sym) {
        if ( _map.count(sym) == 0 ) return nullptr;
        return _map[sym];
    }

    void registerSymbol(SemanticSymbol* sym, ISA::LocationReference* loc) {
        _map.insert_or_assign(useref(sym), useref(loc));
    }

protected:
    SymbolRemapScope* _parent;
    SymbolReplaceMap _map;
};

}

#endif