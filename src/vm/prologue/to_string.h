#ifndef SWARMVM_TO_STRING
#define SWARMVM_TO_STRING

#include "prologue_provider.h"

namespace swarmc::Runtime::Prologue {

    class NumberToStringFunctionCall : public PrologueFunctionCall {
    public:
        NumberToStringFunctionCall(IProvider* provider, CallVector vector, const Type::Type* returnType):
            PrologueFunctionCall(provider, vector, returnType) {}

        void execute() override;

        std::string toString() const override {
            return "NumberToStringFunctionCall<>";
        }
    };

    class NumberToStringFunction : public PrologueFunction {
    public:
        NumberToStringFunction(IProvider* provider) : PrologueFunction("NUMBER_TO_STRING", provider) {}

        FormalTypes paramTypes() const override;

        const Type::Type* returnType() const override;

        PrologueFunctionCall* call(CallVector) const override;

        std::string toString() const override {
            return "NumberToStringFunction<>";
        }
    };

    class BooleanToStringFunctionCall : public PrologueFunctionCall {
    public:
        BooleanToStringFunctionCall(IProvider* provider, CallVector vector, const Type::Type* returnType):
            PrologueFunctionCall(provider, vector, returnType) {}

        void execute() override;

        std::string toString() const override {
            return "BooleanToStringFunctionCall<>";
        }
    };

    class BooleanToStringFunction : public PrologueFunction {
    public:
        BooleanToStringFunction(IProvider* provider) : PrologueFunction("BOOLEAN_TO_STRING", provider) {}

        FormalTypes paramTypes() const override;

        const Type::Type* returnType() const override;

        PrologueFunctionCall* call(CallVector) const override;

        std::string toString() const override {
            return "BooleanToStringFunction<>";
        }
    };

}

#endif //SWARMVM_TO_STRING
