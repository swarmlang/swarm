#ifndef SWARMVM_INTERFACES
#define SWARMVM_INTERFACES

#include <vector>
#include "../../shared/IStringable.h"

namespace swarmc::ISA {
    class Reference;
    class LocationReference;
    enum class Affinity;
}

namespace swarmc::Type {
    class Type;
}

namespace swarmc::Runtime {

    class IStorageLock;
    class IStorageInterface;

    using Stores = std::vector<IStorageInterface*>;
    using Locks = std::vector<IStorageLock*>;

    /** Represents a lock acquired by some control. */
    class IStorageLock : public IStringable {
    public:
        virtual ~IStorageLock() = default;

        /** Get the location this lock covers. */
        virtual ISA::LocationReference* location() const = 0;

        /** Release this lock. */
        virtual void release() = 0;
    };

    /**
     * Interface for VM variable storage backends.
     */
    class IStorageInterface : public IStringable {
    public:
        virtual ~IStorageInterface() = default;

        /** Load the actual value of a variable. */
        virtual ISA::Reference* load(ISA::LocationReference*) = 0;

        /** Store a value into a variable. */
        virtual void store(ISA::LocationReference*, ISA::Reference*) = 0;

        /** Returns true if this backend has a value for the given variable. */
        virtual bool has(ISA::LocationReference*) = 0;

        /** Returns true if this backend should be used to lock/store the given variable. */
        virtual bool manages(ISA::LocationReference*) = 0;

        /** Make the backend forget the value of the given variable, if it exists. */
        virtual void drop(ISA::LocationReference*) = 0;

        /** Get the type of the given location. */
        virtual const Type::Type* typeOf(ISA::LocationReference*) = 0;

        /** Constrain the given location to a specific type. */
        virtual void typify(ISA::LocationReference*, const Type::Type*) = 0;

        /**
         * Attempt to acquire a lock for a particular storage location.
         * If the lock is acquired, an IStorageLock is returned. It not,
         * then nullptr is returned and the caller should retry.
         */
        virtual IStorageLock* acquire(ISA::LocationReference*) = 0;

        /** Forget all stored variables. */
        virtual void clear() = 0;
    };

}

#endif //SWARMVM_INTERFACES
