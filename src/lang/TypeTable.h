#ifndef SWARMC_TYPETABLE_H
#define SWARMC_TYPETABLE_H

#include <map>
#include "../shared/IStringable.h"
#include "../errors/SwarmError.h"
#include "./Type.h"

namespace swarmc {
namespace Lang {

    class ASTNode;

    using TypeMap = std::map<const ASTNode*, const Type*>;

    class TypeTable : public IStringable {
    public:
        TypeTable() {}

        virtual ~TypeTable() {}

        void setTypeOf(const ASTNode* node, const Type* type) {
            _map[node] = type;
        }

        const Type* getTypeOf(const ASTNode* node) {
            const Type* type = _map[node];
            if ( type == nullptr ) {
                throw Errors::SwarmError("Unable to determine type for node");
            }
        }

        std::string toString() const override {
            return "TypeTable<#entries: " + std::to_string(_map.size()) + ">";
        }
    protected:
        TypeMap _map;
    };

}
}

#endif
