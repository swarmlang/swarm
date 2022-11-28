#ifndef SWARMVM_FUNCTIONS
#define SWARMVM_FUNCTIONS

#include "../ISA.h"

namespace swarmc::ISA {

    class BeginFunction : public BinaryInstruction<LocationReference, Reference> {
    public:
        BeginFunction(std::string name, Reference* returnType) :
            BinaryInstruction(
            Tag::BEGINFN,
            new LocationReference(Affinity::FUNCTION, std::move(name)),
            returnType) {}

        [[nodiscard]] virtual bool isPure() const { return _isPure; }

        virtual void markAsPure() { _isPure = true; }

        [[nodiscard]] BeginFunction* copy() const override {
            return new BeginFunction(_first->name(), _second->copy());
        }
    protected:
        bool _isPure = false;
    };

    class FunctionParam : public BinaryInstruction<Reference, LocationReference> {
    public:
        FunctionParam(Reference* type, LocationReference* loc) : BinaryInstruction<Reference, LocationReference>(Tag::FNPARAM, type, loc) {}
        [[nodiscard]] FunctionParam* copy() const override {
            return new FunctionParam(_first->copy(), _second->copy());
        }
    };

    class Return1 : public UnaryInstruction<Reference> {
    public:
        explicit Return1(Reference* value) : UnaryInstruction<Reference>(Tag::RETURN1, value) {}
        [[nodiscard]] Return1* copy() const override {
            return new Return1(_first->copy());
        }
    };

    class Return0 : public NullaryInstruction {
    public:
        Return0() : NullaryInstruction(Tag::RETURN0) {}
        [[nodiscard]] Return0* copy() const override {
            return new Return0();
        }
    };

    class Curry : public BinaryInstruction<Reference, Reference> {
    public:
        Curry(Reference* fn, Reference* param) :
            BinaryInstruction<Reference, Reference>(Tag::CURRY, fn, param) {}
        [[nodiscard]] Curry* copy() const override {
            return new Curry(_first->copy(), _second->copy());
        }
    };

    class Call0 : public UnaryInstruction<Reference> {
    public:
        explicit Call0(Reference* fn) : UnaryInstruction<Reference>(Tag::CALL0, fn) {}
        [[nodiscard]] Call0* copy() const override {
            return new Call0(_first->copy());
        }
    };

    class Call1 : public BinaryInstruction<Reference, Reference> {
    public:
        Call1(Reference* fn, Reference* param) :
                BinaryInstruction<Reference, Reference>(Tag::CALL1, fn, param) {}
        [[nodiscard]] Call1* copy() const override {
            return new Call1(_first->copy(), _second->copy());
        }
    };

    class CallIf0 : public BinaryInstruction<Reference, Reference> {
    public:
        CallIf0(Reference* cond, Reference* fn) :
                BinaryInstruction<Reference, Reference>(Tag::CALLIF0, cond, fn) {}
        [[nodiscard]] CallIf0* copy() const override {
            return new CallIf0(_first->copy(), _second->copy());
        }
    };

    class CallIf1 : public TrinaryInstruction<Reference, Reference, Reference> {
    public:
        CallIf1(Reference* cond, Reference* fn, Reference* param) :
                TrinaryInstruction<Reference, Reference, Reference>(Tag::CALLIF1, cond, fn, param) {}
        [[nodiscard]] CallIf1* copy() const override {
            return new CallIf1(_first->copy(), _second->copy(), _third->copy());
        }
    };

    class CallElse0 : public BinaryInstruction<Reference, Reference> {
    public:
        CallElse0(Reference* cond, Reference* fn) :
                BinaryInstruction<Reference, Reference>(Tag::CALLELSE0, cond, fn) {}
        [[nodiscard]] CallElse0* copy() const override {
            return new CallElse0(_first->copy(), _second->copy());
        }
    };

    class CallElse1 : public TrinaryInstruction<Reference, Reference, Reference> {
    public:
        CallElse1(Reference* cond, Reference* fn, Reference* param) :
                TrinaryInstruction<Reference, Reference, Reference>(Tag::CALLELSE1, cond, fn, param) {}
        [[nodiscard]] CallElse1* copy() const override {
            return new CallElse1(_first->copy(), _second->copy(), _third->copy());
        }
    };

    class PushCall0 : public UnaryInstruction<Reference> {
    public:
        explicit PushCall0(Reference* fn) : UnaryInstruction<Reference>(Tag::PUSHCALL0, fn) {}
        [[nodiscard]] PushCall0* copy() const override {
            return new PushCall0(_first->copy());
        }
    };

    class PushCall1 : public BinaryInstruction<Reference, Reference> {
    public:
        PushCall1(Reference* fn, Reference* param) :
                BinaryInstruction<Reference, Reference>(Tag::PUSHCALL1, fn, param) {}
        [[nodiscard]] PushCall1* copy() const override {
            return new PushCall1(_first->copy(), _second->copy());
        }
    };

    class PushCallIf0 : public BinaryInstruction<Reference, Reference> {
    public:
        PushCallIf0(Reference* cond, Reference* fn) :
                BinaryInstruction<Reference, Reference>(Tag::PUSHCALLIF0, cond, fn) {}
        [[nodiscard]] PushCallIf0* copy() const override {
            return new PushCallIf0(_first->copy(), _second->copy());
        }
    };

    class PushCallIf1 : public TrinaryInstruction<Reference, Reference, Reference> {
    public:
        PushCallIf1(Reference* cond, Reference* fn, Reference* param) :
                TrinaryInstruction<Reference, Reference, Reference>(Tag::PUSHCALLIF1, cond, fn, param) {}
        [[nodiscard]] PushCallIf1* copy() const override {
            return new PushCallIf1(_first->copy(), _second->copy(), _third->copy());
        }
    };

    class PushCallElse0 : public BinaryInstruction<Reference, Reference> {
    public:
        PushCallElse0(Reference* cond, Reference* fn) :
                BinaryInstruction<Reference, Reference>(Tag::PUSHCALLELSE0, cond, fn) {}
        [[nodiscard]] PushCallElse0* copy() const override {
            return new PushCallElse0(_first->copy(), _second->copy());
        }
    };

    class PushCallElse1 : public TrinaryInstruction<Reference, Reference, Reference> {
    public:
        PushCallElse1(Reference* cond, Reference* fn, Reference* param) :
                TrinaryInstruction<Reference, Reference, Reference>(Tag::PUSHCALLELSE1, cond, fn, param) {}
        [[nodiscard]] PushCallElse1* copy() const override {
            return new PushCallElse1(_first->copy(), _second->copy(), _third->copy());
        }
    };

    class Drain : public NullaryInstruction {
    public:
        Drain() : NullaryInstruction(Tag::DRAIN) {}
        [[nodiscard]] Drain* copy() const override {
            return new Drain();
        }
    };

    class Exit : public NullaryInstruction {
    public:
        Exit() : NullaryInstruction(Tag::EXIT) {}
        [[nodiscard]] Exit* copy() const override {
            return new Exit();
        }
    };

}

#endif //SWARMVM_FUNCTIONS
