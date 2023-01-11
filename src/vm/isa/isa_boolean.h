#ifndef SWARM_BOOLEAN
#define SWARM_BOOLEAN

#include "../ISA.h"

namespace swarmc::ISA {

    class And : public BinaryReferenceInstruction {
    public:
        And(Reference* lhs, Reference* rhs) :
            BinaryReferenceInstruction(Tag::AND, lhs, rhs) {}
        [[nodiscard]] And* copy() const override {
            return new And(_first->copy(), _second->copy());
        }
    };

    class Or : public BinaryReferenceInstruction {
    public:
        Or(Reference* lhs, Reference* rhs) :
            BinaryReferenceInstruction(Tag::OR, lhs, rhs) {}
        [[nodiscard]] Or* copy() const override {
            return new Or(_first->copy(), _second->copy());
        }
    };

    class Xor : public BinaryReferenceInstruction {
    public:
        Xor(Reference* lhs, Reference* rhs) :
            BinaryReferenceInstruction(Tag::XOR, lhs, rhs) {}
        [[nodiscard]] Xor* copy() const override {
            return new Xor(_first->copy(), _second->copy());
        }
    };

    class Nand : public BinaryReferenceInstruction {
    public:
        Nand(Reference* lhs, Reference* rhs) :
            BinaryReferenceInstruction(Tag::NAND, lhs, rhs) {}
        [[nodiscard]] Nand* copy() const override {
            return new Nand(_first->copy(), _second->copy());
        }
    };

    class Nor : public BinaryReferenceInstruction {
    public:
        Nor(Reference* lhs, Reference* rhs) :
            BinaryReferenceInstruction(Tag::NOR, lhs, rhs) {}
        [[nodiscard]] Nor* copy() const override {
            return new Nor(_first->copy(), _second->copy());
        }
    };

    class Not : public UnaryInstruction<Reference> {
    public:
        explicit Not(Reference* value) :
            UnaryInstruction<Reference>(Tag::NOT, useref(value)) {}
        ~Not() override {
            freeref(_first);
        }
        [[nodiscard]] Not* copy() const override {
            return new Not(_first->copy());
        }
    };

}

#endif //SWARM_BOOLEAN
