#include "../../errors/SwarmError.h"
#include "../VirtualMachine.h"
#include "BinaryReferenceWalk.h"

namespace swarmc::ISA {

    FunctionReference* BinaryReferenceWalk::walkFunctionReference(std::size_t backend, const std::string& name, std::vector<Reference*> params) {
        if ( _vm == nullptr ) {
            throw Errors::SwarmError("Cannot deserialize FunctionReference without VirtualMachine environment.");
        }

        auto ref = _vm->loadFunction((Runtime::FunctionBackend) backend, name);
        auto fn = ref->fn();

        for ( auto p : params ) {
            fn = fn->curry(p);
        }

        delete ref;
        return new FunctionReference(fn);
    }

    StreamReference* BinaryReferenceWalk::walkStreamReference(const std::string& id, const Type::Type* type) {
        if ( _vm == nullptr ) {
            throw Errors::SwarmError("Cannot deserialize StreamReference without VirtualMachine environment.");
        }

        auto stream = _vm->getStream(id, type);
        return new StreamReference(stream);
    }

}
