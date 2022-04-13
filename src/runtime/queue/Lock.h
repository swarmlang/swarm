#ifndef SWARM_LOCK_H
#define SWARM_LOCK_H

#include <thread>
#include <chrono>
#include <string>
#include <map>
#include "../../Configuration.h"
#include "../../shared/uuid.h"
#include "../../shared/IStringable.h"

namespace swarmc {
namespace Runtime {
    class LockManager;

    class Lock : public IStringable {
    public:
        Lock(std::string name, LockManager* manager) : _name(name), _manager(manager) {}
        virtual ~Lock() {}

        bool tryToAcquire();

        void acquire() {
            while ( !held() && !tryToAcquire() ) {
                std::this_thread::sleep_for(std::chrono::microseconds(Configuration::LOCK_SLEEP_uS));
            }

            _holders += 1;
        }

        void release();

        bool held() {
            return _holders > 0;
        }

        std::string toString() const override {
            return "Lock<name: " + _name + ", holders: " + std::to_string(_holders) + ", uuid: " + _uuid + ">";
        }

    protected:
        std::string _name;
        size_t _holders = 0;
        std::string _uuid = util::uuid4();
        LockManager* _manager;
    };

    class LockManager {
    public:
        LockManager() {}
        ~LockManager() {
            delete _locks;
        }

        Lock* get(const std::string& name) {
            auto result = _locks->find(name);
            if ( result != _locks->end() ) {
                return result->second;
            }

            auto lock = new Lock(name, this);
            _locks->insert({name, lock});
            return lock;
        }

    protected:
        std::map<std::string, Lock*>* _locks = new std::map<std::string, Lock*>;

        friend class Lock;
    };

}
}

#endif //SWARM_LOCK_H
