#ifndef SWARMVM_RANGE
#define SWARMVM_RANGE

#include "prologue_provider.h"
#include "../isa_meta.h"

namespace swarmc::Runtime::Prologue {

    class RangeFunctionCall : public PrologueFunctionCall {
    public:
        RangeFunctionCall(IProvider* provider, CallVector vector, CType returnType):
            PrologueFunctionCall(provider, vector, returnType) {}

        void execute() override;

        std::string toString() const override {
            return "RangeFunctionCall<>";
        }
    };

    class RangeFunction : public PrologueFunction {
    public:
        RangeFunction(IProvider* provider) : PrologueFunction(provider) {}

        FormalTypes paramTypes() const override;

        CType returnType() const override;

        PrologueFunctionCall* call(CallVector) const override;

        std::string toString() const override {
            return "RangeFunction<>";
        }
    };

}

#endif //SWARMVM_RANGE
