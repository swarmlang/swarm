#ifndef SWARM_ISA_OBJECTS_H
#define SWARM_ISA_OBJECTS_H

#include "../ISA.h"

namespace swarmc::ISA {

    class OTypeInit : public NullaryInstruction {
    public:
        OTypeInit() : NullaryInstruction(Tag::OTYPEINIT) {}
        [[nodiscard]] OTypeInit* copy() const override {
            return new OTypeInit;
        }
    };

    class OTypeProp : public TrinaryInstruction<Reference, LocationReference, Reference> {
    public:
        OTypeProp(Reference* otype, LocationReference* propName, Reference* propType)
            : TrinaryInstruction<Reference, LocationReference, Reference>(Tag::OTYPEPROP, useref(otype), useref(propName), useref(propType)) {}
        ~OTypeProp() override {
            freeref(_first);
            freeref(_second);
            freeref(_third);
        }
        [[nodiscard]] OTypeProp* copy() const override {
            return new OTypeProp(_first, _second, _third);
        }
    };

    class OTypeDel : public BinaryInstruction<Reference, LocationReference> {
    public:
        OTypeDel(Reference* otype, LocationReference* propName)
            : BinaryInstruction<Reference, LocationReference>(Tag::OTYPEDEL, useref(otype), useref(propName)) {}
        ~OTypeDel() override {
            freeref(_first);
            freeref(_second);
        }
        [[nodiscard]] OTypeDel* copy() const override {
            return new OTypeDel(_first, _second);
        }
    };

    class OTypeGet : public BinaryInstruction<Reference, LocationReference> {
    public:
        OTypeGet(Reference* otype, LocationReference* propName)
            : BinaryInstruction<Reference, LocationReference>(Tag::OTYPEGET, useref(otype), useref(propName)) {}
        ~OTypeGet() override {
            freeref(_first);
            freeref(_second);
        }
        [[nodiscard]] OTypeGet* copy() const override {
            return new OTypeGet(_first, _second);
        }
    };

    class OTypeFinalize : public UnaryInstruction<Reference> {
    public:
        explicit OTypeFinalize(Reference* otype) : UnaryInstruction<Reference>(Tag::OTYPEFINALIZE, useref(otype)) {}
        ~OTypeFinalize() override { freeref(_first); }
        [[nodiscard]] OTypeFinalize* copy() const override {
            return new OTypeFinalize(_first);
        }
    };

    class OTypeSubset : public UnaryInstruction<Reference> {
    public:
        explicit OTypeSubset(Reference* otype) : UnaryInstruction<Reference>(Tag::OTYPESUBSET, useref(otype)) {}
        ~OTypeSubset() override { freeref(_first); }
        [[nodiscard]] OTypeSubset* copy() const override {
            return new OTypeSubset(_first);
        }
    };

}

#endif //SWARM_ISA_OBJECTS_H
