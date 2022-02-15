#ifndef SWARM_022_LOCAL_STORE_LVAL_H
#define SWARM_022_LOCAL_STORE_LVAL_H

#include <cassert>
#include "Test.h"
#include "../lang/AST.h"
#include "../runtime/LocalSymbolValueStore.h"

namespace swarmc {
namespace Test {

    class LocalStoreLValTest : Test {
    public:
        LocalStoreLValTest() : Test() {}

        bool run() override {
            Runtime::LocalSymbolValueStore store;

            Lang::MapBody body;
            Lang::MapNode map(nullptr, &body);

            Lang::VariableSymbol sym("myMap", nullptr, nullptr);

            store.setValue(&sym, &map);

            auto value = store.getValue(&sym);
            assert(value != nullptr && value == &map);

            Lang::IdentifierNode id(nullptr, "myMapId");
            id.overrideSymbol(&sym);

            Lang::IdentifierNode key(nullptr, "myMapKey");
            Lang::StringLiteralExpressionNode str(nullptr, "myString");
            map.setKey(&key, &str);

            Lang::MapAccessNode accessNode(nullptr, &id, &key);

            auto idValue = id.getValue(&store);
            assert(idValue != nullptr && idValue == &map);

            auto keyValue = accessNode.getValue(&store);
            assert(keyValue->isValue() && keyValue == &str);

            Lang::StringLiteralExpressionNode str2(nullptr, "myStr2");
            accessNode.setValue(&store, &str2);

            auto keyValue2 = accessNode.getValue(&store);
            assert(keyValue->isValue() && keyValue2 == &str2);

            return true;
        }
    };

}
}

#endif //SWARM_022_LOCAL_STORE_LVAL_H
