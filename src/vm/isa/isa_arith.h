#ifndef SWARMVM_ARITH
#define SWARMVM_ARITH

#include "../ISA.h"

namespace swarmc::ISA {

    class Plus : public BinaryInstruction<Reference, Reference> {
    public:
        Plus(Reference* lhs, Reference* rhs) :
            BinaryInstruction<Reference, Reference>(Tag::PLUS, lhs, rhs) {}
        Plus* copy() const override {
            return new Plus(_first->copy(), _second->copy());
        }
    };

    class Minus : public BinaryInstruction<Reference, Reference> {
    public:
        Minus(Reference* lhs, Reference* rhs) :
            BinaryInstruction<Reference, Reference>(Tag::MINUS, lhs, rhs) {}
        Minus* copy() const override {
            return new Minus(_first->copy(), _second->copy());
        }
    };

    class Times : public BinaryInstruction<Reference, Reference> {
    public:
        Times(Reference* lhs, Reference* rhs) :
            BinaryInstruction<Reference, Reference>(Tag::TIMES, lhs, rhs) {}
        Times* copy() const override {
            return new Times(_first->copy(), _second->copy());
        }
    };

    class Divide : public BinaryInstruction<Reference, Reference> {
    public:
        Divide(Reference* lhs, Reference* rhs) :
            BinaryInstruction<Reference, Reference>(Tag::DIVIDE, lhs, rhs) {}
        Divide* copy() const override {
            return new Divide(_first->copy(), _second->copy());
        }
    };

    class Power : public BinaryInstruction<Reference, Reference> {
    public:
        Power(Reference* lhs, Reference* rhs) :
            BinaryInstruction<Reference, Reference>(Tag::POWER, lhs, rhs) {}
        Power* copy() const override {
            return new Power(_first->copy(), _second->copy());
        }
    };

    class Mod : public BinaryInstruction<Reference, Reference> {
    public:
        Mod(Reference* lhs, Reference* rhs) :
            BinaryInstruction<Reference, Reference>(Tag::MOD, lhs, rhs) {}
        Mod* copy() const override {
            return new Mod(_first->copy(), _second->copy());
        }
    };

    class Negative : public UnaryInstruction<Reference> {
    public:
        explicit Negative(Reference* value) :
            UnaryInstruction<Reference>(Tag::NEG, value) {}
        Negative* copy() const override {
            return new Negative(_first->copy());
        }
    };

    class GreaterThan : public BinaryInstruction<Reference, Reference> {
    public:
        GreaterThan(Reference* lhs, Reference* rhs) :
                BinaryInstruction<Reference, Reference>(Tag::GT, lhs, rhs) {}
        GreaterThan* copy() const override {
            return new GreaterThan(_first->copy(), _second->copy());
        }
    };

    class GreaterThanOrEqual : public BinaryInstruction<Reference, Reference> {
    public:
        GreaterThanOrEqual(Reference* lhs, Reference* rhs) :
                BinaryInstruction<Reference, Reference>(Tag::GTE, lhs, rhs) {}
        GreaterThanOrEqual* copy() const override {
            return new GreaterThanOrEqual(_first->copy(), _second->copy());
        }
    };

    class LessThan : public BinaryInstruction<Reference, Reference> {
    public:
        LessThan(Reference* lhs, Reference* rhs) :
                BinaryInstruction<Reference, Reference>(Tag::LT, lhs, rhs) {}
        LessThan* copy() const override {
            return new LessThan(_first->copy(), _second->copy());
        }
    };

    class LessThanOrEqual : public BinaryInstruction<Reference, Reference> {
    public:
        LessThanOrEqual(Reference* lhs, Reference* rhs) :
                BinaryInstruction<Reference, Reference>(Tag::LTE, lhs, rhs) {}
        LessThanOrEqual* copy() const override {
            return new LessThanOrEqual(_first->copy(), _second->copy());
        }
    };

}

#endif //SWARMVM_ARITH
