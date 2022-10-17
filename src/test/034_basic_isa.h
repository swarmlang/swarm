#ifndef SWARM_032_JOB_WORKER_FILTERS_H
#define SWARM_032_JOB_WORKER_FILTERS_H

#include <sstream>
#include "Test.h"
#include "../vm/isa_meta.h"

namespace swarmc {
namespace Test {

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

            return true;
        }
    };

}
}

#endif