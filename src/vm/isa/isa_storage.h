#ifndef SWARMVM_STORAGE
#define SWARMVM_STORAGE

#include "../ISA.h"

namespace swarmc::ISA {

    class Typify : public BinaryInstruction<LocationReference, Reference> {
    public:
        Typify(LocationReference* loc, Reference* type) :
            BinaryInstruction<LocationReference, Reference>(Tag::TYPIFY, useref(loc), useref(type)) {}
        ~Typify() override {
            freeref(_first);
            freeref(_second);
        }
        [[nodiscard]] Typify* copy() const override {
            return new Typify(_first->copy(), _second->copy());
        }
    };

    class AssignValue : public BinaryInstruction<LocationReference, Reference> {
    public:
        AssignValue(LocationReference* loc, Reference* value) :
            BinaryInstruction<LocationReference, Reference>(Tag::ASSIGNVALUE, useref(loc), useref(value)) {}
        ~AssignValue() override {
            freeref(_first);
            freeref(_second);
        }
        [[nodiscard]] AssignValue* copy() const override {
            return new AssignValue(_first->copy(), _second->copy());
        }
    };

    class AssignEval : public BinaryInstruction<LocationReference, Instruction> {
    public:
        AssignEval(LocationReference* loc, Instruction* exe) :
            BinaryInstruction<LocationReference, Instruction>(Tag::ASSIGNEVAL, useref(loc), useref(exe)) {}
        ~AssignEval() override {
            freeref(_first);
            freeref(_second);
        }
        [[nodiscard]] AssignEval* copy() const override {
            return new AssignEval(_first->copy(), _second->copy());
        }
    };

    class Lock : public UnaryInstruction<LocationReference> {
    public:
        explicit Lock(LocationReference* loc) :
            UnaryInstruction<LocationReference>(Tag::LOCK, useref(loc)) {}
        ~Lock() override { freeref(_first); }
        [[nodiscard]] Lock* copy() const override {
            return new Lock(_first->copy());
        }
    };

    class Unlock : public UnaryInstruction<LocationReference> {
    public:
        explicit Unlock(LocationReference* loc) :
            UnaryInstruction<LocationReference>(Tag::UNLOCK, useref(loc)) {}
        ~Unlock() override { freeref(_first); }
        [[nodiscard]] Unlock* copy() const override {
            return new Unlock(_first->copy());
        }
    };

    class IsEqual : public BinaryReferenceInstruction {
    public:
        IsEqual(Reference* lhs, Reference* rhs) :
            BinaryReferenceInstruction(Tag::EQUAL, lhs, rhs) {}
        [[nodiscard]] IsEqual* copy() const override {
            return new IsEqual(_first->copy(), _second->copy());
        }
    };

    class ScopeOf : public UnaryInstruction<LocationReference> {
    public:
        explicit ScopeOf(LocationReference* loc) :
            UnaryInstruction<LocationReference>(Tag::SCOPEOF, useref(loc)) {}
        ~ScopeOf() override { freeref(_first); }
        [[nodiscard]] ScopeOf* copy() const override {
            return new ScopeOf(_first->copy());
        }
    };

}

#endif //SWARMVM_STORAGE
