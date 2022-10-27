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

namespace swarmc::Runtime {

    class VirtualMachine : public IStringable, public IUsesConsole {
    public:
        VirtualMachine() : IUsesConsole() {}
        virtual ~VirtualMachine() = default;

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

        virtual void store(ISA::LocationReference*, ISA::Reference*);

        virtual bool hasLock(ISA::LocationReference*);

        virtual void lock(ISA::LocationReference*);

        virtual void unlock(ISA::LocationReference*);

        virtual void typify(ISA::LocationReference*, const Type::Type*);

        virtual void shadow(ISA::LocationReference*);

        virtual void enterScope();

        virtual void exitScope();

        virtual void call(IFunctionCall*) { /* FIXME */ };

        virtual void pushCall(IFunctionCall*) { /* FIXME */ };

        virtual void whileWaitingForLock() {
            std::this_thread::sleep_for(std::chrono::milliseconds(Configuration::LOCK_SLEEP_uS));
        }

    protected:
        State* _state = nullptr;
        Stores _stores;
        Locks _locks;
        ScopeFrame* _scope;

        virtual IStorageInterface* getStore(ISA::LocationReference*);
    };

}

#endif //SWARMVM_VIRTUALMACHINE
