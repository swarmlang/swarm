#ifndef SWARMVM_TYPES
#define SWARMVM_TYPES

#include "../ISA.h"

namespace swarmc::ISA {

    class TypeOf : public UnaryInstruction<Reference> {
    public:
        TypeOf(Reference* value) :
            UnaryInstruction<Reference>(Tag::TYPEOF, value) {}
    };

    class IsCompatible : public BinaryInstruction<Reference, Reference> {
    public:
        IsCompatible(Reference* lhs, Reference* value) :
            BinaryInstruction<Reference, Reference>(Tag::COMPATIBLE, lhs, value) {}
    };

}

#endif //SWARMVM_TYPES
