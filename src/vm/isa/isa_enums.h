#ifndef SWARMVM_ENUMS
#define SWARMVM_ENUMS

#include "../ISA.h"

namespace swarmc::ISA {

    class EnumInit : public UnaryInstruction<Reference> {
    public:
        explicit EnumInit(Reference* type) :
            UnaryInstruction<Reference>(Tag::ENUMINIT, type) {}
        EnumInit* copy() const override {
            return new EnumInit(_first->copy());
        }
    };

    class EnumAppend : public BinaryInstruction<Reference, LocationReference> {
    public:
        EnumAppend(Reference* value, LocationReference* enumeration) :
            BinaryInstruction<Reference, LocationReference>(Tag::ENUMAPPEND, value, enumeration) {}
        EnumAppend* copy() const override {
            return new EnumAppend(_first->copy(), _second->copy());
        }
    };

    class EnumPrepend : public BinaryInstruction<Reference, LocationReference> {
    public:
        EnumPrepend(Reference* value, LocationReference* enumeration) :
            BinaryInstruction<Reference, LocationReference>(Tag::ENUMPREPEND, value, enumeration) {}
        EnumPrepend* copy() const override {
            return new EnumPrepend(_first->copy(), _second->copy());
        }
    };

    class EnumLength : public UnaryInstruction<LocationReference> {
    public:
        explicit EnumLength(LocationReference* enumeration) :
            UnaryInstruction<LocationReference>(Tag::ENUMLENGTH, enumeration) {}
        EnumLength* copy() const override {
            return new EnumLength(_first->copy());
        }
    };

    class EnumGet : public BinaryInstruction<LocationReference, Reference> {
    public:
        EnumGet(LocationReference* enumeration, Reference* key) :
            BinaryInstruction<LocationReference, Reference>(Tag::ENUMGET, enumeration, key) {}
        EnumGet* copy() const override {
            return new EnumGet(_first->copy(), _second->copy());
        }
    };

    class EnumSet : public TrinaryInstruction<LocationReference, Reference, Reference> {
    public:
        EnumSet(LocationReference* enumeration, Reference* key, Reference* value) :
            TrinaryInstruction<LocationReference, Reference, Reference>(Tag::ENUMSET, enumeration, key, value) {}
        EnumSet* copy() const override {
            return new EnumSet(_first->copy(), _second->copy(), _third->copy());
        }
    };

    class Enumerate : public TrinaryInstruction<Reference, LocationReference, LocationReference> {
    public:
        Enumerate(Reference* elemType, LocationReference* enumeration, LocationReference* fn) :
            TrinaryInstruction<Reference, LocationReference, LocationReference>(Tag::ENUMERATE, elemType, enumeration, fn) {}
        Enumerate* copy() const override {
            return new Enumerate(_first->copy(), _second->copy(), _third->copy());
        }
    };

}

#endif //SWARMVM_ENUMS
