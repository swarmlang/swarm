#ifndef SWARMVM_FUNCTIONS
#define SWARMVM_FUNCTIONS

#include "../ISA.h"

namespace swarmc::ISA {

    class BeginFunction : public BinaryInstruction<LocationReference, Reference> {
    public:
        BeginFunction(std::string name, Reference* returnType) :
            BinaryInstruction(
            Tag::BEGINFN,
            new LocationReference(Affinity::FUNCTION, name),
            returnType) {}
    };

    class FunctionParam : public UnaryInstruction<Reference> {
    public:
        FunctionParam(Reference* type) : UnaryInstruction<Reference>(Tag::FNPARAM, type) {}
    };

    class Return1 : public UnaryInstruction<Reference> {
    public:
        Return1(Reference* value) : UnaryInstruction<Reference>(Tag::RETURN1, value) {}
    };

    class Return0 : public NullaryInstruction {
    public:
        Return0() : NullaryInstruction(Tag::RETURN0) {}
    };

    class Curry : public BinaryInstruction<Reference, Reference> {
    public:
        Curry(Reference* fn, Reference* param) :
            BinaryInstruction<Reference, Reference>(Tag::CURRY, fn, param) {}
    };

    class Call0 : public UnaryInstruction<Reference> {
    public:
        Call0(Reference* fn) : UnaryInstruction<Reference>(Tag::CALL0, fn) {}
    };

    class Call1 : public BinaryInstruction<Reference, Reference> {
    public:
        Call1(Reference* fn, Reference* param) :
                BinaryInstruction<Reference, Reference>(Tag::CALL1, fn, param) {}
    };

    class CallIf0 : public BinaryInstruction<Reference, Reference> {
    public:
        CallIf0(Reference* cond, Reference* fn) :
                BinaryInstruction<Reference, Reference>(Tag::CALLIF0, cond, fn) {}
    };

    class CallIf1 : public TrinaryInstruction<Reference, Reference, Reference> {
    public:
        CallIf1(Reference* cond, Reference* fn, Reference* param) :
                TrinaryInstruction<Reference, Reference, Reference>(Tag::CALLIF1, cond, fn, param) {}
    };

    class CallElse0 : public BinaryInstruction<Reference, Reference> {
    public:
        CallElse0(Reference* cond, Reference* fn) :
                BinaryInstruction<Reference, Reference>(Tag::CALLELSE0, cond, fn) {}
    };

    class CallElse1 : public TrinaryInstruction<Reference, Reference, Reference> {
    public:
        CallElse1(Reference* cond, Reference* fn, Reference* param) :
                TrinaryInstruction<Reference, Reference, Reference>(Tag::CALLELSE1, cond, fn, param) {}
    };

    class PushCall0 : public UnaryInstruction<Reference> {
    public:
        PushCall0(Reference* fn) : UnaryInstruction<Reference>(Tag::PUSHCALL0, fn) {}
    };

    class PushCall1 : public BinaryInstruction<Reference, Reference> {
    public:
        PushCall1(Reference* fn, Reference* param) :
                BinaryInstruction<Reference, Reference>(Tag::PUSHCALL1, fn, param) {}
    };

    class PushCallIf0 : public BinaryInstruction<Reference, Reference> {
    public:
        PushCallIf0(Reference* cond, Reference* fn) :
                BinaryInstruction<Reference, Reference>(Tag::PUSHCALLIF0, cond, fn) {}
    };

    class PushCallIf1 : public TrinaryInstruction<Reference, Reference, Reference> {
    public:
        PushCallIf1(Reference* cond, Reference* fn, Reference* param) :
                TrinaryInstruction<Reference, Reference, Reference>(Tag::PUSHCALLIF1, cond, fn, param) {}
    };

    class PushCallElse0 : public BinaryInstruction<Reference, Reference> {
    public:
        PushCallElse0(Reference* cond, Reference* fn) :
                BinaryInstruction<Reference, Reference>(Tag::PUSHCALLELSE0, cond, fn) {}
    };

    class PushCallElse1 : public TrinaryInstruction<Reference, Reference, Reference> {
    public:
        PushCallElse1(Reference* cond, Reference* fn, Reference* param) :
                TrinaryInstruction<Reference, Reference, Reference>(Tag::PUSHCALLELSE1, cond, fn, param) {}
    };

}

#endif //SWARMVM_FUNCTIONS
