#ifndef SWARMVM_CONTROL
#define SWARMVM_CONTROL

#include "../ISA.h"

namespace swarmc::ISA {

    class While : public BinaryInstruction<Reference, LocationReference> {
    public:
        While(Reference* cond, LocationReference* callback) :
            BinaryInstruction<Reference, LocationReference>(Tag::WHILE, cond, callback) {}
    };

    class With : public BinaryInstruction<Reference, LocationReference> {
    public:
        With(Reference* resource, LocationReference* callback) :
            BinaryInstruction<Reference, LocationReference>(Tag::WITH, resource, callback) {}
    };

}

#endif //SWARMVM_CONTROL
