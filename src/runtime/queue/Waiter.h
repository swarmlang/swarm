#ifndef SWARM_WAITER_H
#define SWARM_WAITER_H

#include <string>
#include <map>
#include <thread>
#include <mutex>
#include <utility>
#include <sw/redis++/redis++.h>

namespace swarmc {
namespace Runtime {

    class Waiter {
    public:
        static std::map<std::string, Waiter*>* instances;

        static void join() {
            if ( _createdSubscriber ) {
                _thread->join();
            }
        }

        explicit Waiter(std::string id): _id(id) {}

        void wait();

        void start() {
            std::unique_lock<std::mutex> lock(_mutex);
            _started = true;
        }

        void finish() {
            std::unique_lock<std::mutex> lock(_mutex);
            _terminated = true;
        }

        bool started() {
            std::unique_lock<std::mutex> lock(_mutex);
            return _started;
        }

        bool finished() {
            std::unique_lock<std::mutex> lock(_mutex);
            return _terminated;
        }

        std::string id() const {
            return _id;
        }

    protected:
        static bool _createdSubscriber;
        static sw::redis::Subscriber _subscriber;
        static void createSubscriber();
        static std::thread* _thread;

        std::string _id;
        std::mutex _mutex;
        bool _terminated = false;
        bool _started = false;
    };

}
}

#endif //SWARM_WAITER_H
