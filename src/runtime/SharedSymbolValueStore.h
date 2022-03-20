#ifndef SWARM_SHAREDSYMBOLVALUESTORE_H
#define SWARM_SHAREDSYMBOLVALUESTORE_H

#include <sstream>
#include <cassert>
#include <ctime>
#include <map>
#include "../Configuration.h"
#include "../../lib/redlock-cpp/redlock.h"
#include "../shared/util/Console.h"
#include "ISymbolValueStore.h"
#include "../errors/FreeSymbolError.h"
#include "../lang/Walk/SerializeWalk.h"
#include "../lang/Walk/DeSerializeWalk.h"
#include "queue/ExecutionQueue.h"

namespace swarmc {
namespace Runtime {

    class SharedSymbolValueStore : public ISymbolValueStore, public IUsesConsole {
    public:
        SharedSymbolValueStore() : ISymbolValueStore(), IUsesConsole() {}

        ~SharedSymbolValueStore() {
            delete _locks;
            delete _lockCounts;
        }

        virtual std::string toString() const {
            return "SharedSymbolValueStore<#locks: " + std::to_string(_locks->size()) + ">";
        }

        virtual void setValue(Lang::SemanticSymbol* symbol, Lang::ExpressionNode* value) override {
            Lang::Walk::SerializeWalk serialize;
            ExecutionQueue::getRedis()->set(symbolKey(symbol->uuid()), serialize.toJSON(value));
        }

        virtual Lang::ExpressionNode* tryGetValue(Lang::SemanticSymbol* symbol) override {
            Lang::Walk::DeSerializeWalk deserialize;
            auto result = ExecutionQueue::getRedis()->get(symbolKey(symbol->uuid()));
            if ( !result ) {
                return nullptr;
            }

            std::istringstream ijson(*result);
            auto node = deserialize.deserialize(&ijson);
            assert(node->isExpression());
            return (Lang::ExpressionNode*) node;
        }

        virtual Lang::ExpressionNode* getValue(Lang::SemanticSymbol* symbol) override {
            auto value = tryGetValue(symbol);
            if ( value == nullptr ) {
                throw Errors::FreeSymbolError(symbol->name());
            }

            return value;
        }

        bool tryLockSymbol(Lang::SemanticSymbol* symbol) override {
            auto pos = _locks->find(symbol->uuid());
            if ( pos != _locks->end() ) {
                // We already hold this lock.
                _lockCounts->at(symbol->uuid()) += 1;
                return true;
            }

            CLock* symbolLock = new CLock();
            bool acquired = getRedlock()->Lock(lockKey(symbol->uuid()).c_str(), 86400000, *symbolLock);
            if ( acquired ) {
                _locks->insert(std::pair<std::string, CLock*>(symbol->uuid(), symbolLock));
                _lockCounts->insert(std::pair<std::string, size_t>(symbol->uuid(), 1));
            }

            return acquired;
        }

        void lockSymbol(Lang::SemanticSymbol* symbol) override {
            while ( !tryLockSymbol(symbol) ) {
                usleep(Configuration::LOCK_SLEEP_uS);
            }
        }

        void unlockSymbol(Lang::SemanticSymbol* symbol) override {
            auto pos = _locks->find(symbol->uuid());
            if ( pos == _locks->end() ) {
                console->warn("Tried to free lock not held by current store for symbol: " + symbol->toString());
                return;
            }

            _lockCounts->at(symbol->uuid()) -= 1;
            if ( _lockCounts->at(symbol->uuid()) < 1 ) {
                CLock* symbolLock = pos->second;
                getRedlock()->Unlock(*symbolLock);
                _locks->erase(symbol->uuid());
                _lockCounts->erase(symbol->uuid());
            }
        }

    protected:
        static CRedLock* _redlock;

        std::map<std::string, CLock*>* _locks = new std::map<std::string, CLock*>();
        std::map<std::string, size_t>* _lockCounts = new std::map<std::string, size_t>();

        static CRedLock* getRedlock() {
            if ( _redlock == nullptr ) {
                _redlock = new CRedLock();
                _redlock->AddServerUrl(Configuration::REDIS_HOST.c_str(), Configuration::REDIS_PORT);
            }

            return _redlock;
        }

        static std::string symbolKey(const std::string& uuid) {
            return Configuration::REDIS_PREFIX + "semantic_symbol_value_" + uuid;
        }

        static std::string lockKey(const std::string& uuid) {
            return Configuration::REDIS_PREFIX + "semantic_symbol_lock_" + uuid;
        }
    };

}
}

#endif //SWARM_SHAREDSYMBOLVALUESTORE_H
