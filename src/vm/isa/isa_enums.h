#ifndef SWARMVM_ENUMS
#define SWARMVM_ENUMS

#include "../ISA.h"

namespace swarmc::ISA {

    class EnumInit : public UnaryInstruction<Reference> {
    public:
        EnumInit(Reference* type) :
            UnaryInstruction<Reference>(Tag::ENUMINIT, type) {}
    };

    class EnumAppend : public BinaryInstruction<Reference, LocationReference> {
    public:
        EnumAppend(Reference* value, LocationReference* enumeration) :
            BinaryInstruction<Reference, LocationReference>(Tag::ENUMAPPEND, value, enumeration) {}
    };

    class EnumPrepend : public BinaryInstruction<Reference, LocationReference> {
    public:
        EnumPrepend(Reference* value, LocationReference* enumeration) :
            BinaryInstruction<Reference, LocationReference>(Tag::ENUMPREPEND, value, enumeration) {}
    };

    class EnumLength : public UnaryInstruction<LocationReference> {
    public:
        EnumLength(LocationReference* enumeration) :
            UnaryInstruction<LocationReference>(Tag::ENUMLENGTH, enumeration) {}
    };

    class EnumGet : public BinaryInstruction<LocationReference, Reference> {
    public:
        EnumGet(LocationReference* enumeration, Reference* key) :
            BinaryInstruction<LocationReference, Reference>(Tag::ENUMGET, enumeration, key) {}
    };

    class EnumSet : public TrinaryInstruction<LocationReference, Reference, Reference> {
    public:
        EnumSet(LocationReference* enumeration, Reference* key, Reference* value) :
            TrinaryInstruction<LocationReference, Reference, Reference>(Tag::ENUMSET, enumeration, key, value) {}
    };

    class Enumerate : public TrinaryInstruction<Reference, LocationReference, LocationReference> {
    public:
        Enumerate(Reference* elemType, LocationReference* enumeration, LocationReference* fn) :
            TrinaryInstruction<Reference, LocationReference, LocationReference>(Tag::ENUMERATE, elemType, enumeration, fn) {}
    };

}

#endif //SWARMVM_ENUMS
