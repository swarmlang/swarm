#ifndef SWARM_LOCALSYMBOLVALUESTORE_H
#define SWARM_LOCALSYMBOLVALUESTORE_H

#include <vector>
#include <cassert>
#include "ISymbolValueStore.h"
#include "../errors/FreeSymbolError.h"
#include "../shared/util/Console.h"
#include "../lang/Walk/SymbolWalk.h"

namespace swarmc {
namespace Lang {
    class TagResourceNode;
}

namespace Runtime {

    /**
     * A symbol value store implementation that records values in an in-memory map.
     */
    class LocalSymbolValueStore : public ISymbolValueStore, public IUsesConsole {
    protected:
        friend class Lang::TagResourceNode;

        /** The internal symbol -> value mapping. */
        std::map<Lang::SemanticSymbol*, Lang::ExpressionNode*>* _map;
        std::map<std::string, std::string> _filters;
    public:
        LocalSymbolValueStore() : ISymbolValueStore(), IUsesConsole() {
            _map = new std::map<Lang::SemanticSymbol*, Lang::ExpressionNode*>();
        }

        virtual ~LocalSymbolValueStore() {
            delete _map;
        }

        virtual std::map<std::string, std::string> filters() const {
            return _filters;
        }

        virtual std::string serialize(Lang::Walk::SymbolMap* symbols);

        virtual void deserialize(std::string payload);

        virtual void setValue(Lang::SemanticSymbol* symbol, Lang::ExpressionNode* value) override {
            auto iterator = _map->begin();
            while ( iterator != _map->end() ) {
                auto potentialSymbol = iterator->first;
                if ( potentialSymbol->uuid() == symbol->uuid() ) {
                    _map->erase(potentialSymbol);
                    break;
                }

                iterator++;
            }

//            console->debug("Set: " + symbol->name() + ", Value: " + value->toString());
            _map->insert(std::pair<Lang::SemanticSymbol*, Lang::ExpressionNode*>(symbol, value));
        }

        virtual Lang::ExpressionNode* tryGetValue(Lang::SemanticSymbol* symbol) override {
            assert(symbol != nullptr);

            auto iterator = _map->begin();
            while ( iterator != _map->end() ) {
                auto potentialSymbol = iterator->first;
                if ( potentialSymbol->uuid() == symbol->uuid() ) {
                    return iterator->second;
                }

                iterator++;
            }

            return nullptr;
        }

        virtual Lang::ExpressionNode* getValue(Lang::SemanticSymbol* symbol) override {
            auto value = tryGetValue(symbol);
            if ( value == nullptr ) {
                console->error("[local] invalid access of free symbol: " + symbol->toString());
                throw Errors::FreeSymbolError(symbol->name());
            }

            return value;
        }

        virtual std::string toString() const {
            return "LocalSymbolValueStore<#symbols: " + std::to_string(_map->size()) + ">";
        }

        bool tryLockSymbol(Lang::SemanticSymbol* symbol) override {
            return true;
        }

        void lockSymbol(Lang::SemanticSymbol* symbol) override {}

        void unlockSymbol(Lang::SemanticSymbol* symbol) override {}
    };

}
}

#endif //SWARM_LOCALSYMBOLVALUESTORE_H
