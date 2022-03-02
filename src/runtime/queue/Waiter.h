#ifndef SWARM_WAITER_H
#define SWARM_WAITER_H

#include <pthread.h>
#include <stdlib.h>
#include <string>
#include <map>

namespace swarmc {
namespace Runtime {
    void* waiterThread(std::string id);

    struct threadArgs {
        int tid;
        int inc;
        int loop;
    } threadArgs;

    class Waiter {
    public:
        static std::map<std::string, Waiter*>* instances;

        Waiter(std::string id): _id(id) {}

        void wait() {

        }

        void start() {
            acquire();
            _started = true;
            release();
        }

        void finish() {
            acquire();
            _terminated = true;
            release();
        }

        bool started() {
            acquire();
            bool val = _started;
            release();
            return val;
        }

        bool finished() {
            acquire();
            bool val = _terminated;
            release();
            return val;
        }

        std::string id() const {
            return _id;
        }
    private:

        void acquire() {
            pthread_mutex_lock(&this->_access);
        }

        void release() {
            pthread_mutex_unlock(&this->_access);
        }

        std::string _id;
        pthread_mutex_t _access;
        bool _terminated = false;
        bool _started = false;
    };

}
}

#endif //SWARM_WAITER_H
