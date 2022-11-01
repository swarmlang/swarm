#include <cassert>
#include "../errors/SwarmError.h"
#include "VirtualMachine.h"

namespace swarmc::Runtime {
    using namespace swarmc::ISA;

    IStorageInterface* VirtualMachine::getStore(LocationReference* loc) {
        Stores::size_type idx = _stores.size() - 1;
        for ( auto it = _stores.rbegin(); it != _stores.rend(); ++it, --idx ) {
            auto store = *it;
            if ( store->manages(loc) ) {
                return _stores[idx];
            }
        }

        throw Errors::SwarmError("Unable to find storage backend for location: " + loc->toString());
    }

    IQueue* VirtualMachine::getQueue(IFunctionCall* j) {
        Queues::size_type idx = _queues.size() - 1;
        for ( auto it = _queues.rbegin(); it != _queues.rend(); ++it, --idx ) {
            auto queue = *it;
            if ( queue->shouldHandle(j) ) {
                return _queues[idx];
            }
        }

        throw Errors::SwarmError("Unable to find queue backend for job: " + j->toString());
    }

    Reference* VirtualMachine::load(LocationReference* loc) {
        auto store = getStore(loc);
        if ( !store->has(loc) ) {
            throw Errors::SwarmError("Attempted to load undefined location (" + loc->toString() + ") from store (" + store->toString() + ")");
        }

        return store->load(loc);
    }

    Reference* VirtualMachine::resolve(Reference* ref) {
        if ( ref->tag() == ReferenceTag::LOCATION ) {
            return load((LocationReference*) ref);
        }

        return ref;
    }

    void VirtualMachine::step() {
        _exec->walkOne(_state->current());
        _state->advance();
    }

    void VirtualMachine::rewind() {
        _state->rewind();
    }

    void VirtualMachine::execute() {
        while ( !_state->isEndOfProgram() ) step();
    }

    void VirtualMachine::store(LocationReference* loc, Reference* ref) {
        getStore(loc)->store(loc, ref);
    }

    bool VirtualMachine::hasLock(LocationReference* loc) {
        for ( auto lock : _locks ) {
            if ( lock->location()->is(loc) ) {
                return true;
            }
        }
        return false;
    }

    void VirtualMachine::lock(LocationReference* loc) {
        if ( hasLock(loc) ) {
            console->warn("Attempted to acquire lock that is already held by the requesting control: " + loc->toString());
            return;  // FIXME: should this raise an error?
        }

        auto store = getStore(loc);
        for ( int i = 0; i < Configuration::LOCK_MAX_RETRIES; i += 1 ) {
            auto lock = store->acquire(loc);
            if ( lock == nullptr ) {
                whileWaitingForLock();
                continue;
            }

            _locks.push_back(lock);
            return;
        }

        // FIXME: should this throw a runtime error?
        throw Errors::SwarmError("Unable to acquire lock for location (" + loc->toString() + ") from store (" + store->toString() + ") -- max retries exceeded");
    }

    void VirtualMachine::unlock(LocationReference* loc) {
        if ( !hasLock(loc) ) {
            console->warn("Attempted to release lock that is not held by the requesting control: " + loc->toString());
            return;  // FIXME: should this raise an error?
        }

        for ( auto it = _locks.begin(); it != _locks.end(); ++it ) {
            auto lock = *it;
            if ( lock->location()->is(loc) ) {
                lock->release();
                _locks.erase(it);
                return;
            }
        }

        // FIXME: should this throw a runtime error?
        throw Errors::SwarmError("Unable to release lock for location (" + loc->toString() + ")");
    }

    void VirtualMachine::typify(ISA::LocationReference* loc, const Type::Type* type) {
        getStore(loc)->typify(loc, type);
    }

    void VirtualMachine::shadow(ISA::LocationReference* loc) {
        _scope->shadow(loc);
    }

    void VirtualMachine::enterScope() {
        _scope = _scope->newChild();
    }

    void VirtualMachine::enterCallScope(IFunctionCall* call) {
        _scope = _scope->newCall(call);
    }

    void VirtualMachine::exitScope() {
        // FIXME - this likely needs to do better cleanup work
        auto old = _scope;
        _scope = _scope->parent();
        delete old;
    }

    void VirtualMachine::call(IFunctionCall* call) {
        if ( call->backend() == FunctionBackend::INLINE ) return callInlineFunction((InlineFunctionCall*) call);
//        if ( call->backend() == FunctionBackend::BUILTIN ) return callBuiltinFunction((BuiltinFunctionCall*) call);
        throw Errors::SwarmError("Cannot call function `" + call->toString() + "` (invalid backend)");
    }

    IFunctionCall* VirtualMachine::getCall() {
        return _scope->call();
    }

    IQueueJob* VirtualMachine::pushCall(IFunctionCall* call) {
        // fixme: need to account for contexts!
        auto queue = getQueue(call);
        auto job = queue->build(call, _scope, _state);
        queue->push(job);
        return job;
    }

    void VirtualMachine::drain() {
        for ( auto queue : _queues ) {
            while ( !queue->isEmpty() ) {
                whileWaitingForDrain();
            }
        }
    }

    void VirtualMachine::enterQueueContext() {
        QueueContextID id = _global->getUuid();
        _queueContexts.push(id);

        for ( auto queue : _queues ) {
            queue->setContext(id);
        }
    }

    void VirtualMachine::exitQueueContext() {
        if ( _queueContexts.empty() ) {
            throw Errors::SwarmError("Attempted to exit from non-existent queue context.");
        }

        _queueContexts.pop();

        if ( _queueContexts.empty() ) {
            throw Errors::SwarmError("Exited from last queue context. Cannot continue.");
        }

        QueueContextID id = _queueContexts.top();
        for ( auto queue : _queues ) {
            queue->setContext(id);
        }
    }

    void VirtualMachine::returnToCaller() {
        // Pop the callee's scope
        exitScope();

        // Set the return flag so we can execute assignments properly
        _state->setFlag(StateFlag::JUMPED_FROM_RETURN);

        // Jump back to the caller's site
        _state->jumpReturn();
    }

    bool VirtualMachine::hasFlag(StateFlag f) const {
        return _state->hasFlag(f);
    }

    void VirtualMachine::callInlineFunction(InlineFunctionCall* call) {
        // Validate the inline function location
        auto pc = _state->getInlineFunctionPC(call->name());

        // Type check the parameters  // todo: generalize this
        auto vector = call->vector();
        for ( auto pair : vector ) {
            auto type = pair.first;
            auto ref = pair.second;
            if ( !ref->type()->isAssignableTo(type) ) {
                // FIXME: eventually, this needs to raise a runtime exception
                throw Errors::SwarmError("Unable to make function call (" + call->toString() + ") - argument " + ref->toString() + " is not assignable to type " + type->toString());
            }
        }

        // Start a new scope
        enterCallScope(call);

        // Jump to the function call
        _state->jumpCall(pc);
    }

    void VirtualMachine::callBuiltinFunction(BuiltinFunctionCall* call) {
        assert(false);
    }
}
