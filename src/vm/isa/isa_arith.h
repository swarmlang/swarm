#ifndef SWARMVM_ARITH
#define SWARMVM_ARITH

#include "../ISA.h"

namespace swarmc::ISA {

    class Plus : public BinaryReferenceInstruction {
    public:
        Plus(Reference* lhs, Reference* rhs) :
            BinaryReferenceInstruction(Tag::PLUS, lhs, rhs) {}
        [[nodiscard]] Plus* copy() const override {
            return new Plus(_first->copy(), _second->copy());
        }
    };

    class Minus : public BinaryReferenceInstruction {
    public:
        Minus(Reference* lhs, Reference* rhs) :
            BinaryReferenceInstruction(Tag::MINUS, lhs, rhs) {}
        [[nodiscard]] Minus* copy() const override {
            return new Minus(_first->copy(), _second->copy());
        }
    };

    class Times : public BinaryReferenceInstruction {
    public:
        Times(Reference* lhs, Reference* rhs) :
            BinaryReferenceInstruction(Tag::TIMES, lhs, rhs) {}
        [[nodiscard]] Times* copy() const override {
            return new Times(_first->copy(), _second->copy());
        }
    };

    class Divide : public BinaryReferenceInstruction {
    public:
        Divide(Reference* lhs, Reference* rhs) :
            BinaryReferenceInstruction(Tag::DIVIDE, lhs, rhs) {}
        [[nodiscard]] Divide* copy() const override {
            return new Divide(_first->copy(), _second->copy());
        }
    };

    class Power : public BinaryReferenceInstruction {
    public:
        Power(Reference* lhs, Reference* rhs) :
            BinaryReferenceInstruction(Tag::POWER, lhs, rhs) {}
        [[nodiscard]] Power* copy() const override {
            return new Power(_first->copy(), _second->copy());
        }
    };

    class Mod : public BinaryReferenceInstruction {
    public:
        Mod(Reference* lhs, Reference* rhs) :
            BinaryReferenceInstruction(Tag::MOD, lhs, rhs) {}
        [[nodiscard]] Mod* copy() const override {
            return new Mod(_first->copy(), _second->copy());
        }
    };

    class Negative : public UnaryInstruction<Reference> {
    public:
        explicit Negative(Reference* value) :
            UnaryInstruction<Reference>(Tag::NEG, useref(value)) {}
        ~Negative() override {
            freeref(_first);
        }
        [[nodiscard]] Negative* copy() const override {
            return new Negative(_first->copy());
        }
    };

    class GreaterThan : public BinaryReferenceInstruction {
    public:
        GreaterThan(Reference* lhs, Reference* rhs) :
                BinaryReferenceInstruction(Tag::GT, lhs, rhs) {}
        [[nodiscard]] GreaterThan* copy() const override {
            return new GreaterThan(_first->copy(), _second->copy());
        }
    };

    class GreaterThanOrEqual : public BinaryReferenceInstruction {
    public:
        GreaterThanOrEqual(Reference* lhs, Reference* rhs) :
                BinaryReferenceInstruction(Tag::GTE, lhs, rhs) {}
        [[nodiscard]] GreaterThanOrEqual* copy() const override {
            return new GreaterThanOrEqual(_first->copy(), _second->copy());
        }
    };

    class LessThan : public BinaryReferenceInstruction {
    public:
        LessThan(Reference* lhs, Reference* rhs) :
                BinaryReferenceInstruction(Tag::LT, lhs, rhs) {}
        [[nodiscard]] LessThan* copy() const override {
            return new LessThan(_first->copy(), _second->copy());
        }
    };

    class LessThanOrEqual : public BinaryReferenceInstruction {
    public:
        LessThanOrEqual(Reference* lhs, Reference* rhs) :
                BinaryReferenceInstruction(Tag::LTE, lhs, rhs) {}
        [[nodiscard]] LessThanOrEqual* copy() const override {
            return new LessThanOrEqual(_first->copy(), _second->copy());
        }
    };

}

#endif //SWARMVM_ARITH
