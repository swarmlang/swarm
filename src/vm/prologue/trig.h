#ifndef SWARMVM_TRIG
#define SWARMVM_TRIG

#include <utility>

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
            PrologueFunctionCall(provider, std::move(vector), returnType), _op(op) {}

        void execute(VirtualMachine*) override;

        [[nodiscard]] std::string toString() const override {
            return "NumberToStringFunctionCall<>";
        }

    protected:
        TrigOperation _op;
    };

    class TrigFunction : public PrologueFunction {
    public:
        static std::string opToString(TrigOperation op) {
            if ( op == TrigOperation::SIN ) return "SIN";
            if ( op == TrigOperation::COS ) return "COS";
            if ( op == TrigOperation::TAN ) return "TAN";
            throw Errors::SwarmError("Unknown trig operation.");
        }

        TrigFunction(TrigOperation op, IProvider* provider) : PrologueFunction(opToString(op), provider), _op(op) {}

        [[nodiscard]] FormalTypes paramTypes() const override;

        [[nodiscard]] const Type::Type* returnType() const override;

        [[nodiscard]] PrologueFunctionCall* call(CallVector) const override;

        [[nodiscard]] std::string toString() const override {
            return "TrigFunction<>";
        }

    protected:
        TrigOperation _op;
    };
}

#endif //SWARMVM_TRIG
