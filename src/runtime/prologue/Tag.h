#ifndef SWARM_TAG_H
#define SWARM_TAG_H

#include "../lang/AST.h"
#include "IPrologueFunction.h"

using namespace swarmc::Lang;

namespace swarmc {
namespace Runtime {

    class InterpretWalk;

namespace Prologue {

    class Tag : public IPrologueFunction {
    public:
        Tag() : IPrologueFunction("tag") {}

        const FunctionType* type() const override {
            auto returnType = GenericType::of(ValueType::TRESOURCE, PrimitiveType::of(ValueType::TUNIT));
            auto type = FunctionType::of(returnType);
            type->addArgument(PrimitiveType::of(ValueType::TSTRING));
            type->addArgument(PrimitiveType::of(ValueType::TSTRING));
            return type;
        }

        ExpressionNode* call(ExpressionList* args) override {
            auto key = (StringLiteralExpressionNode*) args->at(0);
            auto value = (StringLiteralExpressionNode*) args->at(1);
            return new TagResourceNode(getNewPosition(), key->value(), value->value());
        }
    };

}
}
}

#endif //SWARM_TAG_H
