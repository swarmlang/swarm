#ifndef SWARMC_TYPETABLE_H
#define SWARMC_TYPETABLE_H

#include <map>
#include "../shared/nslib.h"
#include "../errors/SwarmError.h"
#include "./Type.h"

using namespace nslib;

namespace swarmc::Lang {

    class ASTNode;

    using TypeMap = std::map<const ASTNode*, const Type::Type*>;

    class TypeTable : public IStringable {
    public:
        TypeTable() = default;

        ~TypeTable() override = default;

        void setTypeOf(const ASTNode* node, const Type::Type* type) {
            _map[node] = type;
        }

        const Type::Type* getTypeOf(const ASTNode* node) {
            const Type::Type* type = _map[node];
            if ( type == nullptr ) {
                throw Errors::SwarmError("Unable to determine type for node");
            }

            return type;
        }

        [[nodiscard]] std::string toString() const override {
            return "TypeTable<#entries: " + std::to_string(_map.size()) + ">";
        }
    protected:
        TypeMap _map;
    };

}

#endif
