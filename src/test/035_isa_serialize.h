#ifndef SWARM_035_ISA_SERIALIZE_H
#define SWARM_035_ISA_SERIALIZE_H

#include <sstream>
#include "Test.h"
#include "../vm/isa_meta.h"
#include "../vm/walk/ISASerializeWalk.h"

namespace swarmc {
namespace Test {

    class ISASerializationTest : public Test {
    public:
        ISASerializationTest() : Test() {}

        bool run() override {
            ISA::Instructions is = {
                new ISA::AssignValue(
                    ISA::local("a"),
                    ISA::number(2)
                ),
                new ISA::AssignValue(
                    ISA::local("b"),
                    ISA::number(3)
                ),
                new ISA::AssignEval(
                    ISA::local("isEq"),
                    ISA::isEqual(ISA::local("a"), ISA::local("b"))
                )
            };

            ISA::ISASerializeWalk walk;

            auto result = walk.walk(is);
            return true;
        }
    };

}
}

#endif