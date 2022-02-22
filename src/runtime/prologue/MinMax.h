#ifndef SWARM_MINMAX_H
#define SWARM_MINMAX_H

#include "IPrologueFunction.h"

namespace swarmc {
namespace Runtime {
namespace Prologue {

    class Min : public IPrologueFunction {
    public:
        Min() : IPrologueFunction("min") {}

        ExpressionNode* call(ExpressionList *args) override {
            auto lnum = ((NumberLiteralExpressionNode*) args->at(0))->value();
            auto rnum = ((NumberLiteralExpressionNode*) args->at(1))->value();
            return new NumberLiteralExpressionNode(getNewPosition(), lnum <= rnum ? lnum : rnum);
        }

        const FunctionType* type() const override {
            auto tnum = PrimitiveType::of(ValueType::TNUM);
            auto type = FunctionType::of(tnum);
            type->addArgument(tnum);
            type->addArgument(tnum);
            return type;
        }
    };

    class Max : public IPrologueFunction {
    public:
        Max() : IPrologueFunction("max") {}

        ExpressionNode* call(ExpressionList *args) override {
            auto lnum = ((NumberLiteralExpressionNode*) args->at(0))->value();
            auto rnum = ((NumberLiteralExpressionNode*) args->at(1))->value();
            return new NumberLiteralExpressionNode(getNewPosition(), lnum >= rnum ? lnum : rnum);
        }

        const FunctionType* type() const override {
            auto tnum = PrimitiveType::of(ValueType::TNUM);
            auto type = FunctionType::of(tnum);
            type->addArgument(tnum);
            type->addArgument(tnum);
            return type;
        }
    };

}
}
}

#endif //SWARM_MINMAX_H
