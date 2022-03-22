#ifndef SWARM_030_LOCAL_SYMBOL_SERIALIZE_H
#define SWARM_030_LOCAL_SYMBOL_SERIALIZE_H

#include <sstream>
#include "Test.h"
#include "../shared/uuid.h"
#include "../shared/util/Console.h"
#include "../lang/SymbolTable.h"
#include "../lang/AST.h"
#include "../lang/Walk/SymbolWalk.h"
#include "../runtime/LocalSymbolValueStore.h"

namespace swarmc {
namespace Test {

    class LocalSymbolSerialize : public Test {
    public:
        bool run() override {
            util::USE_DETERMINISTIC_UUIDS = true;
            std::stringstream input;
            input << "number pi = 3.14;\nstring str = \"string 1\";\nnumber n = pi * 3;\n";

            Pipeline pipeline(&input);

            Lang::ProgramNode* pgNode = pipeline.targetASTSymbolicTyped();

            Runtime::LocalSymbolValueStore locals;
            Lang::Walk::SymbolWalk walk;

            assert(pgNode->body()->at(0)->getName() == "VariableDeclarationNode");
            assert(pgNode->body()->at(1)->getName() == "VariableDeclarationNode");
            assert(pgNode->body()->at(2)->getName() == "VariableDeclarationNode");

            VariableDeclarationNode* assign1 = (VariableDeclarationNode*)pgNode->body()->at(0);
            VariableDeclarationNode* assign2 = (VariableDeclarationNode*)pgNode->body()->at(1);
            VariableDeclarationNode* assign3 = (VariableDeclarationNode*)pgNode->body()->at(2);

            NumberLiteralExpressionNode nvalue(assign3->value()->position(), 9.42);

            locals.setValue(assign1->id()->symbol(), assign1->value());
            locals.setValue(assign2->id()->symbol(), assign2->value());
            locals.setValue(assign3->id()->symbol(), &nvalue);

            std::string serial = locals.serialize(walk.walk(pgNode));
            console->debug(serial);

            Runtime::LocalSymbolValueStore locals2;
            locals2.deserialize(serial);

            Console::get()->debug()
                ->info(locals.getValue(assign1->id()->symbol())->toString())
                ->info(locals2.getValue(assign1->id()->symbol())->toString())
                ->end();

            assert(locals2.getValue(assign1->id()->symbol())->equals(locals.getValue(assign1->id()->symbol())));
            assert(locals2.getValue(assign2->id()->symbol())->equals(locals.getValue(assign2->id()->symbol())));
            assert(locals2.getValue(assign3->id()->symbol())->equals(locals.getValue(assign3->id()->symbol())));

            return true;
        }
    };

}
}

#endif //SWARM_030_LOCAL_SYMBOL_SERIALIZE_H
