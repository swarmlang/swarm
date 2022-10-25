#include "../errors/SwarmError.h"
#include "VirtualMachine.h"

namespace swarmc::Runtime {
    using namespace swarmc::ISA;

    IStorageInterface* VirtualMachine::getStore(LocationReference* loc) {
        Stores::size_type idx = _stores.size() - 1;
        for ( auto it = _stores.rbegin(); it != _stores.rend(); ++it, --idx ) {
            auto store = *it;
            if ( store->shouldContain(loc) ) {
                return _stores[idx];
            }
        }

        throw new Errors::SwarmError("Unable to find storage backend for location: " + loc->toString());
    }

    Reference* VirtualMachine::load(LocationReference* loc) {
        auto store = getStore(loc);
        if ( !store->has(loc) ) {
            throw new Errors::SwarmError("Attempted to load undefined location (" + loc->toString() + ") from store (" + store->toString() + ")");
        }

        return store->load(loc);
    }

    Reference* VirtualMachine::resolve(Reference* ref) {
        if ( ref->tag() == ReferenceTag::LOCATION ) {
            return load((LocationReference*) ref);
        }

        return ref;
    }

}
