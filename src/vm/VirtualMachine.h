#ifndef SWARMVM_VIRTUALMACHINE
#define SWARMVM_VIRTUALMACHINE

#include <chrono>
#include <thread>
#include <utility>
#include "../Configuration.h"
#include "../shared/IStringable.h"
#include "../shared/util/Console.h"
#include "../shared/uuid.h"
#include "runtime/interfaces.h"
#include "runtime/runtime_functions.h"
#include "runtime/State.h"
#include "walk/ExecuteWalk.h"

namespace swarmc::Runtime {

    class VirtualMachine : public IStringable, public IUsesConsole {
    public:
        VirtualMachine(IGlobalServices* global) : IUsesConsole(), _global(global) {
            _exec = new ExecuteWalk(this);
            _queueContexts.push(_global->getUuid());
        }

        virtual ~VirtualMachine() {
            delete _exec;
        }

        virtual IGlobalServices* global() const { return _global; }

        void initialize(ISA::Instructions is) {
            _state = new State(std::move(is));
            _scope = new ScopeFrame(util::uuid4(), nullptr);
        }

        void addStore(IStorageInterface* store) {
            _stores.push_back(store);
        }

        void cleanup() {
            if ( _state != nullptr ) delete _state;
            _state = nullptr;

            if ( _scope != nullptr ) delete _scope;
            _scope = nullptr;

            for ( auto lock : _locks ) lock->release();
            _locks.clear();

            _stores.clear();
        }

        virtual ISA::Reference* load(ISA::LocationReference*);

        virtual ISA::Reference* resolve(ISA::Reference*);

        virtual void step();

        virtual void rewind();

        virtual void execute();

        virtual void store(ISA::LocationReference*, ISA::Reference*);

        virtual bool hasLock(ISA::LocationReference*);

        virtual void lock(ISA::LocationReference*);

        virtual void unlock(ISA::LocationReference*);

        virtual void typify(ISA::LocationReference*, const Type::Type*);

        virtual void shadow(ISA::LocationReference*);

        virtual void enterScope();

        virtual void enterCallScope(IFunctionCall*);

        virtual void exitScope();

        virtual void call(IFunctionCall*);

        virtual IFunctionCall* getCall();

        virtual IQueueJob* pushCall(IFunctionCall*);

        virtual void drain();

        virtual void enterQueueContext();

        virtual void exitQueueContext();

        virtual void returnToCaller();

        virtual bool hasFlag(StateFlag) const;

        virtual void whileWaitingForLock() {
            std::this_thread::sleep_for(std::chrono::milliseconds(Configuration::LOCK_SLEEP_uS));
        }

        virtual void whileWaitingForDrain() {
            std::this_thread::sleep_for(std::chrono::milliseconds(Configuration::LOCK_SLEEP_uS));
        }

    protected:
        IGlobalServices* _global;
        State* _state = nullptr;
        Stores _stores;
        Queues _queues;
        Locks _locks;
        ScopeFrame* _scope;
        ExecuteWalk* _exec;
        std::stack<QueueContextID> _queueContexts;

        virtual IStorageInterface* getStore(ISA::LocationReference*);

        virtual IQueue* getQueue(IFunctionCall*);

        virtual void callInlineFunction(InlineFunctionCall*);

        virtual void callBuiltinFunction(BuiltinFunctionCall*);
    };

}

#endif //SWARMVM_VIRTUALMACHINE
