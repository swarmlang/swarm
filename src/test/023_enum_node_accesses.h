#ifndef SWARM_023_ENUM_NODE_ACCESSES_H
#define SWARM_023_ENUM_NODE_ACCESSES_H

#include "Test.h"
#include "../lang/AST.h"

namespace swarmc {
namespace Test {

    class EnumNodeAccessesTest : Test {
    public:
        EnumNodeAccessesTest() : Test() {}

        bool run() override {
            Lang::ExpressionList body;
            Lang::EnumerationLiteralExpressionNode node(nullptr, &body);
            Lang::StringLiteralExpressionNode str1(nullptr, "string 1");
            Lang::StringLiteralExpressionNode str2(nullptr, "string 2");

            assert(!node.hasIndex(0));
            assert(!node.hasIndex(1));
            assert(node.isEmpty());

            node.push(&str1);

            assert(node.hasIndex(0));
            assert(!node.hasIndex(1));
            assert(!node.isEmpty());
            assert(node.getIndex(0) == &str1);

            node.setIndex(0, &str2);

            assert(node.hasIndex(0));
            assert(!node.hasIndex(1));
            assert(!node.isEmpty());
            assert(node.getIndex(0) == &str2);

            node.pop();

            assert(!node.hasIndex(0));
            assert(!node.hasIndex(1));
            assert(node.isEmpty());

            return true;
        }
    };

}
}

#endif //SWARM_023_ENUM_NODE_ACCESSES_H
