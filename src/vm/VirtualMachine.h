#ifndef SWARMVM_VIRTUALMACHINE
#define SWARMVM_VIRTUALMACHINE

#include "../shared/IStringable.h"
#include "../shared/util/Console.h"
#include "../shared/uuid.h"
#include "runtime/interfaces.h"
#include "runtime/State.h"

namespace swarmc::Runtime {

    class VirtualMachine : public IStringable, public IUsesConsole {
    public:
        VirtualMachine() : IUsesConsole() {}
        virtual ~VirtualMachine() = default;

        void initialize(ISA::Instructions is) {
            _state = new State(is);
            _scope = new ScopeFrame(util::uuid4(), nullptr);
        }

        void addStore(IStorageInterface* store) {
            _stores.push_back(store);
        }

        void cleanup() {
            if ( _state != nullptr ) delete _state;
            if ( _scope != nullptr ) delete _scope;
            _state = nullptr;
            _scope = nullptr;
            _stores.clear();
        }

        virtual ISA::Reference* load(ISA::LocationReference*);

        virtual ISA::Reference* resolve(ISA::Reference*);

    protected:
        State* _state = nullptr;
        Stores _stores;
        ScopeFrame* _scope;

        virtual IStorageInterface* getStore(ISA::LocationReference*);
    };

}

#endif //SWARMVM_VIRTUALMACHINE
