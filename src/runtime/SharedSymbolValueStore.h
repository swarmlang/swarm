#ifndef SWARM_SHAREDSYMBOLVALUESTORE_H
#define SWARM_SHAREDSYMBOLVALUESTORE_H

#include <sstream>
#include <cassert>
#include <ctime>
#include <map>
#include "../Configuration.h"
#include "../shared/util/Console.h"
#include "ISymbolValueStore.h"
#include "../errors/FreeSymbolError.h"
#include "../lang/Walk/SerializeWalk.h"
#include "../lang/Walk/DeSerializeWalk.h"
#include "queue/ExecutionQueue.h"
#include "queue/Lock.h"

namespace swarmc {
namespace Runtime {

    class SharedSymbolValueStore : public ISymbolValueStore, public IUsesConsole {
    public:
        SharedSymbolValueStore() : ISymbolValueStore(), IUsesConsole() {}

        ~SharedSymbolValueStore() {
            delete _locks;
        }

        virtual std::string toString() const {
            return "SharedSymbolValueStore<>";
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
            console->debug("Trying to lock symbol: " + symbol->toString());
            return _locks->get(lockKey(symbol->uuid()))->tryToAcquire();
        }

        void lockSymbol(Lang::SemanticSymbol* symbol) override {
            _locks->get(lockKey(symbol->uuid()))->acquire();
        }

        void unlockSymbol(Lang::SemanticSymbol* symbol) override {
            auto lock = _locks->get(lockKey(symbol->uuid()));
            if ( !lock->held() ) {
                console->warn("Tried to free lock not held by current store for symbol: " + symbol->toString());
                return;
            }

            lock->release();
        }

    protected:
        LockManager* _locks = new LockManager;

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
