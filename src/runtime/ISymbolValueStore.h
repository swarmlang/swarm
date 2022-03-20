#ifndef SWARM_ISYMBOLVALUESTORE_H
#define SWARM_ISYMBOLVALUESTORE_H

#include <functional>
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

        virtual bool tryLockSymbol(Lang::SemanticSymbol* symbol) = 0;

        virtual void lockSymbol(Lang::SemanticSymbol* symbol) = 0;

        virtual void unlockSymbol(Lang::SemanticSymbol* symbol) = 0;

        template <typename TReturn>
        TReturn withLockedSymbol(Lang::SemanticSymbol* symbol, const std::function<TReturn()>& callback) {
            lockSymbol(symbol);

            try {
                TReturn val = callback();
                unlockSymbol(symbol);
                return val;
            } catch (...) {
                unlockSymbol(symbol);
                throw;
            }
        }
    };

}
}


#endif //SWARM_ISYMBOLVALUESTORE_H
