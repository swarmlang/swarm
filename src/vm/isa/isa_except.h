#ifndef SWARMVM_EXCEPT
#define SWARMVM_EXCEPT

#include "../ISA.h"

namespace swarmc::ISA {

    class PushExceptionHandler1 : public UnaryInstruction<LocationReference> {
    public:
        explicit PushExceptionHandler1(LocationReference* handlerFn) :
            UnaryInstruction<LocationReference>(Tag::PUSHEXHANDLER1, handlerFn) {}
        [[nodiscard]] PushExceptionHandler1* copy() const override {
            return new PushExceptionHandler1(_first->copy());
        }
    };

    class PushExceptionHandler2 : public BinaryInstruction<LocationReference, LocationReference> {
    public:
        PushExceptionHandler2(LocationReference* handlerFn, LocationReference* discriminatorFn) :
            BinaryInstruction<LocationReference, LocationReference>(Tag::PUSHEXHANDLER2, handlerFn, discriminatorFn) {}
        [[nodiscard]] PushExceptionHandler2* copy() const override {
            return new PushExceptionHandler2(_first->copy(), _second->copy());
        }
    };

    class PopExceptionHandler : public UnaryInstruction<Reference> {
    public:
        explicit PopExceptionHandler(Reference* handlerId) :
            UnaryInstruction<Reference>(Tag::POPEXHANDLER, handlerId) {}
        [[nodiscard]] PopExceptionHandler* copy() const override {
            return new PopExceptionHandler(_first->copy());
        }
    };

    class Raise : public UnaryInstruction<Reference> {
    public:
        explicit Raise(Reference* exceptionId) :
            UnaryInstruction<Reference>(Tag::RAISE, exceptionId) {}
        [[nodiscard]] Raise* copy() const override {
            return new Raise(_first->copy());
        }
    };

    class Resume : public UnaryInstruction<LocationReference> {
    public:
        explicit Resume(LocationReference* fn) :
            UnaryInstruction<LocationReference>(Tag::RESUME, fn) {}
        [[nodiscard]] Resume* copy() const override {
            return new Resume(_first->copy());
        }
    };

}

#endif //SWARMVM_EXCEPT
