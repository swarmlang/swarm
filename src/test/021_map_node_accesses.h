#ifndef SWARM_021_MAP_NODE_ACCESSES_H
#define SWARM_021_MAP_NODE_ACCESSES_H

#include "Test.h"
#include "../lang/AST.h"

namespace swarmc {
namespace Test {

    class MapNodeAccessesTest : Test {
    public:
        MapNodeAccessesTest() : Test() {}

        bool run() override {
            Lang::MapBody body;
            Lang::MapNode node(nullptr, &body);
            Lang::StringLiteralExpressionNode str1(nullptr, "string 1");
            Lang::IdentifierNode key1(nullptr, "key 1");
            Lang::IdentifierNode key2(nullptr, "key 2");

            assert(node.getKey(&key1) == nullptr);

            node.setKey(&key1, &str1);

            assert(node.getKey(&key2) == nullptr);

            auto value1 = node.getKey(&key1);
            assert(value1 != nullptr);
            assert(value1->getTag() == str1.getTag());
            assert(((Lang::StringLiteralExpressionNode*) value1)->value() == str1.value());

            return true;
        }
    };

}
}

#endif //SWARM_021_MAP_NODE_ACCESSES_H
