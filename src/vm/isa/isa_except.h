#ifndef SWARMVM_EXCEPT
#define SWARMVM_EXCEPT

#include "../ISA.h"

namespace swarmc::ISA {

    class PushExceptionHandler1 : public UnaryInstruction<LocationReference> {
    public:
        explicit PushExceptionHandler1(LocationReference* handlerFn) :
            UnaryInstruction<LocationReference>(Tag::PUSHEXHANDLER1, useref(handlerFn)) {}
        ~PushExceptionHandler1() override { freeref(_first); }
        [[nodiscard]] PushExceptionHandler1* copy() const override {
            return new PushExceptionHandler1(_first->copy());
        }
    };

    class PushExceptionHandler2 : public BinaryInstruction<LocationReference, LocationReference> {
    public:
        PushExceptionHandler2(LocationReference* handlerFn, LocationReference* discriminator) :
            BinaryInstruction<LocationReference, LocationReference>(Tag::PUSHEXHANDLER2, useref(handlerFn), useref(discriminator)) {}
        ~PushExceptionHandler2() override {
            freeref(_first);
            freeref(_second);
        }
        [[nodiscard]] PushExceptionHandler2* copy() const override {
            return new PushExceptionHandler2(_first->copy(), _second->copy());
        }
    };

    class PopExceptionHandler : public UnaryInstruction<Reference> {
    public:
        explicit PopExceptionHandler(Reference* handlerId) :
            UnaryInstruction<Reference>(Tag::POPEXHANDLER, useref(handlerId)) {}
        ~PopExceptionHandler() override { freeref(_first); }
        [[nodiscard]] PopExceptionHandler* copy() const override {
            return new PopExceptionHandler(_first->copy());
        }
    };

    class Raise : public UnaryInstruction<Reference> {
    public:
        explicit Raise(Reference* exceptionId) :
            UnaryInstruction<Reference>(Tag::RAISE, useref(exceptionId)) {}
        ~Raise() override { freeref(_first); }
        [[nodiscard]] Raise* copy() const override {
            return new Raise(_first->copy());
        }
    };

    class Resume : public UnaryInstruction<LocationReference> {
    public:
        explicit Resume(LocationReference* fn) :
            UnaryInstruction<LocationReference>(Tag::RESUME, useref(fn)) {}
        ~Resume() override { freeref(_first); }
        [[nodiscard]] Resume* copy() const override {
            return new Resume(_first->copy());
        }
    };

}

#endif //SWARMVM_EXCEPT
