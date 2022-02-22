#include "SymbolTable.h"
#include "../../runtime/prologue/IPrologueFunction.h"

namespace swarmc {
namespace Lang {
    ScopeTable* ScopeTable::prologue() {
        auto prologueScope = new swarmc::Lang::ScopeTable();
        Runtime::Prologue::IPrologueFunction::buildScope(prologueScope);
        return prologueScope;
    }
}
}
