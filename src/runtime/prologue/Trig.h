#ifndef SWARM_TRIG_H
#define SWARM_TRIG_H

#include <cmath>
#include "../../shared/rand.h"
#include "../../lang/AST.h"
#include "IPrologueFunction.h"

namespace swarmc {
namespace Runtime {
namespace Prologue {

    class Sin : public IPrologueFunction {
    public:
        Sin() : IPrologueFunction("sin") {}

        ExpressionNode* call(ExpressionList *args) override {
            auto num = (NumberLiteralExpressionNode*) args->at(0);
            return new NumberLiteralExpressionNode(getNewPosition(), sin(num->value()));
        }

        const FunctionType* type() const override {
            auto ft = FunctionType::of(PrimitiveType::of(Lang::ValueType::TNUM));
            ft->addArgument(PrimitiveType::of(Lang::ValueType::TNUM));
            return ft;
        }
    };

}
}
}

#endif //SWARM_TRIG_H
