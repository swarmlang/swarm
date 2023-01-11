#ifndef SWARMVM_CONTROL
#define SWARMVM_CONTROL

#include "../ISA.h"

namespace swarmc::ISA {

    class While : public BinaryInstruction<Reference, LocationReference> {
    public:
        While(Reference* cond, LocationReference* callback) :
            BinaryInstruction<Reference, LocationReference>(Tag::WHILE, useref(cond), useref(callback)) {}
        ~While() override {
            freeref(_first);
            freeref(_second);
        }
        [[nodiscard]] While* copy() const override {
            return new While(_first->copy(), _second->copy());
        }
    };

    class With : public BinaryInstruction<Reference, LocationReference> {
    public:
        With(Reference* resource, LocationReference* callback) :
            BinaryInstruction<Reference, LocationReference>(Tag::WITH, useref(resource), useref(callback)) {}
        ~With() override {
            freeref(_first);
            freeref(_second);
        }
        [[nodiscard]] With* copy() const override {
            return new With(_first->copy(), _second->copy());
        }
    };

}

#endif //SWARMVM_CONTROL
