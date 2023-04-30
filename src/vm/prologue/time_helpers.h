#ifndef SWARMVM_TIME
#define SWARMVM_TIME

#include <utility>

#include "prologue_provider.h"
#include "../../lang/Type.h"

namespace swarmc::Runtime::Prologue {

    class TimeFunctionCall : public PrologueFunctionCall {
    public:
        TimeFunctionCall(IProvider* provider, const CallVector& vector, Type::Type* returnType):
            PrologueFunctionCall(provider, "TIME", vector, returnType) {}

        void execute(VirtualMachine*) override;

        [[nodiscard]] std::string toString() const override {
            return "TimeFunctionCall<>";
        }
    };

    class TimeFunction : public PrologueFunction {
    public:
        explicit TimeFunction(IProvider* provider) : PrologueFunction("TIME", provider) {}

        [[nodiscard]] FormalTypes paramTypes() const override { return {}; }

        [[nodiscard]] Type::Type* returnType() const override {
            return Type::Primitive::of(Type::Intrinsic::NUMBER);
        }

        [[nodiscard]] PrologueFunctionCall* call(CallVector) const override;

        [[nodiscard]] std::string toString() const override {
            return "TimeFunction<>";
        }
    };
}

#endif //SWARMVM_TIME
