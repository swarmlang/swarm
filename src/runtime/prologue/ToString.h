#ifndef SWARM_TOSTRING_H
#define SWARM_TOSTRING_H

#include "IPrologueFunction.h"

namespace swarmc {
namespace Runtime {
namespace Prologue {

    class NumberToString final : public IPrologueFunction {
    public:
        NumberToString() : IPrologueFunction("numberToString") {}

        ExpressionNode* call(ExpressionList *args) override {
            auto num = (NumberLiteralExpressionNode*) args->at(0);
            return new StringLiteralExpressionNode(getNewPosition(), std::to_string(num->value()));
        }

        const FunctionType* type() const override {
            auto type = FunctionType::of(PrimitiveType::of(Lang::ValueType::TSTRING));
            type->addArgument(PrimitiveType::of(Lang::ValueType::TNUM));
            return type;
        }
    };


    class BoolToString final : public IPrologueFunction {
    public:
        BoolToString() : IPrologueFunction("boolToString") {}

        ExpressionNode* call(ExpressionList *args) override {
            auto boolean = (BooleanLiteralExpressionNode*) args->at(0);
            return new StringLiteralExpressionNode(getNewPosition(), boolean ? "true" : "false");
        }

        const FunctionType* type() const override {
            auto type = FunctionType::of(PrimitiveType::of(Lang::ValueType::TSTRING));
            type->addArgument(PrimitiveType::of(Lang::ValueType::TBOOL));
            return type;
        }
    };

}
}
}

#endif //SWARM_TOSTRING_H
