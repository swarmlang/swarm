#ifndef SWARMVM_RANGE
#define SWARMVM_RANGE

#include <utility>

#include "prologue_provider.h"
#include "../isa_meta.h"

namespace swarmc::Runtime::Prologue {

    class RangeFunctionCall : public PrologueFunctionCall {
    public:
        RangeFunctionCall(IProvider* provider, CallVector vector, const Type::Type* returnType):
            PrologueFunctionCall(provider, std::move(vector), returnType) {}

        void execute() override;

        [[nodiscard]] std::string toString() const override {
            return "RangeFunctionCall<>";
        }
    };

    class RangeFunction : public PrologueFunction {
    public:
        explicit RangeFunction(IProvider* provider) : PrologueFunction("RANGE", provider) {}

        [[nodiscard]] FormalTypes paramTypes() const override;

        [[nodiscard]] const Type::Type* returnType() const override;

        [[nodiscard]] PrologueFunctionCall* call(CallVector) const override;

        [[nodiscard]] std::string toString() const override {
            return "RangeFunction<>";
        }
    };

}

#endif //SWARMVM_RANGE
