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

    void VirtualMachine::restore(ScopeFrame* scope) {
        if ( _scope != scope ) delete _scope;  // FIXME: delete parent chain?
        _scope = scope;
    }

    void VirtualMachine::restore(ScopeFrame* scope, State* state) {
        delete _scope;
        _scope = scope;

        delete _state;
        _state = state;
    }

    Reference* VirtualMachine::loadFromStore(LocationReference* loc) {
        auto scopeLoc = _scope->map(loc);
        auto store = getStore(scopeLoc);
        if ( !store->has(scopeLoc) ) {
            throw Errors::SwarmError("Attempted to load undefined location (" + scopeLoc->toString() + ") from store (" + store->toString() + ")");
        }

        return store->load(scopeLoc);
    }

    StreamReference* VirtualMachine::loadBuiltinStream(LocationReference* ref) {
        if ( ref->affinity() == Affinity::SHARED && ref->name() == "STDERR" ) {
            return new StreamReference(getSharedError());
        } else if ( ref->affinity() == Affinity::SHARED && ref->name() == "STDOUT" ) {
            return new StreamReference(getSharedOutput());
        } else if ( ref->affinity() == Affinity::LOCAL && ref->name() == "STDERR" ) {
            return new StreamReference(getLocalError());
        } else if ( ref->affinity() == Affinity::LOCAL && ref->name() == "STDOUT" ) {
            return new StreamReference(getLocalOutput());
        }

        throw Errors::SwarmError("Attempted to load invalid built-in stream: " + ref->toString());
    }

    FunctionReference* VirtualMachine::loadFunction(LocationReference* loc) {
        if ( _state->hasInlineFunction(loc->name()) ) {
            return new FunctionReference(loadInlineFunction(loc->name()));
        }

        auto providerFn = loadProviderFunction(loc->name());
        if ( providerFn != nullptr ) return new FunctionReference(providerFn);

        throw Errors::SwarmError("Attempted to load invalid function " + loc->toString() + ". Location has an unknown function backend, or is an invalid name.");
    }

    FunctionReference* VirtualMachine::loadFunction(FunctionBackend backend, const std::string& name) {
        if ( backend == FunctionBackend::INLINE ) {
            return new FunctionReference(loadInlineFunction(name));
        }

        if ( backend == FunctionBackend::PROVIDER ) {
            return new FunctionReference(loadProviderFunction(name));
        }

        throw Errors::SwarmError("Attempted to load invalid function " + name + ". Location has an unknown function backend, or is an invalid name.");
    }

    InlineFunction* VirtualMachine::loadInlineFunction(const std::string& name) {
        auto pc = _state->getInlineFunctionPC(name);

        FormalTypes paramTypes;
        for ( auto param : _state->loadInlineFunctionParams(pc) ) {
            auto paramType = _exec->ensureType(resolve(param->first()));
            paramTypes.push_back(paramType->value());
        }

        auto header = _state->getInlineFunctionHeader(pc);
        auto returnType = _exec->ensureType(resolve(header->second()));

        verbose("load inline function: " + name + " (#params: " + std::to_string(paramTypes.size()) + ") (returns: " + returnType->toString() + ")");
        return new InlineFunction(name, paramTypes, returnType->value());
    }

    IProviderFunction* VirtualMachine::loadProviderFunction(const std::string& name) {
        size_t idx = 0;
        for ( auto it = _providers.rbegin(); it != _providers.rend(); ++it, --idx ) {
            auto provider = _providers.at(idx);
            auto fn = provider->loadFunction(name);
            if ( fn != nullptr ) return fn;
        }
        return nullptr;
    }

    Reference* VirtualMachine::resolve(Reference* ref) {
        if ( ref->tag() == ReferenceTag::LOCATION ) {
            auto loc = (LocationReference*) ref;
            if ( loc->affinity() == Affinity::FUNCTION ) return loadFunction(loc);
            if ( isBuiltinStream(loc) ) return loadBuiltinStream(loc);
            return loadFromStore(loc);
        }

        return ref;
    }

    Instruction* VirtualMachine::current() {
        return _state->current();
    }

    void VirtualMachine::step() {
        _exec->walkOne(_state->current());

        if ( !_state->isEndOfProgram() && _shouldAdvance ) {
            advance();
        }
        _shouldAdvance = true;

        if ( _shouldClearReturn ) {
            _scope->clearReturnCall();
            _scope->shouldCaptureReturn(false);
            _shouldClearReturn = false;
        } else if ( _scope->getReturnCall() != nullptr ) {
            _shouldClearReturn = true;
        }
    }

    void VirtualMachine::advance() {
        _state->advance();
    }

    void VirtualMachine::rewind() {
        _state->rewind();
    }

    void VirtualMachine::execute() {
        _shouldRunToCompletion = true;
        while ( !_state->isEndOfProgram() ) step();
    }

    void VirtualMachine::store(LocationReference* loc, Reference* ref) {
        auto scopeLoc = _scope->map(loc);
        getStore(scopeLoc)->store(scopeLoc, ref);
    }

    bool VirtualMachine::hasLock(LocationReference* loc) {
        return std::any_of(_locks.begin(), _locks.end(), [loc](IStorageLock* lock) {
            return lock->location()->is(loc);
        });
    }

    void VirtualMachine::lock(LocationReference* loc) {
        if ( hasLock(loc) ) {
            console->warn("Attempted to acquire lock that is already held by the requesting control: " + loc->toString());
            return;  // FIXME: should this raise an error?
        }

        auto scopeLoc = _scope->map(loc);
        auto store = getStore(scopeLoc);
        for ( int i = 0; i < Configuration::LOCK_MAX_RETRIES; i += 1 ) {
            auto lock = store->acquire(scopeLoc);
            if ( lock == nullptr ) {
                whileWaitingForLock();
                continue;
            }

            _locks.push_back(lock);
            return;
        }

        // FIXME: should this throw a runtime error?
        throw Errors::SwarmError("Unable to acquire lock for location (" + scopeLoc->toString() + ") from store (" + store->toString() + ") -- max retries exceeded");
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

    void VirtualMachine::inheritCallScope(IFunctionCall* call) {
        _scope = _scope->overrideCall(call);
    }

    void VirtualMachine::exitScope() {
        // FIXME - this likely needs to do better cleanup work
        auto old = _scope;
        _scope = _scope->parent();
        delete old;
    }

    void VirtualMachine::call(IFunctionCall* fc) {
        return call(fc, false);
    }

    void VirtualMachine::callWithInheritedScope(IFunctionCall* fc) {
        return call(fc, true);
    }

    void VirtualMachine::call(IFunctionCall* call, bool inheritScope) {
        _shouldAdvance = false;
        if ( call->backend() == FunctionBackend::INLINE ) return callInlineFunction((InlineFunctionCall*) call, inheritScope);
        if ( call->backend() == FunctionBackend::PROVIDER ) return callProviderFunction((IProviderFunctionCall*) call, inheritScope);
        throw Errors::SwarmError("Cannot call function `" + call->toString() + "` (invalid backend)");
    }

    void VirtualMachine::executeCall(IFunctionCall* c) {
        auto singleStep = _debugger != nullptr && _debugger->isInteractive() && !_shouldRunToCompletion;

        call(c);

        // `call()` sets this to false because most calls are set up from w/in other instructions
        // In that case, we don't want to advance at the end of the current instruction, since the
        // call already does that for us.
        // However, `executeCall()` is called from outside the normal instruction pipeline, so we
        // need to allow `step()` to advance, otherwise the first instruction will be executed
        // twice.
        _shouldAdvance = !singleStep;  // fixme: !isInteractive

        // FIXME: need to allow debugger to step through this
        while ( !_state->isEndOfProgram() && !c->hasReturned() && !singleStep ) step();
    }

    void VirtualMachine::skip(ISA::BeginFunction* fn) {
        auto pc = _state->getInlineFunctionSkipPC(fn->first()->name());
        verbose("Skipping uncalled function body: " + fn->toString() + ", pc: " + std::to_string(pc));
        _state->jump(pc);
    }

    IFunctionCall* VirtualMachine::getCall() {
        return _scope->call();
    }

    IFunctionCall* VirtualMachine::getReturn() {
        return _scope->getReturnCall();
    }

    void VirtualMachine::setCaptureReturn() {
        _scope->shouldCaptureReturn(true);
    }

    IQueueJob* VirtualMachine::pushCall(IFunctionCall* call) {
        // fixme: need to account for contexts!
        auto queue = getQueue(call);
        auto job = queue->build(call, _scope, _state);
        verbose("pushCall - call: " + call->toString() + " | job: " + job->toString());
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

    void VirtualMachine::exit() {
        _state->jumpEnd();
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

    void VirtualMachine::returnToCaller(bool shouldJump) {
        auto call = getCall();
        if ( call == nullptr ) {
            throw Errors::SwarmError("Unable to return from caller: no call in progress.");
        }

        // Mark the call as returned
        call->setReturned();

        // Jump back to the caller's site
        if ( shouldJump ) _scope = _state->jumpReturn(_scope);
        else exitScope();

        // Make the returned IFunctionCall available to the caller
        if ( _scope->shouldCaptureReturn() ) {
            _scope->setReturnCall(call);
        }
    }

    std::pair<ScopeFrame*, IFunction*> VirtualMachine::getExceptionHandler(size_t code) {
        auto scope = _scope;
        while ( scope != nullptr ) {
            auto handlers = scope->getExceptionHandlers();
            while ( !handlers.empty() ) {
                auto handler = handlers.top();
                handlers.pop();

                // If the handler is universal, it fits
                if ( exceptionHandlerIsUniversal(handler) ) {
                    return {scope, unpackExceptionHandler(handler)};
                }

                // If the handler defines a specific code, it fits
                if ( exceptionHandlerIsCode(handler, code) ) {
                    return {scope, unpackExceptionHandler(handler)};
                }

                // If the handler defines a discriminator function, and that function is satisfied, it fits
                auto discriminatorFn = unpackExceptionHandlerDiscriminator(handler);
                if ( discriminatorFn != nullptr ) {
                    auto call = discriminatorFn->curry(new NumberReference(static_cast<double>(code)))->call();

                    copy([call](VirtualMachine* vm) {
                        vm->executeCall(call);
                    });

                    // the handler is type-checked before being pushed
                    assert(call->returnType()->isAssignableTo(Type::Primitive::of(Type::Intrinsic::BOOLEAN)));
                    auto result = (BooleanReference*) call->getReturn();
                    auto resultValue = result->value();
                    delete result;

                    if ( resultValue ) {
                        return {scope, unpackExceptionHandler(handler)};
                    }
                }
            }

            scope = scope->parent();
        }

        return {nullptr, nullptr};
    }

    void VirtualMachine::raise(size_t code) {
        auto handler = getExceptionHandler(code);

        if ( handler.first == nullptr || handler.second == nullptr ) {
            throw Errors::SwarmError("Unhandled runtime exception: " + s(code));
        }

        // Rewind to the scope of the selected exception handler
        restore(handler.first->copy()->asExceptionFrame());

        // The exception handler is responsible for using the `resume` instruction
        // to jump back to the correct context. If it doesn't assume that was a mistake
        // and halt execution.
        exit();

        // Call the exception handler
        call(handler.second->curry(new NumberReference(static_cast<double>(code)))->call());
    }

    ScopeFrame* VirtualMachine::getExceptionFrame() {
        auto scope = _scope;
        while ( scope != nullptr && !scope->isExceptionFrame() )
            scope = scope->parent();
        return scope;
    }

    void VirtualMachine::checkCall(IFunctionCall* call) {
        auto vector = call->vector();
        for ( auto pair : vector ) {
            auto type = pair.first;
            auto ref = pair.second;
            if ( !ref->type()->isAssignableTo(type) ) {
                // FIXME: eventually, this needs to raise a runtime exception
                throw Errors::SwarmError("Unable to make function call (" + call->toString() + ") - argument " + ref->toString() + " is not assignable to type " + type->toString());
            }
        }
    }

    void VirtualMachine::callInlineFunction(InlineFunctionCall* call, bool inheritScope) {
        // Validate the inline function location
        auto pc = _state->getInlineFunctionPC(call->name());

        // Type check the parameters
        checkCall(call);

        // Jump to the function call
        debug("inline call: " + s(call) + " (pc: " + s(pc) + ")");
        if ( inheritScope ) _state->jump(pc);
        else _state->jumpCall(_scope, pc);

        // Start a new scope
        if ( inheritScope ) inheritCallScope(call);
        else enterCallScope(call);

        // Skip over the beginfn instruction
        advance();

        verbose("next instruction for inline call: " + _state->current()->toString());
    }

    void VirtualMachine::callProviderFunction(IProviderFunctionCall* call, bool inheritScope) {
        // Type check the parameters
        checkCall(call);

        // Start a new scope
        if ( inheritScope ) inheritCallScope(call);
        else enterCallScope(call);

        // Invoke the provider to execute the function call
        debug("provider call: " + call->toString());
        call->provider()->call(call);

        // Skip over the call instruction
        advance();

        // Immediately return to the previous control
        returnToCaller(false);
    }

    bool VirtualMachine::isBuiltinStream(LocationReference* ref) {
        if ( ref->affinity() != Affinity::SHARED && ref->affinity() != Affinity::LOCAL ) {
            return false;
        }

        return ref->name() == "STDERR" || ref->name() == "STDOUT";
    }
}
