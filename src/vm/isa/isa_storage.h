#ifndef SWARMVM_STORAGE
#define SWARMVM_STORAGE

#include "../ISA.h"

namespace swarmc::ISA {

    class Typify : public BinaryInstruction<LocationReference, Reference> {
    public:
        Typify(LocationReference* loc, Reference* type) :
            BinaryInstruction<LocationReference, Reference>(Tag::TYPIFY, loc, type) {}
        virtual Typify* copy() const override {
            return new Typify(_first->copy(), _second->copy());
        }
    };

    class AssignValue : public BinaryInstruction<LocationReference, Reference> {
    public:
        AssignValue(LocationReference* loc, Reference* value) :
            BinaryInstruction<LocationReference, Reference>(Tag::ASSIGNVALUE, loc, value) {}
        virtual AssignValue* copy() const override {
            return new AssignValue(_first->copy(), _second->copy());
        }
    };

    class AssignEval : public BinaryInstruction<LocationReference, Instruction> {
    public:
        AssignEval(LocationReference* loc, Instruction* exe) :
            BinaryInstruction<LocationReference, Instruction>(Tag::ASSIGNEVAL, loc, exe) {}
        virtual AssignEval* copy() const override {
            return new AssignEval(_first->copy(), _second->copy());
        }
    };

    class Lock : public UnaryInstruction<LocationReference> {
    public:
        Lock(LocationReference* loc) :
            UnaryInstruction<LocationReference>(Tag::LOCK, loc) {}
        virtual Lock* copy() const override {
            return new Lock(_first->copy());
        }
    };

    class Unlock : public UnaryInstruction<LocationReference> {
    public:
        Unlock(LocationReference* loc) :
            UnaryInstruction<LocationReference>(Tag::UNLOCK, loc) {}
        virtual Unlock* copy() const override {
            return new Unlock(_first->copy());
        }
    };

    class IsEqual : public BinaryInstruction<Reference, Reference> {
    public:
        IsEqual(Reference* lhs, Reference* rhs) :
            BinaryInstruction<Reference, Reference>(Tag::EQUAL, lhs, rhs) {}
        virtual IsEqual* copy() const override {
            return new IsEqual(_first->copy(), _second->copy());
        }
    };

    class ScopeOf : public UnaryInstruction<LocationReference> {
    public:
        ScopeOf(LocationReference* loc) :
            UnaryInstruction<LocationReference>(Tag::SCOPEOF, loc) {}
        virtual ScopeOf* copy() const override {
            return new ScopeOf(_first->copy());
        }
    };

}

#endif //SWARMVM_STORAGE
