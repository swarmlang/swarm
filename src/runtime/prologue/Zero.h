#ifndef SWARM_ZERO_H
#define SWARM_ZERO_H

#include "../../lang/AST.h"
#include "../ISwarmFunction.h"

namespace swarmc {
namespace Runtime {
namespace Prologue {

    class Zero : public ISwarmFunction {
    public:
        virtual bool validateCall(const std::vector<Lang::ExpressionNode*>* args) const {
            return args->size() < 3;  // supports zero, zero1, zero2
        }

        virtual Lang::ExpressionNode* call(std::vector<Lang::ExpressionNode*>* args) {
            return new Lang::NumberLiteralExpressionNode(getPosition(), 0.0);
        }
    };

}
}
}

#endif //SWARM_ZERO_H
