#ifndef SWARMVM_ISA_HELPERS
#define SWARMVM_ISA_HELPERS

#include "ISA.h"
#include "isa/isa_storage.h"

namespace swarmc::ISA {

    LocationReference* local(std::string name) {
        return new LocationReference(Affinity::LOCAL, name);
    }

    LocationReference* shared(std::string name) {
        return new LocationReference(Affinity::SHARED, name);
    }

    LocationReference* fn(std::string name) {
        return new LocationReference(Affinity::FUNCTION, name);
    }

    NumberReference* number(double value) {
        return new NumberReference(value);
    }

    IsEqual* isEqual(Reference* lhs, Reference* rhs) {
        return new IsEqual(lhs, rhs);
    }

}

#endif //SWARMVM_ISA_HELPERS
