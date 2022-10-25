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
}
