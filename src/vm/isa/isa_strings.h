#ifndef SWARMVM_STRINGS
#define SWARMVM_STRINGS

#include "../ISA.h"

namespace swarmc::ISA {

    class StringConcat : public BinaryInstruction<Reference, Reference> {
    public:
        StringConcat(Reference* lhs, Reference* rhs) :
            BinaryInstruction<Reference, Reference>(Tag::STRCONCAT, lhs, rhs) {}
    };

    class StringLength : public UnaryInstruction<Reference> {
    public:
        StringLength(Reference* string) :
            UnaryInstruction<Reference>(Tag::STRLENGTH, string) {}
    };

    class StringSliceFrom : public BinaryInstruction<Reference, Reference> {
    public:
        StringSliceFrom(Reference* string, Reference* startAtIndex) :
            BinaryInstruction<Reference, Reference>(Tag::STRSLICEFROM, string, startAtIndex) {}
    };

    class StringSliceFromTo : public TrinaryInstruction<Reference, Reference, Reference> {
    public:
        StringSliceFromTo(Reference* string, Reference* startAtIndex, Reference* endAtIndex) :
            TrinaryInstruction<Reference, Reference, Reference>(Tag::STRSLICEFROMTO, string, startAtIndex, endAtIndex) {}
    };

}

#endif //SWARMVM_STRINGS
