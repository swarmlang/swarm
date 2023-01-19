#include <cassert>
#include "swarm_thread.h"
#include "multi_threaded.h"
#include "../VirtualMachine.h"
#include "../../errors/ClearLockedReferences.h"

namespace swarmc::Runtime::MultiThreaded {

    size_t SwarmThread::CurrentThreads = 0;

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
        if ( SwarmThread::moreThreads() ) {
            SwarmThread::execute(_vm, job);
        } else {
            auto qlock = new std::lock_guard<std::mutex>(_qtex);
            _queue.push(job);
            delete qlock;
        }
    }
    
    IQueueJob* Queue::pop() {
        auto lock = new std::lock_guard<std::mutex>(_qtex);
        assert(!_queue.empty());
        auto job = _queue.front();
        _queue.pop();
        delete lock;
        return job;
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