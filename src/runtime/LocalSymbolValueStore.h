#ifndef SWARM_LOCALSYMBOLVALUESTORE_H
#define SWARM_LOCALSYMBOLVALUESTORE_H

#include <vector>
#include <assert.h>
#include "ISymbolValueStore.h"
#include "../errors/FreeSymbolError.h"

namespace swarmc {
namespace Runtime {

    /**
     * A symbol value store implementation that records values in an in-memory map.
     */
    class LocalSymbolValueStore : public ISymbolValueStore {
    protected:
        /** The internal symbol -> value mapping. */
        std::map<Lang::SemanticSymbol*, Lang::ExpressionNode*>* _map;
    public:
        LocalSymbolValueStore() : ISymbolValueStore() {
            _map = new std::map<Lang::SemanticSymbol*, Lang::ExpressionNode*>();
        }

        virtual ~LocalSymbolValueStore() {
            delete _map;
        }

        virtual void setValue(Lang::SemanticSymbol* symbol, Lang::ExpressionNode* value) override {
            auto search = _map->find(symbol);
            if ( search != _map->end() ) {
                _map->erase(symbol);
            }

            _map->insert(std::pair<Lang::SemanticSymbol*, Lang::ExpressionNode*>(symbol, value));
        }

        virtual Lang::ExpressionNode* tryGetValue(Lang::SemanticSymbol* symbol) override {
            assert(symbol != nullptr);

            auto search = _map->find(symbol);
            if ( search != _map->end() ) {
                return search->second;
            }

            return nullptr;
        }

        virtual Lang::ExpressionNode* getValue(Lang::SemanticSymbol* symbol) override {
            auto value = tryGetValue(symbol);
            if ( value == nullptr ) {
                throw Errors::FreeSymbolError(symbol->name());
            }

            return value;
        }
    };

}
}

#endif //SWARM_LOCALSYMBOLVALUESTORE_H
