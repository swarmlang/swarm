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



    class ObjInit : public UnaryInstruction<Reference> {
    public:
        explicit ObjInit(Reference* otype) : UnaryInstruction<Reference>(Tag::OBJINIT, useref(otype)) {}
        ~ObjInit() override { freeref(_first); }
        [[nodiscard]] ObjInit* copy() const override {
            return new ObjInit(_first);
        }
    };

    class ObjSet : public TrinaryInstruction<Reference, LocationReference, Reference> {
    public:
        ObjSet(Reference* obj, LocationReference* prop, Reference* value) :
            TrinaryInstruction<Reference, LocationReference, Reference>(Tag::OBJSET, useref(obj), useref(prop), useref(value)) {}
        ~ObjSet() override {
            freeref(_first);
            freeref(_second);
            freeref(_third);
        }
        [[nodiscard]] ObjSet* copy() const override {
            return new ObjSet(_first, _second, _third);
        }
    };

    class ObjGet : public BinaryInstruction<Reference, LocationReference> {
    public:
        ObjGet(Reference* obj, LocationReference* prop) :
            BinaryInstruction<Reference, LocationReference>(Tag::OBJGET, useref(obj), useref(prop)) {}
        ~ObjGet() override {
            freeref(_first);
            freeref(_second);
        }
        [[nodiscard]] ObjGet* copy() const override {
            return new ObjGet(_first, _second);
        }
    };

    class ObjInstance : public UnaryInstruction<Reference> {
    public:
        explicit ObjInstance(Reference* obj) : UnaryInstruction<Reference>(Tag::OBJINSTANCE, useref(obj)) {}
        ~ObjInstance() override { freeref(_first); }
        [[nodiscard]] ObjInstance* copy() const override {
            return new ObjInstance(_first);
        }
    };

    class ObjCurry : public BinaryInstruction<Reference, LocationReference> {
    public:
        ObjCurry(Reference* obj, LocationReference* prop) :
            BinaryInstruction<Reference, LocationReference>(Tag::OBJCURRY, useref(obj), useref(prop)) {}
        ~ObjCurry() override {
            freeref(_first);
            freeref(_second);
        }
        [[nodiscard]] ObjCurry* copy() const override {
            return new ObjCurry(_first, _second);
        }
    };
}

#endif //SWARM_ISA_OBJECTS_H
