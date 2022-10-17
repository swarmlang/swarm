#ifndef SWARMVM_ARITH
#define SWARMVM_ARITH

#include "../ISA.h"

namespace swarmc::ISA {

    class Plus : public BinaryInstruction<Reference, Reference> {
    public:
        Plus(Reference* lhs, Reference* rhs) :
            BinaryInstruction<Reference, Reference>(Tag::PLUS, lhs, rhs) {}
    };

    class Minus : public BinaryInstruction<Reference, Reference> {
    public:
        Minus(Reference* lhs, Reference* rhs) :
            BinaryInstruction<Reference, Reference>(Tag::MINUS, lhs, rhs) {}
    };

    class Times : public BinaryInstruction<Reference, Reference> {
    public:
        Times(Reference* lhs, Reference* rhs) :
            BinaryInstruction<Reference, Reference>(Tag::TIMES, lhs, rhs) {}
    };

    class Divide : public BinaryInstruction<Reference, Reference> {
    public:
        Divide(Reference* lhs, Reference* rhs) :
            BinaryInstruction<Reference, Reference>(Tag::DIVIDE, lhs, rhs) {}
    };

    class Power : public BinaryInstruction<Reference, Reference> {
    public:
        Power(Reference* lhs, Reference* rhs) :
            BinaryInstruction<Reference, Reference>(Tag::POWER, lhs, rhs) {}
    };

    class Mod : public BinaryInstruction<Reference, Reference> {
    public:
        Mod(Reference* lhs, Reference* rhs) :
            BinaryInstruction<Reference, Reference>(Tag::MOD, lhs, rhs) {}
    };

    class Negative : public UnaryInstruction<Reference> {
    public:
        Negative(Reference* value) :
            UnaryInstruction<Reference>(Tag::NEG, value) {}
    };

    class GreaterThan : public BinaryInstruction<Reference, Reference> {
    public:
        GreaterThan(Reference* lhs, Reference* rhs) :
                BinaryInstruction<Reference, Reference>(Tag::GT, lhs, rhs) {}
    };

    class GreaterThanOrEqual : public BinaryInstruction<Reference, Reference> {
    public:
        GreaterThanOrEqual(Reference* lhs, Reference* rhs) :
                BinaryInstruction<Reference, Reference>(Tag::GTE, lhs, rhs) {}
    };

    class LessThan : public BinaryInstruction<Reference, Reference> {
    public:
        LessThan(Reference* lhs, Reference* rhs) :
                BinaryInstruction<Reference, Reference>(Tag::LT, lhs, rhs) {}
    };

    class LessThanOrEqual : public BinaryInstruction<Reference, Reference> {
    public:
        LessThanOrEqual(Reference* lhs, Reference* rhs) :
                BinaryInstruction<Reference, Reference>(Tag::LTE, lhs, rhs) {}
    };

}

#endif //SWARMVM_ARITH
