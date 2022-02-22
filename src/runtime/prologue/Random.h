#ifndef SWARM_RANDOM_H
#define SWARM_RANDOM_H

#include "../../lang/AST.h"
#include "../ISwarmFunction.h"

namespace swarmc {
namespace Runtime {
namespace Prologue {

    class Random : public ISwarmFunction {
    public:
        virtual bool validateCall(const std::vector<Lang::ExpressionNode*>* args) const {
            return args->empty();
        }

        virtual Lang::ExpressionNode* call(std::vector<Lang::ExpressionNode*>* args) {
            std::random_device dev;
            std::mt19937 rng(dev());
            return new Lang::NumberLiteralExpressionNode(getPosition(), rng());
        }
    };

}
}
}

#endif //SWARM_RANDOM_H
