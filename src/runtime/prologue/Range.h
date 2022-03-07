#ifndef SWARM_RANGE_H
#define SWARM_RANGE_H

#include "../../lang/AST.h"
#include "IPrologueFunction.h"

namespace swarmc {
namespace Runtime {
namespace Prologue {

    class Range : public IPrologueFunction {
    public:
        Range() : IPrologueFunction("range") {}

        ExpressionNode* call(ExpressionList *args) override {
            auto node = new EnumerationLiteralExpressionNode(getNewPosition(), new ExpressionList);

            auto startAt = (NumberLiteralExpressionNode*) args->at(0);
            auto endAt = (NumberLiteralExpressionNode*) args->at(1);
            auto everyNth = (NumberLiteralExpressionNode*) args->at(2);

            for ( double i = startAt->value(); i <= endAt->value(); i += everyNth->value() ) {
                node->push(new NumberLiteralExpressionNode(getNewPosition(), i));
            }

            return node;
        }

        const FunctionType* type() const override {
            auto ft = FunctionType::of(GenericType::of(Lang::ValueType::TENUMERABLE, PrimitiveType::of(Lang::ValueType::TNUM)));
            ft->addArgument(PrimitiveType::of(Lang::ValueType::TNUM));
            ft->addArgument(PrimitiveType::of(Lang::ValueType::TNUM));
            ft->addArgument(PrimitiveType::of(Lang::ValueType::TNUM));
            return ft;
        }
    };

}
}
}

#endif //SWARM_RANGE_H
