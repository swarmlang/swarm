#ifndef SWARMVM_TRIG
#define SWARMVM_TRIG

#include "prologue_provider.h"

namespace swarmc::Runtime::Prologue {
    enum class TrigOperation {
        SIN,
        COS,
        TAN,
    };

    class TrigFunctionCall : public PrologueFunctionCall {
    public:
        TrigFunctionCall(TrigOperation op, IProvider* provider, CallVector vector, const Type::Type* returnType):
            PrologueFunctionCall(provider, vector, returnType), _op(op) {}

        void execute() override;

        std::string toString() const override {
            return "NumberToStringFunctionCall<>";
        }

    protected:
        TrigOperation _op;
    };

    class TrigFunction : public PrologueFunction {
    public:
        TrigFunction(TrigOperation op, IProvider* provider) : PrologueFunction(provider), _op(op) {}

        FormalTypes paramTypes() const override;

        const Type::Type* returnType() const override;

        PrologueFunctionCall* call(CallVector) const override;

        std::string toString() const override {
            return "TrigFunction<>";
        }

    protected:
        TrigOperation _op;
    };
}

#endif //SWARMVM_TRIG
