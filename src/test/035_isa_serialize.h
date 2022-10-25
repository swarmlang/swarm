#ifndef SWARM_035_ISA_SERIALIZE_H
#define SWARM_035_ISA_SERIALIZE_H

#include <sstream>
#include "Test.h"
#include "../vm/isa_meta.h"
#include "../vm/walk/ISASerializeWalk.h"

namespace swarmc::Test {

    class ISASerializationTest : public Test {
    public:
        ISASerializationTest() : Test() {}

        bool run() override {
            ISA::Instructions is = {
                new ISA::AssignValue(
                    new ISA::LocationReference(ISA::Affinity::LOCAL, "a"),
                    new ISA::NumberReference(2)
                ),
                new ISA::AssignValue(
                    new ISA::LocationReference(ISA::Affinity::LOCAL, "b"),
                    new ISA::NumberReference(3)
                ),
                new ISA::AssignEval(
                    new ISA::LocationReference(ISA::Affinity::LOCAL, "isEq"),
                    new ISA::IsEqual(
                        new ISA::LocationReference(ISA::Affinity::LOCAL, "a"),
                        new ISA::LocationReference(ISA::Affinity::LOCAL, "b")
                        )
                )
            };

            ISA::ISASerializeWalk walk;

            auto result = walk.walk(is);
            return true;
        }
    };

}

#endif