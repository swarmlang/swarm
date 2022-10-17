#ifndef SWARM_RUNNER_H
#define SWARM_RUNNER_H

#include <string>
#include "Test.h"
#include "021_map_node_accesses.h"
#include "023_enum_node_accesses.h"
#include "024_shared_variables.h"
#include "031_symbol_walk.h"
#include "034_basic_isa.h"

namespace swarmc {
namespace Test {

    class Runner {
    public:
        Runner() {};

        ~Runner() {}

        virtual bool run(std::string name) {
            if ( name == "021_map_node_accesses" ) {
                MapNodeAccessesTest test;
                return test.run();
            } else if ( name == "023_enum_node_accesses" ) {
                EnumNodeAccessesTest test;
                return test.run();
            } else if ( name == "024_shared_variables" ) {
                SharedVariablesTest test;
                return test.run();
            } else if ( name == "031_symbol_walk" ) {
                SymbolWalkTest test;
                return test.run();
            } else if ( name == "034_basic_isa" ) {
                BasicISATest test;
                return test.run();
            }

            return false;
        }
    };

}
}

#endif //SWARM_RUNNER_H
