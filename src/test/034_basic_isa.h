#ifndef SWARM_034_BASIC_ISA_H
#define SWARM_034_BASIC_ISA_H

#include <sstream>
#include "Test.h"
#include "../vm/isa_meta.h"

namespace swarmc::Test {

    class BasicISATest : public Test {
    public:
        BasicISATest() : Test() {}

        bool run() override {
            /*
             * $l:a <- 2
             * $l:b <- 3
             * $l:isEq <- equal $l:a $l:b
             */
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

            return true;
        }
    };

}

#endif