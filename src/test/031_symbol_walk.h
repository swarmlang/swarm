#ifndef SWARM_031_SYMBOL_WALK_H
#define SWARM_031_SYMBOL_WALK_H

#include <sstream>
#include "Test.h"
#include "../lang/Walk/SymbolWalk.h"

namespace swarmc {
namespace Test {

    class SymbolWalkTest : public Test {
    public:
        SymbolWalkTest() : Test() {}

        bool run() override {
            std::stringstream input;
            input << R"END(
number a = 69;
map<string> b = {} of string;
enumerate b as c {
    string d = c;
}
            )END";

            Pipeline pipeline(&input);
            Lang::ProgramNode* pgNode = pipeline.targetASTSymbolicTyped();

            Lang::Walk::SymbolWalk walk;
            Lang::Walk::SymbolMap* symbols = walk.walk(pgNode);

            for (auto s : *symbols) {
                assert(s.first == s.second->uuid());
            }

            assert(symbols->size() == 4);

            return true;
        }
    };

}
}

#endif