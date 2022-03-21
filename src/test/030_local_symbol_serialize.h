#ifndef SWARM_030_LOCAL_SYMBOL_SERIALIZE_H
#define SWARM_030_LOCAL_SYMBOL_SERIALIZE_H

#include "Test.h"
#include "../shared/uuid.h"
#include "../shared/util/Console.h"
#include "../lang/SymbolTable.h"
#include "../lang/AST.h"
#include "../runtime/LocalSymbolValueStore.h"

namespace swarmc {
namespace Test {

    class LocalSymbolSerialize : public Test {
    public:
        bool run() override {
            util::USE_DETERMINISTIC_UUIDS = true;
            std::string expectedSerial = R"END({
    "entries": [
        [
            {
                "declaredAt": {
                    "endCol": 1,
                    "endLine": 1,
                    "startCol": 1,
                    "startLine": 1
                },
                "isPrologue": false,
                "kind": 0,
                "name": "testSymbol1",
                "type": {
                    "shared": false,
                    "valueType": 0
                },
                "uuid": "d-guid-0"
            },
            {
                "astNodeName": "StringLiteralExpressionNode",
                "position": {
                    "endCol": 1,
                    "endLine": 1,
                    "startCol": 1,
                    "startLine": 1
                },
                "value": "string 1"
            }
        ],
        [
            {
                "declaredAt": {
                    "endCol": 1,
                    "endLine": 1,
                    "startCol": 1,
                    "startLine": 1
                },
                "isPrologue": false,
                "kind": 0,
                "name": "testSymbol2",
                "type": {
                    "shared": false,
                    "valueType": 1
                },
                "uuid": "d-guid-1"
            },
            {
                "astNodeName": "NumberLiteralExpressionNode",
                "position": {
                    "endCol": 1,
                    "endLine": 1,
                    "startCol": 1,
                    "startLine": 1
                },
                "value": 3.14
            }
        ]
    ]
})END";

            Runtime::LocalSymbolValueStore locals;

            Lang::Position p(1, 1, 1, 1);

            Lang::VariableSymbol sym1("testSymbol1", Lang::PrimitiveType::of(Lang::ValueType::TSTRING), &p);
            Lang::VariableSymbol sym2("testSymbol2", Lang::PrimitiveType::of(Lang::ValueType::TNUM), &p);

            Lang::StringLiteralExpressionNode str1(&p, "string 1");
            Lang::NumberLiteralExpressionNode num1(&p, 3.14);

            locals.setValue(&sym1, &str1);
            locals.setValue(&sym2, &num1);

            std::string serial = locals.serialize();
            assert(serial == expectedSerial);

            Runtime::LocalSymbolValueStore locals2;
            locals2.deserialize(serial);

            Console::get()->debug()
                ->info(locals.getValue(&sym1)->toString())
                ->info(locals2.getValue(&sym1)->toString())
                ->end();

            assert(locals2.getValue(&sym1)->equals(locals.getValue(&sym1)));
            assert(locals2.getValue(&sym2)->equals(locals.getValue(&sym2)));

            return true;
        }
    };

}
}

#endif //SWARM_030_LOCAL_SYMBOL_SERIALIZE_H
