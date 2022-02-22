#ifndef SWARM_RANDOM_H
#define SWARM_RANDOM_H

#include "../../shared/rand.h"
#include "../../lang/AST.h"
#include "IPrologueFunction.h"

namespace swarmc {
namespace Runtime {
namespace Prologue {

    class Random : public IPrologueFunction {
    public:
        Random() : IPrologueFunction("random") {}

        ExpressionNode* call(ExpressionList *args) override {
            return new NumberLiteralExpressionNode(getNewPosition(), randomDouble());
        }

        const FunctionType* type() const override {
            return FunctionType::of(PrimitiveType::of(Lang::ValueType::TNUM));
        }
    };

}
}
}

#endif //SWARM_RANDOM_H
