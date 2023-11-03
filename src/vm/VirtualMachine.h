#ifndef SWARMVM_VIRTUALMACHINE
#define SWARMVM_VIRTUALMACHINE

#include <chrono>
#include <thread>
#include <utility>
#include "../Configuration.h"
#include "../shared/nslib.h"
#include "runtime/interfaces.h"
#include "runtime/runtime_functions.h"
#include "runtime/runtime_provider.h"
#include "runtime/fabric.h"
#include "runtime/local_streams.h"
#include "runtime/State.h"
#include "walk/ExecuteWalk.h"
#include "debug/Debugger.h"
#include "../errors/RuntimeError.h"
#include "Wire.h"

using namespace nslib;

namespace swarmc::Runtime {
    class ProviderModule;
    class VirtualMachine;

    class VirtualMachineEvent : public event::Event {
    public:
        explicit VirtualMachineEvent(VirtualMachine* vm) : event::Event(), _vm(vm) {}

        [[nodiscard]] VirtualMachine* getVM() const {
            return _vm;
        }
    private:
        VirtualMachine* _vm;
    };

    class VirtualMachineInitializingEvent : public VirtualMachineEvent {
    public:
        explicit VirtualMachineInitializingEvent(VirtualMachine* vm) : VirtualMachineEvent(vm) {}

        [[nodiscard]] std::string toString() const override {
            return "VirtualMachineInitializingEvent<>";
        }
    };

    /** The Swarm runtime virtual machine (aka "the runtime"). */
    class VirtualMachine : public IStringable, public IUsesLogger {
    public:
        explicit VirtualMachine(IGlobalServices* global) : IUsesLogger("vm"), _global(useref(global)) {
            _exec = useref(new ExecuteWalk(this));
            _fabric = useref(new Fabric(_global));
            _queueContexts.push(_global->getUuid());
            _lifecycleEvents = useref(event::EventBus::getNew("swarmc::Runtime::VirtualMachine._lifecycleEvents"));
        }

        ~VirtualMachine() override {
            freeref(_global);
            freeref(_exec);
            freeref(_fabric);
            freeref(_lifecycleEvents);
        }

        [[nodiscard]] std::string toString() const override {
            std::string adv = "yes";
            if ( !_shouldAdvance ) adv = "no";

            return "Runtime::VirtualMachine<shouldAdvance: " + adv + ">";
        }

        /** Get the IGlobalServices used by the runtime. */
        [[nodiscard]] virtual IGlobalServices* global() const { return _global; }

        /** Get the Fabric driver used by the runtime. */
        [[nodiscard]] virtual Fabric* fabric() const { return _fabric; }

        [[nodiscard]] virtual event::EventBus* lifecycle() const { return _lifecycleEvents; }

        /** Load a set of parsed instructions into the runtime. */
        void initialize(ISA::Instructions is) {
            _state = useref(new State(std::move(is)));
            _scope = useref(new ScopeFrame(_global, nslib::uuid(), nullptr));
            freeref(_localOut);
            _localOut = useref(new LocalOutputStream());

            freeref(_localErr);
            _localErr = useref(new LocalErrorStream());

            clearStores();

            VirtualMachineInitializingEvent event(this);
            _lifecycleEvents->dispatch(event);
        }

        void initializeWorker() {
            // _state and _scope need to exist but their values don't matter
            _state = useref(new State(ISA::Instructions()));
            _scope = useref(new ScopeFrame(_global, nslib::uuid(), nullptr));

            freeref(_localOut);
            _localOut = useref(new LocalOutputStream());

            freeref(_localErr);
            _localErr = useref(new LocalErrorStream());

            VirtualMachineInitializingEvent event(this);
            _lifecycleEvents->dispatch(event);
        }

        /**
         * Configure the runtime to use the given storage driver.
         * The runtime will give higher priority to an interface the later it is added.
         */
        void addStore(IStorageInterface* store) {
            _stores.push_back(useref(store));
        }

        /**
         * Drop the contents of every registered store.
         */
        void clearStores() {
            for ( auto store : _stores ) store->clear();
        }

        /**
         * Configure the runtime to use the given queue driver.
         * The runtime will give higher priority to an interface the later it is added.
         */
        void addQueue(IQueue* queue) {
            _queues.push_back(useref(queue));
        }

        /**
         * Configure the runtime to use the given resource provider.
         * The runtime will give higher priority to a provider the later it is added.
         */
        void addProvider(IProvider* provider) {
            _providers.push_back(useref(provider));
        }

        void addExternalProvider(dynamic::Module<ProviderModule>* m);

        void addExternalProvider(const std::string& path) {
            addExternalProvider(new dynamic::Module<ProviderModule>(path));
        }

        void useStreamDriver(IStreamDriver* driver) {
            _streams = useref(driver);
        }

        /** Reset the VM to an uninitialized state. */
        void cleanup() {
            freeref(_state);
            _state = nullptr;

            freeref(_scope);
            _scope = nullptr;

            freeref(_streams);
            _streams = nullptr;

            freeref(_sharedErr);
            _sharedErr = nullptr;

            freeref(_sharedOut);
            _sharedOut = nullptr;

            freeref(_debugger);
            _debugger = nullptr;

            for ( auto lock : _locks ) {
                lock->release();
                freeref(lock);
            }
            _locks.clear();

            for ( auto store : _stores ) freeref(store);
            _stores.clear();

            for ( auto provider : _providers ) freeref(provider);
            _providers.clear();

            for ( auto queue : _queues ) freeref(queue);
            _queues.clear();

            freeref(_localErr);
            freeref(_localOut);
        }

        virtual void restore(ScopeFrame*);

        /** Restore the VM to the specified state. Can be used to re-hydrate VMs for queued call execution. */
        virtual void restore(ScopeFrame*, State*);

        virtual void attachDebugger(Debug::Debugger* debugger) {
            _debugger = useref(debugger);
            _debugger->setMetadata(_state->getMetadata());
        }

        /** Load the value stored in the given location. */
        virtual ISA::Reference* loadFromStore(ISA::LocationReference*);

        virtual ISA::StreamReference* loadBuiltinStream(ISA::LocationReference*);

        /** Load the function stored in the given location. */
        virtual ISA::FunctionReference* loadFunction(ISA::LocationReference*);

        virtual ISA::FunctionReference* loadFunction(FunctionBackend, const std::string&);

        /** Load the inline function with the given name. */
        virtual InlineFunction* loadInlineFunction(const std::string& name);

        /** Load the external provider function with the given name. */
        virtual IProviderFunction* loadProviderFunction(const std::string& name);

        /**
         * Resolve a reference from an instruction to its most primitive value.
         * e.g. will take a LocationReference to a StringReference
         */
        virtual ISA::Reference* resolve(ISA::Reference*);

        virtual InlineRefHandle<ISA::Reference> resolvei(ISA::Reference*);

        /** Get the instruction which will be executed during this step. */
        virtual ISA::Instruction* current();

        /** Execute a the current instruction, and advance to the next one. */
        virtual void step();

        /** Advance to the next instruction. */
        virtual void advance();

        /** Rewind to the previous instruction. */
        virtual void rewind();

        /** Execute the loaded program to completion. */
        virtual void execute();

        /** Store the given reference in the specified location using the appropriate storage driver. */
        virtual void store(ISA::LocationReference*, ISA::Reference*);

        /** Returns true if this VM instance holds a lock for the given location. */
        virtual bool hasLock(ISA::LocationReference*);

        /** Attempt to acquire a lock for the given location. */
        virtual bool lock(ISA::LocationReference*);

        /** Release the lock for the given location, if this VM holds it. */
        virtual void unlock(ISA::LocationReference*);

        /** Assert the type of the specified location in the appropriate storage driver. */
        virtual void typify(ISA::LocationReference*, Type::Type*);

        /** Shadow the given variable in the current scope. */
        virtual void shadow(ISA::LocationReference*);

        /** Create a new scope frame and make it the current scope. */
        virtual void enterScope();

        /** Create a new scope frame (adding the IFunctionCall to the call stack), and make it the current scope. */
        virtual void enterCallScope(IFunctionCall*);

        virtual void inheritCallScope(IFunctionCall*);

        /** Pop the current scope frame and return to its parent. */
        virtual void exitScope();

        /** Perform the specified function call using the appropriate FunctionBackend. */
        virtual void call(IFunctionCall*);

        virtual void callWithInheritedScope(IFunctionCall*);

        /**
         * Perform the specified function call using the appropriate FunctionBackend, then
         * immediately stop execution. (Useful for queue workers which perform a deferred
         * call, then stop.)
         */
        virtual void executeCall(IFunctionCall*);

        /**
         * Advance to the end of the given function declaration.
         * Used to skip over function bodies encountered during normal execution, not calls.
         */
        virtual void skip(ISA::BeginFunction*);

        /** Get the current function call, if one exists. Otherwise, `nullptr`. */
        virtual IFunctionCall* getCall();

        /**
         * Get the function call from which we just returned, if one exists. Otherwise, `nullptr`.
         * Used by the runtime to capture return values.
         */
        virtual IFunctionCall* getReturn();

        /** Get the current program state, if on exists. Otherwise, `nullptr`. */
        virtual State* getState() const;

        /** Get the current scope frame, if on exists. Otherwise, `nullptr`. */
        virtual ScopeFrame* getScopeFrame() const;

        /**
         * Return capture is used when the runtime needs to temporarily store the return value
         * of a call so it can be used by the instruction the call returns to. The canonical
         * example of this is the AssignEval instruction. The runtime "captures" the return
         * value of the RHS when the RHS is a call so the value can be used to perform the
         * assignment.
         */
        virtual void setCaptureReturn();

        /** Defer the given function call onto the queue. */
        virtual IQueueJob* pushCall(IFunctionCall*);

        /** Wait for all jobs in the current queue context to finish. */
        virtual void drain();

        /** Immediately stop execution. */
        virtual void exit();

        /** Enter a new queue context. */
        virtual void enterQueueContext();

        /** Exit the current queue context, returning to the previous one. */
        virtual void exitQueueContext();

        /**
         * Perform a function call return.
         * If `shouldJump` is true, the VM will jump to the position where the
         * call was performed (used during inline function execution).
         */
        virtual void returnToCaller(bool shouldJump);

        [[nodiscard]] virtual IStream* getLocalOutput() const { return _localOut; }

        [[nodiscard]] virtual IStream* getLocalError() const { return _localErr; }

        virtual IStream* getSharedOutput() {
            if ( _sharedOut == nullptr ) {
                _sharedOut = useref(_streams->open("s:STDOUT", Type::Primitive::of(Type::Intrinsic::STRING)));
            }

            return _sharedOut;
        }

        virtual IStream* getSharedError() {
            if ( _sharedErr == nullptr ) {
                _sharedErr = useref(_streams->open("s:STDERR", Type::Primitive::of(Type::Intrinsic::STRING)));
            }

            return _sharedErr;
        }

        virtual IStream* getStream(const std::string& id, Type::Type* innerType) {
            return _streams->open(id, innerType);
        }

        /**
         * This callback is executed each time the VM fails to acquire a lock,
         * before it retries its attempt.
         *
         * TODO: use to execute queue jobs while idle
         */
        virtual void whileWaitingForLock() {
            std::this_thread::sleep_for(std::chrono::microseconds(Configuration::LOCK_SLEEP_uS));
        }

        /**
         * This callback is executed each time the VM check to see if all pending
         * jobs have completed, and finds jobs not yet completed.
         *
         * TODO: use to execute queue jobs while idle
         */
        virtual void whileWaitingForDrain() {
            for ( auto q : _queues ) {
                q->tick();
            }
            std::this_thread::sleep_for(std::chrono::microseconds(Configuration::LOCK_SLEEP_uS));
        }

        virtual ExceptionHandlerId pushExceptionHandler(IFunction* selector, IFunction* handler) {
            return _scope->pushExceptionHandler(selector, handler);
        }

        virtual ExceptionHandlerId pushExceptionHandler(std::size_t code, IFunction* handler) {
            return _scope->pushExceptionHandler(code, handler);
        }

        virtual ExceptionHandlerId pushExceptionHandler(IFunction* handler) {
            return _scope->pushExceptionHandler(handler);
        }

        virtual void popExceptionHandler(const ExceptionHandlerId& id) {
            _scope->popExceptionHandler(id);
        }

        virtual std::pair<ScopeFrame*, IFunction*> getExceptionHandler(std::size_t code);

        virtual void raise(std::size_t code);

        virtual ScopeFrame* getExceptionFrame();

        /** Make a deep copy of this instance. */
        [[nodiscard]] VirtualMachine* copy() const {
            auto copy = new VirtualMachine(_global);
            copy->_state = useref(_state->copy());
            copy->_scope = useref(_scope->copy());
            copy->_queueContexts = _queueContexts;
            copy->_streams = useref(_streams);
            copy->_localOut = useref(new LocalOutputStream());
            copy->_localErr = useref(new LocalErrorStream());
            copy->_sharedOut = useref(_sharedOut);
            copy->_sharedErr = useref(_sharedErr);

            copy->_queues = _queues;
            for ( auto e : _queues ) useref(e);

            copy->_locks = _locks;
            for ( auto e : _locks ) useref(e);

            copy->_providers = _providers;
            for ( auto e : _providers ) useref(e);

            Stores stores;
            for ( auto store : _stores ) stores.push_back(useref(store->copy()));
            copy->_stores = stores;

            return copy;
        }

        void copy(const std::function<void(VirtualMachine*)>& handler) const {
            auto vm = copy();
            handler(vm);
            vm->cleanup();
            delete vm;
        }

        template <typename ReturnT>
        ReturnT copy(const std::function<ReturnT(VirtualMachine*)>& handler) const {
            auto vm = copy();
            auto ret = handler(vm);
            delete vm;
            return ret;
        }

    protected:
        IGlobalServices* _global;
        State* _state = nullptr;
        Stores _stores;
        Queues _queues;
        Locks _locks;
        Providers _providers;
        std::vector<dynamic::Module<ProviderModule>*> _externalProviders;
        IStreamDriver* _streams = nullptr;
        ScopeFrame* _scope = nullptr;
        ExecuteWalk* _exec = nullptr;
        Fabric* _fabric = nullptr;
        std::stack<QueueContextID> _queueContexts;
        IFunctionCall* _return = nullptr;
        Debug::Debugger* _debugger = nullptr;
        IStream* _localOut = nullptr;
        IStream* _localErr = nullptr;
        IStream* _sharedOut = nullptr;
        IStream* _sharedErr = nullptr;
        event::EventBus* _lifecycleEvents = nullptr;
        bool _shouldClearReturn = false;
        bool _shouldAdvance = true;
        bool _shouldRunToCompletion = false;

        /** Print output visible by default in the debug binary. */
        virtual void debug(const std::string& output) const {
            logger->debug(output);
        }

        /** Print verbose output visible when the `--verbose` flag is present. */
        virtual void verbose(const std::string& output) const {
            if ( Configuration::VERBOSE ) debug(output);
        }

        /** Get the storage driver which should be used to access the given location. */
        virtual IStorageInterface* getStore(ISA::LocationReference*);

        /** Get the queue which should be used to perform the given function call. */
        virtual IQueue* getQueue(IFunctionCall*);

        /** Validate the given function call (e.g. type check params). */
        virtual void checkCall(IFunctionCall*);

        virtual void call(IFunctionCall*, bool inheritScope);

        /** Perform an inline function call. */
        virtual void callInlineFunction(InlineFunctionCall*, bool inheritScope);

        /** Perform an external provider function call. */
        virtual void callProviderFunction(IProviderFunctionCall*, bool inheritScope);

        virtual std::pair<SchedulingFilters, bool> buildResourceFilters(const Resources&);

        virtual bool isBuiltinStream(ISA::LocationReference*);

        friend class Debug::Debugger;
    };

}

#endif //SWARMVM_VIRTUALMACHINE
