#ifndef SWARM_BOOLEAN
#define SWARM_BOOLEAN

#include "../ISA.h"

namespace swarmc::ISA {

    class And : public BinaryInstruction<Reference, Reference> {
    public:
        And(Reference* lhs, Reference* rhs) :
            BinaryInstruction<Reference, Reference>(Tag::AND, lhs, rhs) {}
    };

    class Or : public BinaryInstruction<Reference, Reference> {
    public:
        Or(Reference* lhs, Reference* rhs) :
            BinaryInstruction<Reference, Reference>(Tag::OR, lhs, rhs) {}
    };

    class Xor : public BinaryInstruction<Reference, Reference> {
    public:
        Xor(Reference* lhs, Reference* rhs) :
            BinaryInstruction<Reference, Reference>(Tag::XOR, lhs, rhs) {}
    };

    class Nand : public BinaryInstruction<Reference, Reference> {
    public:
        Nand(Reference* lhs, Reference* rhs) :
            BinaryInstruction<Reference, Reference>(Tag::NAND, lhs, rhs) {}
    };

    class Nor : public BinaryInstruction<Reference, Reference> {
    public:
        Nor(Reference* lhs, Reference* rhs) :
            BinaryInstruction<Reference, Reference>(Tag::NOR, lhs, rhs) {}
    };

    class Not : public UnaryInstruction<Reference> {
    public:
        Not(Reference* value) :
            UnaryInstruction<Reference>(Tag::NOT, value) {}
    };

}

#endif //SWARM_BOOLEAN
