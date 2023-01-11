#ifndef SWARMVM_STRINGS
#define SWARMVM_STRINGS

#include "../ISA.h"

namespace swarmc::ISA {

    class StringConcat : public BinaryReferenceInstruction {
    public:
        StringConcat(Reference* lhs, Reference* rhs) :
            BinaryReferenceInstruction(Tag::STRCONCAT, lhs, rhs) {}
        [[nodiscard]] StringConcat* copy() const override {
            return new StringConcat(_first->copy(), _second->copy());
        }
    };

    class StringLength : public UnaryInstruction<Reference> {
    public:
        explicit StringLength(Reference* string) :
            UnaryInstruction<Reference>(Tag::STRLENGTH, useref(string)) {}
        ~StringLength() override { freeref(_first); }
        [[nodiscard]] StringLength* copy() const override {
            return new StringLength(_first->copy());
        }
    };

    class StringSliceFrom : public BinaryReferenceInstruction {
    public:
        StringSliceFrom(Reference* string, Reference* startAtIndex) :
            BinaryReferenceInstruction(Tag::STRSLICEFROM, string, startAtIndex) {}
        [[nodiscard]] StringSliceFrom* copy() const override {
            return new StringSliceFrom(_first->copy(), _second->copy());
        }
    };

    class StringSliceFromTo : public TrinaryReferenceInstruction {
    public:
        StringSliceFromTo(Reference* string, Reference* startAtIndex, Reference* endAtIndex) :
            TrinaryReferenceInstruction(Tag::STRSLICEFROMTO, string, startAtIndex, endAtIndex) {}
        [[nodiscard]] StringSliceFromTo* copy() const override {
            return new StringSliceFromTo(_first->copy(), _second->copy(), _third->copy());
        }
    };

}

#endif //SWARMVM_STRINGS
