#ifndef SWARMVM_THREAD_H
#define SWARMVM_THREAD_H

#include <mutex>
#include <thread>
#include "../../Configuration.h"
#include "../VirtualMachine.h"

namespace swarmc::Runtime::MultiThreaded {

class SwarmThread {
public:

    static void execute(VirtualMachine* vm, IQueueJob* job) {
        static std::mutex threadCount;

        // Incremenet thread counter
        auto lock = new std::lock_guard<std::mutex>(threadCount);
        CurrentThreads++;
        delete lock;

        // execute
        vm->copy([job](VirtualMachine* vm) {
            Console::get()->debug("Got VM from queue: " + vm->toString());
            vm->restore(job->getScope()->copy(), job->getState()->copy());
            vm->executeCall(job->getCall());
        });

        // Decrement Thread counter
        lock = new std::lock_guard<std::mutex>(threadCount);
        CurrentThreads--;
        delete lock;
    }

    SwarmThread() = delete;

    ~SwarmThread() = delete;

    static bool moreThreads() { 
        return CurrentThreads < Configuration::MAX_THREADS;
    }

protected:
    static size_t CurrentThreads;
};

}

#endif