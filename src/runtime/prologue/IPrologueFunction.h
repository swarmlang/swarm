#ifndef SWARM_IPROLOGUEFUNCTION_H
#define SWARM_IPROLOGUEFUNCTION_H

#include <utility>

#include "../../shared/IStringable.h"
#include "../../lang/SymbolTable.h"
#include "../../lang/AST.h"

using namespace swarmc::Lang;

namespace swarmc {
namespace Runtime {
namespace Prologue {

    class IPrologueFunction : public IStringable {
    public:
        static IPrologueFunction* resolveByName(std::string);

        static void buildScope(Lang::ScopeTable*);

        explicit IPrologueFunction(std::string name) : _name(name) {}

        virtual bool validateCall(const ExpressionList* args) const {
            auto fnType = type();
            auto argTypes = fnType->getArgumentTypes();

            if ( args->size() != argTypes->size() ) return false;
            for ( size_t i = 0; i < args->size(); i += 1 ) {
                if ( !args->at(i)->type()->is(argTypes->at(i)) ) {
                    return false;
                }
            }

            return true;
        }

        virtual ExpressionNode* call(ExpressionList* args) = 0;

        virtual const FunctionType* type() const = 0;

        virtual std::string name() const {
            return _name;
        }

        virtual std::string toString() const {
            return "Prologue<" + _name + ">";
        }

        virtual Position* position() const {
            return getNewPosition();
        }
    protected:
        std::string _name;

        ProloguePosition* getNewPosition() const {
            return new ProloguePosition(_name);
        }

        UnitNode* getNewUnit() const {
            return new UnitNode(getNewPosition());
        }
    };

}
}
}

#endif //SWARM_IPROLOGUEFUNCTION_H
