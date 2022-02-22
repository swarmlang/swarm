#ifndef SWARM_LOG_H
#define SWARM_LOG_H

#include "../../shared/util/Console.h"
#include "IPrologueFunction.h"

namespace swarmc {
namespace Runtime {
namespace Prologue {

    class LogFunction final : public IPrologueFunction {
    public:
        LogFunction() : IPrologueFunction("log") {}

        ExpressionNode* call(ExpressionList *args) override {
            auto str = (StringLiteralExpressionNode*) args->at(0);
            Console::get()->info(str->value());
            return getNewUnit();
        }

        const FunctionType* type() const override {
            auto type = FunctionType::of(PrimitiveType::of(Lang::ValueType::TUNIT));
            type->addArgument(PrimitiveType::of(Lang::ValueType::TSTRING));
            return type;
        }
    };


    class LogErrorFunction final : public IPrologueFunction {
    public:
        LogErrorFunction() : IPrologueFunction("logError") {}

        ExpressionNode* call(ExpressionList *args) override {
            auto str = (StringLiteralExpressionNode*) args->at(0);
            Console::get()->error(str->value());
            return getNewUnit();
        }

        const FunctionType* type() const override {
            auto type = FunctionType::of(PrimitiveType::of(Lang::ValueType::TUNIT));
            type->addArgument(PrimitiveType::of(Lang::ValueType::TSTRING));
            return type;
        }
    };

}
}
}

#endif //SWARM_LOG_H
