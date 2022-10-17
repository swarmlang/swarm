#ifndef SWARMVM_STORAGE
#define SWARMVM_STORAGE

#include "../ISA.h"

namespace swarmc::ISA {

    class Typify : public BinaryInstruction<LocationReference, Reference> {
    public:
        Typify(LocationReference* loc, Reference* type) :
            BinaryInstruction<LocationReference, Reference>(Tag::TYPIFY, loc, type) {}
    };

    class AssignValue : public BinaryInstruction<LocationReference, Reference> {
    public:
        AssignValue(LocationReference* loc, Reference* value) :
            BinaryInstruction<LocationReference, Reference>(Tag::ASSIGNVALUE, loc, value) {}
    };

    class AssignEval : public BinaryInstruction<LocationReference, Instruction> {
    public:
        AssignEval(LocationReference* loc, Instruction* exe) :
            BinaryInstruction<LocationReference, Instruction>(Tag::ASSIGNEVAL, loc, exe) {}
    };

    class Lock : public UnaryInstruction<LocationReference> {
    public:
        Lock(LocationReference* loc) :
            UnaryInstruction<LocationReference>(Tag::LOCK, loc) {}
    };

    class Unlock : public UnaryInstruction<LocationReference> {
    public:
        Unlock(LocationReference* loc) :
            UnaryInstruction<LocationReference>(Tag::UNLOCK, loc) {}
    };

    class IsEqual : public BinaryInstruction<Reference, Reference> {
    public:
        IsEqual(Reference* lhs, Reference* rhs) :
            BinaryInstruction<Reference, Reference>(Tag::EQUAL, lhs, rhs) {}
    };

    class ScopeOf : public UnaryInstruction<LocationReference> {
    public:
        ScopeOf(LocationReference* loc) :
            UnaryInstruction<LocationReference>(Tag::SCOPEOF, loc) {}
    };

}

#endif //SWARMVM_STORAGE
