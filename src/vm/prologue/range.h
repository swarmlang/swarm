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

        std::string toString() const override {
            return "RangeFunctionCall<>";
        }
    };

    class RangeFunction : public PrologueFunction {
    public:
        explicit RangeFunction(IProvider* provider) : PrologueFunction(provider) {}

        FormalTypes paramTypes() const override;

        const Type::Type* returnType() const override;

        PrologueFunctionCall* call(CallVector) const override;

        std::string toString() const override {
            return "RangeFunction<>";
        }
    };

}

#endif //SWARMVM_RANGE
