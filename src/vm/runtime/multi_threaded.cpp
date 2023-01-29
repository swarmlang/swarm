#include <cassert>
#include <iostream>
#include "swarm_thread.h"
#include "multi_threaded.h"
#include "../VirtualMachine.h"
#include "../../errors/ClearLockedReferences.h"

namespace swarmc::Runtime::MultiThreaded {

    size_t SwarmThread::CurrentThreads = 0;
    std::mutex SwarmThread::CountMutex;
    std::map<size_t, std::thread*> SwarmThread::Threads = std::map<size_t, std::thread*>();
    std::map<size_t, bool> SwarmThread::Finished  = std::map<size_t, bool>();

    IStorageLock *SharedStorageInterface::acquire(ISA::LocationReference *loc) {
        if ( _locks.find(loc->fqName()) != _locks.end() ) return nullptr;
        _locks[loc->fqName()] = new StorageLock(this, loc, _mutexes[loc->fqName()]);
        return _locks[loc->fqName()];
    }

    void SharedStorageInterface::clear() {
        if ( !_locks.empty() ) throw Errors::ClearLockedReferences();
        _map.clear();
        _types.clear();
        _mutexes.clear();
    }

    ISA::LocationReference *StorageLock::location() const {
        return _loc;
    }

    void StorageLock::release() {
        delete _lock;
        _store->_locks.erase(_loc->fqName());
    }

    std::string StorageLock::toString() const {
        return "SingleThreaded::StorageLock<loc: " + _loc->toString() + ">";
    }

    QueueJob::QueueJob(
        JobID id, JobState jobState, IFunctionCall *call, ScopeFrame *scope, State *vmState):
            _id(id), _jobState(jobState), _call(useref(call)), _scope(useref(scope)), _vmState(useref(vmState)) {}

    QueueJob::~QueueJob() noexcept {
        freeref(_call);
        freeref(_scope);
        freeref(_vmState);
    }

    bool Queue::shouldHandle(IFunctionCall* call) {
        return true;
    }

    IQueueJob* Queue::build(IFunctionCall* call, const ScopeFrame* scope, const State* state) {
        return new QueueJob(_nextId++, JobState::PENDING, call, scope->copy(), state->copy());
    }

    void Queue::push(IQueueJob* job) {
        std::unique_lock<std::mutex> threadLock(_threadMutex);

        auto qlock = new std::lock_guard<std::mutex>(_qtex);
        if ( _threads.size() < Configuration::MAX_THREADS ) {
            std::cout << "Create thread! (host tid: " + Framework::getThreadDisplay() + ")\n";
            auto vm = _vm->copy();
            auto scope = job->getScope()->copy();
            auto state = job->getState()->copy();
            auto call = job->getCall();

            auto ctx = Framework::newThread([vm, scope, state, call] () {
                std::cout << "thread execution! (host tid: " + s(Framework::context()->getID()) + ")\n";

                Framework::context()
                    ->onShutdown([vm]() {
                        std::cout << "Cleaning up thread: " << Framework::context()->getID() << "\n";
                        vm->cleanup();
                        delete vm;
                    });

                Console::get()->debug("Got VM from queue: " + vm->toString());
                std::cout << "Pre-restore\n";
                vm->restore(scope, state);
                std::cout << "post-restore\n";
                vm->executeCall(call);
                std::cout << "post-execute\n";

                return 0;
            });

            _threads.push_back(ctx);
            threadLock.unlock();

//            SwarmThread::addThread(job->id(), new std::thread(SwarmThread::execute, vm, job));
//            SwarmThread::addThread(job->id(), new std::thread(SwarmThread::execute, vm, job, job->getScope()->copy(), job->getState()->copy(), job->getCall()));
//            SwarmThread::addThread(job->id(), nullptr);
//            SwarmThread::execute(vm, job);

//            vm->cleanup();
//            delete vm;
        } else {
            std::cout << "Push to queue!\n";
            _queue.push(job);
        }
        delete qlock;
    }

    IQueueJob* Queue::pop() {
        std::cout << "Popping job: " << _queue.front()->toString() << "\n";
        auto lock = new std::lock_guard<std::mutex>(_qtex);
        assert(!_queue.empty());
        auto job = _queue.front();
        _queue.pop();
        delete lock;
        return job;
    }

    void Queue::tick() {
        std::cout << "Tick!\n";

        Framework::tickThreads();

        std::unique_lock<std::mutex> threadLock(_threadMutex);
        std::unique_lock<std::mutex> queueLock(_qtex);
        std::cout << "# threads to tick: " << _threads.size() << "\n";
        for ( auto iter = _threads.begin(); iter != _threads.end(); ) {
            std::cout << "Tick Iter!\n";
            if ( (*iter)->hasExited() ) {
                std::cout << "Thread removed: " << (*iter)->getID() << "\n";
                iter = _threads.erase(iter);
                delete *iter;
            } else {
                ++iter;
            }
        }
    }

    Stream::~Stream() noexcept {
        while ( !_items.empty() ) {
            freeref(_items.front());
            _items.pop();
        }
    }

    void Stream::push(ISA::Reference* value) {
        assert(value->type()->isAssignableTo(_innerType));
        _items.push(useref(value));
    }

    ISA::Reference* Stream::pop() {
        assert(!_items.empty());
        auto top = _items.front();
        _items.pop();
        top->nslibDecRef();  // to undo the `useref`, but without freeing it
        return top;
    }

    IStream* StreamDriver::open(const std::string &id, Type::Type* innerType) {
        return new Stream(id, innerType);
    }

}
