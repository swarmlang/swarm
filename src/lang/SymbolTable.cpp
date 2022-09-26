#include "SymbolTable.h"

namespace swarmc {
namespace Lang {
    ScopeTable* ScopeTable::prologue() {
        auto prologueScope = new swarmc::Lang::ScopeTable();
        return prologueScope;
    }
}
}
