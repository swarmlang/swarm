#ifndef SWARMVM_THREAD_H
#define SWARMVM_THREAD_H

#include <mutex>
#include <thread>
#include <iostream>
#include "../../Configuration.h"
#include "../VirtualMachine.h"

namespace swarmc::Runtime::MultiThreaded {

class SwarmThread {
public:

    static void execute(VirtualMachine* vm, IQueueJob* job, ScopeFrame* scope, State* state, IFunctionCall* call) {
        std::cout << "thread execution!\n";

        // Increment thread counter
        auto lock = new std::lock_guard<std::mutex>(CountMutex);
        CurrentThreads++;
        std::cout << "Thread Count: " << CurrentThreads << "\n";
        delete lock;

        // execute
        //Console::get()->debug("Got VM from queue: " + vm->toString());
        std::cout << "Pre-restore\n";
        vm->restore(scope, state);
        std::cout << "post-restore\n";
        vm->executeCall(call);
        std::cout << "post-execute\n";

        // Decrement Thread counter
        lock = new std::lock_guard<std::mutex>(CountMutex);
        CurrentThreads--;
        std::cout << "Thread Count: " << CurrentThreads << "\n";
        delete lock;

        std::lock_guard<std::mutex> fmLock(FinishedMutex);
        Finished.at(job->id()) = true;
    }

    /*static void execute(VirtualMachine* vm, IQueueJob* job) {
        std::cout << "thread execution!\n";
        // Increment thread counter
        auto lock = new std::lock_guard<std::mutex>(CountMutex);
        CurrentThreads++;
        std::cout << "Thread Count: " << CurrentThreads << "\n";
        delete lock;

        // execute
        //Console::get()->debug("Got VM from queue: " + vm->toString());
        std::cout << "Pre-restore\n";
        vm->restore(job->getScope()->copy(), job->getState()->copy());
        std::cout << "post-restore\n";
        vm->executeCall(job->getCall());
        std::cout << "post-execute\n";

        // Decrement Thread counter

        Finished.at(job->id()) = true;
    }*/

    SwarmThread() = delete;

    ~SwarmThread() = delete;

    static bool moreThreads() {
        auto lock = new std::lock_guard<std::mutex>(CountMutex);
        auto more = CurrentThreads < Configuration::MAX_THREADS;
        delete lock;
        return more;
    }

    static void addThread(size_t id, std::thread* th) {
        std::lock_guard<std::mutex> fmLock(FinishedMutex);
        std::lock_guard<std::mutex> tmLock(ThreadsMutex);
        Finished.insert({ id, false });
        Threads.insert({ id, th });
    }

    static std::map<size_t, std::thread*> getThreads() {
        return Threads;
    }

    static bool isFinished(size_t id) {
        std::lock_guard<std::mutex> fmLock(FinishedMutex);
        return Finished.at(id);
    }

    static void cleanThread(size_t id) {
        std::lock_guard<std::mutex> fmLock(FinishedMutex);
        std::lock_guard<std::mutex> tmLock(ThreadsMutex);
        Threads.at(id)->join();
        delete Threads.at(id);
        Threads.erase(id);
        Finished.erase(id);
        auto lock = new std::lock_guard<std::mutex>(CountMutex);
        CurrentThreads--;
        delete lock;
    }

protected:
    static size_t CurrentThreads;
    static std::mutex CountMutex;
    static std::map<size_t, std::thread*> Threads;
    static std::mutex ThreadsMutex;
    static std::map<size_t, bool> Finished;
    static std::mutex FinishedMutex;
};

}

#endif
