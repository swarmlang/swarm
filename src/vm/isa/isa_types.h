#ifndef SWARMVM_TYPES
#define SWARMVM_TYPES

#include "../ISA.h"

namespace swarmc::ISA {

    class TypeOf : public UnaryInstruction<Reference> {
    public:
        explicit TypeOf(Reference* value) :
            UnaryInstruction<Reference>(Tag::TYPEOF, useref(value)) {}
        ~TypeOf() override { freeref(_first); }
        [[nodiscard]] TypeOf* copy() const override {
            return new TypeOf(_first->copy());
        }
    };

    class IsCompatible : public BinaryReferenceInstruction {
    public:
        IsCompatible(Reference* lhs, Reference* value) :
            BinaryReferenceInstruction(Tag::COMPATIBLE, lhs, value) {}
        [[nodiscard]] IsCompatible* copy() const override {
            return new IsCompatible(_first->copy(), _second->copy());
        }
    };

}

#endif //SWARMVM_TYPES
