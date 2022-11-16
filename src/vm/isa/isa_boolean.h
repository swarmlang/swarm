#ifndef SWARM_BOOLEAN
#define SWARM_BOOLEAN

#include "../ISA.h"

namespace swarmc::ISA {

    class And : public BinaryInstruction<Reference, Reference> {
    public:
        And(Reference* lhs, Reference* rhs) :
            BinaryInstruction<Reference, Reference>(Tag::AND, lhs, rhs) {}
        virtual And* copy() const override {
            return new And(_first->copy(), _second->copy());
        }
    };

    class Or : public BinaryInstruction<Reference, Reference> {
    public:
        Or(Reference* lhs, Reference* rhs) :
            BinaryInstruction<Reference, Reference>(Tag::OR, lhs, rhs) {}
        virtual Or* copy() const override {
            return new Or(_first->copy(), _second->copy());
        }
    };

    class Xor : public BinaryInstruction<Reference, Reference> {
    public:
        Xor(Reference* lhs, Reference* rhs) :
            BinaryInstruction<Reference, Reference>(Tag::XOR, lhs, rhs) {}
        virtual Xor* copy() const override {
            return new Xor(_first->copy(), _second->copy());
        }
    };

    class Nand : public BinaryInstruction<Reference, Reference> {
    public:
        Nand(Reference* lhs, Reference* rhs) :
            BinaryInstruction<Reference, Reference>(Tag::NAND, lhs, rhs) {}
        virtual Nand* copy() const override {
            return new Nand(_first->copy(), _second->copy());
        }
    };

    class Nor : public BinaryInstruction<Reference, Reference> {
    public:
        Nor(Reference* lhs, Reference* rhs) :
            BinaryInstruction<Reference, Reference>(Tag::NOR, lhs, rhs) {}
        virtual Nor* copy() const override {
            return new Nor(_first->copy(), _second->copy());
        }
    };

    class Not : public UnaryInstruction<Reference> {
    public:
        Not(Reference* value) :
            UnaryInstruction<Reference>(Tag::NOT, value) {}
        virtual Not* copy() const override {
            return new Not(_first->copy());
        }
    };

}

#endif //SWARM_BOOLEAN
