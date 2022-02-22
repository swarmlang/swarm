#ifndef SWARM_ISWARMFUNCTION_H
#define SWARM_ISWARMFUNCTION_H

#include "../shared/IStringable.h"
#include "../lang/AST.h"

namespace swarmc {
namespace Runtime {

    class ISwarmFunction : public IStringable {
    public:
        virtual bool validateCall(const std::vector<Lang::ExpressionNode*>* args) const = 0;

        virtual Lang::ExpressionNode* call(std::vector<Lang::ExpressionNode*>* args) = 0;

    protected:
        Lang::ProloguePosition* getPosition() {
            return new Lang::ProloguePosition("prologue evaluation");
        }

        Lang::UnitNode* getUnit() {
            return new Lang::UnitNode(getPosition());
        }
    };

}
}

#endif //SWARM_ISWARMFUNCTION_H
