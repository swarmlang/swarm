#ifndef SWARM_WIRE_H
#define SWARM_WIRE_H

#include "../shared/nslib.h"
#include "ISA.h"

using namespace nslib::serial;

namespace swarmc::Type {
    class Type;
}

namespace swarmc::Runtime {
    class VirtualMachine;
    class IFunctionCall;
    class ScopeFrame;
    class State;

    class Wire {
    public:
        static Factory<Type::Type, void*>* types() {
            if ( _type == nullptr ) {
                _type = buildTypes();
            }

            return _type;
        }

        static Factory<ISA::Reference, VirtualMachine*>* references() {
            if ( _reference == nullptr ) {
                _reference = buildReferences();
            }

            return _reference;
        }

        static Factory<IFunctionCall, VirtualMachine*>* calls() {
            if ( _call == nullptr ) {
                _call = buildCalls();
            }

            return _call;
        }

        static Factory<ScopeFrame, VirtualMachine*>* scopes() {
            if ( _scope == nullptr ) {
                _scope = buildScopes();
            }

            return _scope;
        }

        static Factory<State, VirtualMachine*>* states() {
            if ( _state == nullptr ) {
                _state = buildStates();
            }

            return _state;
        }
    protected:
        static inline Factory<Type::Type, void*>* _type = nullptr;
        static inline Factory<ISA::Reference, VirtualMachine*>* _reference = nullptr;
        static inline Factory<IFunctionCall, VirtualMachine*>* _call = nullptr;
        static inline Factory<ScopeFrame, VirtualMachine*>* _scope = nullptr;
        static inline Factory<State, VirtualMachine*>* _state = nullptr;

        static Factory<Type::Type, void*>* buildTypes();
        static Factory<ISA::Reference, VirtualMachine*>* buildReferences();
        static Factory<IFunctionCall, VirtualMachine*>* buildCalls();
        static Factory<ScopeFrame, VirtualMachine*>* buildScopes();
        static Factory<State, VirtualMachine*>* buildStates();
    };

}

#endif //SWARM_WIRE_H
