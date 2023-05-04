#include <cassert>
#include <iostream>
#include "multi_threaded.h"
#include "../VirtualMachine.h"
#include "../../errors/ClearLockedReferences.h"

namespace swarmc::Runtime::MultiThreaded {

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
        JobID id, JobState jobState, IFunctionCall* call, VirtualMachine* vm):
            _id(id), _jobState(jobState), _call(useref(call)), _vm(vm) {}

    QueueJob::~QueueJob() noexcept {
        freeref(_call);
        delete _vm;
    }

    Queue::Queue(VirtualMachine* vm) : _vm(vm) {
        _vm->lifecycle()->listen<VirtualMachineInitializingEvent>([this](VirtualMachineInitializingEvent& e) {
            spawnThreads();
        });
    }

    bool Queue::shouldHandle(IFunctionCall* call) {
        return true;
    }

    IQueueJob* Queue::build(IFunctionCall* call) {
        return new QueueJob(_nextId++, JobState::PENDING, call, _vm->copy());
    }

    void Queue::push(IQueueJob* job) {
        std::unique_lock<std::mutex> queueLock(_queueMutex);

        auto queueIter = _contextQueues.find(_context);
        if ( queueIter == _contextQueues.end() ) {
            std::queue<IQueueJob*> queue;
            queue.push(job);
            _contextQueues[_context] = queue;
            return;
        }

        queueIter->second.push(job);
    }

    IQueueJob* Queue::pop() {
        std::unique_lock<std::mutex> queueLock(_queueMutex);

        auto queueIter = _contextQueues.find(_context);
        if ( queueIter == _contextQueues.end() || queueIter->second.empty() ) {
            return nullptr;
        }

        auto job = queueIter->second.front();
        queueIter->second.pop();
        return job;
    }

    void Queue::tick() {
        // This is called while we are waiting for jobs to drain.
        // So, try to process any pending jobs here so we can wait productively.
        // This also helps avoid infinite waiting loops.
        tryToProcessJob();

        // Also, tick threads to keep nslib happy
        Framework::tickThreads();
    }

    void Queue::tryToProcessJob() {
        // Currently, the semantics of the VM guarantees that a Queue will only receive the instances of
        // IQueueJob that its Queue::build(...) method returns, so we're safe to cast the IQueueJob* as
        // QueueJob* here.
        auto jobPair = popForProcessing();
        auto job = dynamic_cast<QueueJob*>(jobPair.first);
        if ( job != nullptr ) {
            auto call = job->getCall();
            try {
                Console::get()->debug("Running job: " + s(job));
                job->getVM()->executeCall(call);
                job->setState(JobState::COMPLETE);
                decrementProcessingCount(jobPair.second);
            } catch (Errors::SwarmError& rte) {
                Console::get()->error("Thread error: " + s(rte));
                job->setState(JobState::ERROR);
                decrementProcessingCount(jobPair.second);
            } catch (...) {
                Console::get()->error("Unknown thread error!");
                job->setState(JobState::ERROR);
                decrementProcessingCount(jobPair.second);
            }
        }
    }

    void Queue::spawnThreads() {
        std::unique_lock<std::mutex> threadLock(_threadMutex);
        if ( !_threads.empty() ) {
            return;  // We've already done this!
        }

        Console::get()->debug("Starting " + s(Configuration::MAX_THREADS) + " worker threads...");
        for ( std::size_t i = 0; i < Configuration::MAX_THREADS; i += 1 ) {
            auto ctx = Framework::newThread([this]() -> int {
                Console::get()->debug("Started worker thread.");

                while ( !_shouldExit ) {
                    tryToProcessJob();
                    std::this_thread::sleep_for(std::chrono::microseconds(Configuration::QUEUE_SLEEP_uS));
                }

                return 0;
            });

            _threads.push_back(ctx);
        }

        // Trigger the worker threads to exit when the main thread shuts down.
        Framework::onShutdown([this]() {
            _shouldExit = true;
        });
    }

    std::pair<IQueueJob*, QueueContextID> Queue::popForProcessing() {
        std::unique_lock<std::mutex> queueLock(_queueMutex);

        // Try to pop a job from the current context first
        auto job = popForProcessingFromContext(_context);
        if ( job != nullptr ) {
            return {job, _context};
        }

        // Otherwise, run any jobs pending in other contexts
        for ( const auto& pair : _contextQueues ) {
            auto otherJob = popForProcessingFromContext(pair.first);
            if ( otherJob != nullptr ) {
                return {otherJob, pair.first};
            }
        }

        return {nullptr, _context};
    }

    IQueueJob* Queue::popForProcessingFromContext(const QueueContextID& context) {
        auto queueIter = _contextQueues.find(context);
        if ( queueIter == _contextQueues.end() || queueIter->second.empty() ) {
            return nullptr;
        }

        auto job = queueIter->second.front();
        queueIter->second.pop();
        _contextJobsInProgress[context] += 1;

        return job;
    }

    void Queue::decrementProcessingCount(const QueueContextID& context) {
        std::unique_lock<std::mutex> queueLock(_queueMutex);
        _contextJobsInProgress[context] -= 1;
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
