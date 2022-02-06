#ifndef SWARM_ISYMBOLVALUESTORE_H
#define SWARM_ISYMBOLVALUESTORE_H

#include "../shared/IStringable.h"
#include "../lang/SymbolTable.h"
#include "../lang/AST.h"

namespace swarmc {
namespace Runtime {

    /**
     * Interface for a runtime store that maps symbols to their values. An environment, if you will.
     */
    class ISymbolValueStore : public IStringable {
    public:
        virtual ~ISymbolValueStore() {}

        /** Set the value of the symbol in the store. */
        virtual void setValue(Lang::SemanticSymbol* symbol, Lang::ExpressionNode* value) = 0;

        /** Get the value of the symbol in the store. Returns nullptr if none is found. */
        virtual Lang::ExpressionNode* tryGetValue(Lang::SemanticSymbol* symbol) = 0;

        /** Get the value of the symbol in the store. Throws FreeSymbolError if none is found. */
        virtual Lang::ExpressionNode* getValue(Lang::SemanticSymbol* symbol) = 0;
    };

}
}


#endif //SWARM_ISYMBOLVALUESTORE_H
