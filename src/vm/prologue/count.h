#ifndef SWARMVM_COUNT
#define SWARMVM_COUNT

#include <utility>

#include "prologue_provider.h"
#include "../isa_meta.h"

namespace swarmc::Runtime::Prologue {

    class CountFunctionCall : public PrologueFunctionCall {
    public:
        CountFunctionCall(IProvider* provider, const CallVector& vector, Type::Type* returnType):
            PrologueFunctionCall(provider, "COUNT", vector, returnType) {}

        void execute(VirtualMachine*) override;

        [[nodiscard]] std::string toString() const override {
            return "CountFunctionCall<>";
        }
    };

    class CountFunction : public PrologueFunction {
    public:
        explicit CountFunction(IProvider* provider) : PrologueFunction("COUNT", provider) {}

        [[nodiscard]] FormalTypes paramTypes() const override {
            return {
                new Type::Enumerable(Type::Ambiguous::of())
            };
        }

        [[nodiscard]] Type::Type* returnType() const override {
            return Type::Primitive::of(Type::Intrinsic::NUMBER);
        }

        [[nodiscard]] PrologueFunctionCall* call(CallVector) const override;

        [[nodiscard]] std::string toString() const override {
            return "CountFunction<>";
        }
    };

}

#endif //SWARMVM_COUNT
