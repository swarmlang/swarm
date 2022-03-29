#include <cassert>
#include "../lib/json.hpp"
#include "../lang/Walk/SerializeWalk.h"
#include "../lang/Walk/DeSerializeWalk.h"
#include "LocalSymbolValueStore.h"

using namespace swarmc::Runtime;

std::string LocalSymbolValueStore::serialize(Lang::Walk::SymbolMap* symbols) {
    nlohmann::json json;
    Lang::Walk::SerializeWalk serialize;
    std::vector<std::pair<nlohmann::json, nlohmann::json>> entries;

    auto iter = symbols->begin();
    while ( iter != symbols->end() ) {
        std::string uuid = iter->first;

        auto expression = tryGetValue(iter->second);
        if (expression != nullptr) {
            auto symbolJson = serialize.walkSemanticSymbol(iter->second);
            auto exprJson = serialize.walk(expression);
            entries.push_back({*symbolJson, *exprJson});
        }

        iter++;
    }

    json["entries"] = entries;
    return json.dump(4);
}

void LocalSymbolValueStore::deserialize(std::string payload) {
    Lang::Walk::DeSerializeWalk deserialize;
    std::vector<std::pair<nlohmann::json, nlohmann::json>> entries = nlohmann::json::parse(payload)["entries"];

    for ( auto entry : entries ) {
        Lang::SemanticSymbol* symbol = deserialize.walkSymbol(entry.first);
        auto expression = deserialize.walk(entry.second);
        assert(expression->isExpression());

        // NOTE: right now, we only call this method on empty stores, so inserting raw is fine.
        //       however, if we need to call this on an existing store, will need to change this
        //       to a this->setValue(...) call to avoid duplicate symbol entries.
        _map->insert({symbol, (Lang::ExpressionNode*) expression});
    }
}
