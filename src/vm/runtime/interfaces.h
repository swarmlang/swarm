#ifndef SWARMVM_INTERFACES_
#define SWARMVM_INTERFACES_

#include <vector>
#include "../../shared/IStringable.h"

namespace swarmc::ISA {
    class Reference;
    class LocationReference;
    enum class Affinity;
}

namespace swarmc::Runtime {

    class IStorageInterface;

    using Stores = std::vector<IStorageInterface*>;

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

        /** Returns true if this backend should be used to store the given variable. */
        virtual bool shouldContain(ISA::LocationReference*) = 0;

        /** Make the backend forget the value of the given variable, if it exists. */
        virtual void drop(ISA::LocationReference*) = 0;

        /** Forget all stored variables. */
        virtual void clear() = 0;
    };

}

#endif //SWARMVM_INTERFACES_
