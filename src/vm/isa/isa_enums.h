#ifndef SWARMVM_ENUMS
#define SWARMVM_ENUMS

#include "../ISA.h"

namespace swarmc::ISA {

    class EnumInit : public UnaryInstruction<Reference> {
    public:
        explicit EnumInit(Reference* type) :
            UnaryInstruction<Reference>(Tag::ENUMINIT, useref(type)) {}
        ~EnumInit() override { freeref(_first); }
        [[nodiscard]] EnumInit* copy() const override {
            return new EnumInit(_first->copy());
        }
    };

    class EnumAppend : public BinaryInstruction<Reference, LocationReference> {
    public:
        EnumAppend(Reference* value, LocationReference* enumeration) :
            BinaryInstruction<Reference, LocationReference>(Tag::ENUMAPPEND, useref(value), useref(enumeration)) {}
        ~EnumAppend() override {
            freeref(_first);
            freeref(_second);
        }
        [[nodiscard]] EnumAppend* copy() const override {
            return new EnumAppend(_first->copy(), _second->copy());
        }
    };

    class EnumPrepend : public BinaryInstruction<Reference, LocationReference> {
    public:
        EnumPrepend(Reference* value, LocationReference* enumeration) :
            BinaryInstruction<Reference, LocationReference>(Tag::ENUMPREPEND, useref(value), useref(enumeration)) {}
        ~EnumPrepend() override {
            freeref(_first);
            freeref(_second);
        }
        [[nodiscard]] EnumPrepend* copy() const override {
            return new EnumPrepend(_first->copy(), _second->copy());
        }
    };

    class EnumLength : public UnaryInstruction<LocationReference> {
    public:
        explicit EnumLength(LocationReference* enumeration) :
            UnaryInstruction<LocationReference>(Tag::ENUMLENGTH, useref(enumeration)) {}
        ~EnumLength() override { freeref(_first); }
        [[nodiscard]] EnumLength* copy() const override {
            return new EnumLength(_first->copy());
        }
    };

    class EnumGet : public BinaryInstruction<LocationReference, Reference> {
    public:
        EnumGet(LocationReference* enumeration, Reference* key) :
            BinaryInstruction<LocationReference, Reference>(Tag::ENUMGET, useref(enumeration), useref(key)) {}
        ~EnumGet() override {
            freeref(_first);
            freeref(_second);
        }
        [[nodiscard]] EnumGet* copy() const override {
            return new EnumGet(_first->copy(), _second->copy());
        }
    };

    class EnumSet : public TrinaryInstruction<LocationReference, Reference, Reference> {
    public:
        EnumSet(LocationReference* enumeration, Reference* key, Reference* value) :
            TrinaryInstruction<LocationReference, Reference, Reference>(Tag::ENUMSET, useref(enumeration), useref(key), useref(value)) {}
        ~EnumSet() override {
            freeref(_first);
            freeref(_second);
            freeref(_third);
        }
        [[nodiscard]] EnumSet* copy() const override {
            return new EnumSet(_first->copy(), _second->copy(), _third->copy());
        }
    };

    class EnumConcat : public BinaryInstruction<LocationReference, LocationReference> {
    public:
        EnumConcat(LocationReference* enum1, LocationReference* enum2) :
            BinaryInstruction<LocationReference, LocationReference>(Tag::ENUMCONCAT, useref(enum1), useref(enum2)) {}
        ~EnumConcat() override {
            freeref(_first);
            freeref(_second);
        }
        [[nodiscard]] EnumConcat* copy() const override {
            return new EnumConcat(_first->copy(), _second->copy());
        }
    };

    class Enumerate : public TrinaryInstruction<Reference, LocationReference, LocationReference> {
    public:
        Enumerate(Reference* elemType, LocationReference* enumeration, LocationReference* fn) :
            TrinaryInstruction<Reference, LocationReference, LocationReference>(Tag::ENUMERATE, useref(elemType), useref(enumeration), useref(fn)) {}
        ~Enumerate() override {
            freeref(_first);
            freeref(_second);
            freeref(_third);
        }
        [[nodiscard]] Enumerate* copy() const override {
            return new Enumerate(_first->copy(), _second->copy(), _third->copy());
        }
    };

}

#endif //SWARMVM_ENUMS
