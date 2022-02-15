#ifndef SWARM_TEST_H
#define SWARM_TEST_H

#include "../shared/util/Console.h"

namespace swarmc {
namespace Test {

    class Test : public IUsesConsole {
    public:
        Test() : IUsesConsole() {};

        ~Test() {}

        virtual bool run() = 0;
    };

}
}

#endif //SWARM_TEST_H
