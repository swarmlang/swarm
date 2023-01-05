#ifndef SWARMVM_TO_STRING
#define SWARMVM_TO_STRING

#include <utility>

#include "prologue_provider.h"

namespace swarmc::Runtime::Prologue {

    class NumberToStringFunctionCall : public PrologueFunctionCall {
    public:
        NumberToStringFunctionCall(IProvider* provider, const CallVector& vector, const Type::Type* returnType):
            PrologueFunctionCall(provider, "NUMBER_TO_STRING", vector, returnType) {}

        void execute(VirtualMachine*) override;

        [[nodiscard]] std::string toString() const override {
            return "NumberToStringFunctionCall<>";
        }
    };

    class NumberToStringFunction : public PrologueFunction {
    public:
        explicit NumberToStringFunction(IProvider* provider) : PrologueFunction("NUMBER_TO_STRING", provider) {}

        [[nodiscard]] FormalTypes paramTypes() const override;

        [[nodiscard]] const Type::Type* returnType() const override;

        [[nodiscard]] PrologueFunctionCall* call(CallVector) const override;

        [[nodiscard]] std::string toString() const override {
            return "NumberToStringFunction<>";
        }
    };

    class BooleanToStringFunctionCall : public PrologueFunctionCall {
    public:
        BooleanToStringFunctionCall(IProvider* provider, const CallVector& vector, const Type::Type* returnType):
            PrologueFunctionCall(provider, "BOOLEAN_TO_STRING", vector, returnType) {}

        void execute(VirtualMachine*) override;

        [[nodiscard]] std::string toString() const override {
            return "BooleanToStringFunctionCall<>";
        }
    };

    class BooleanToStringFunction : public PrologueFunction {
    public:
        explicit BooleanToStringFunction(IProvider* provider) : PrologueFunction("BOOLEAN_TO_STRING", provider) {}

        [[nodiscard]] FormalTypes paramTypes() const override;

        [[nodiscard]] const Type::Type* returnType() const override;

        [[nodiscard]] PrologueFunctionCall* call(CallVector) const override;

        [[nodiscard]] std::string toString() const override {
            return "BooleanToStringFunction<>";
        }
    };

}

#endif //SWARMVM_TO_STRING
