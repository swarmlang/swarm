#ifndef SWARMVM_RAND
#define SWARMVM_RAND

#include <utility>

#include "prologue_provider.h"
#include "../../lang/Type.h"

namespace swarmc::Runtime::Prologue {

    class RandomFunctionCall : public PrologueFunctionCall {
    public:
        RandomFunctionCall(IProvider* provider, const CallVector& vector, const Type::Type* returnType):
            PrologueFunctionCall(provider, "RANDOM", vector, returnType) {}

        void execute(VirtualMachine*) override;

        [[nodiscard]] std::string toString() const override {
            return "RandomFunctionCall<>";
        }
    };

    class RandomFunction : public PrologueFunction {
    public:
        explicit RandomFunction(IProvider* provider) : PrologueFunction("RANDOM", provider) {}

        [[nodiscard]] FormalTypes paramTypes() const override { return {}; }

        [[nodiscard]] const Type::Type* returnType() const override;

        [[nodiscard]] PrologueFunctionCall* call(CallVector) const override;

        [[nodiscard]] std::string toString() const override {
            return "RandomFunction<>";
        }
    };

    class RandomVectorFunctionCall : public PrologueFunctionCall {
    public:
        RandomVectorFunctionCall(IProvider* provider, const CallVector& vector, const Type::Type* returnType):
            PrologueFunctionCall(provider, "RANDOM_VECTOR", vector, returnType) {}

        void execute(VirtualMachine*) override;

        [[nodiscard]] std::string toString() const override {
            return "RandomVectorFunctionCall<>";
        }
    };

    class RandomVectorFunction : public PrologueFunction {
    public:
        explicit RandomVectorFunction(IProvider* provider) : PrologueFunction("RANDOM_VECTOR", provider) {}

        [[nodiscard]] FormalTypes paramTypes() const override;

        [[nodiscard]] const Type::Type* returnType() const override;

        [[nodiscard]] PrologueFunctionCall* call(CallVector) const override;

        [[nodiscard]] std::string toString() const override {
            return "RandomVectorFunction<>";
        }
    };

    class RandomMatrixFunctionCall : public PrologueFunctionCall {
    public:
        RandomMatrixFunctionCall(IProvider* provider, const CallVector& vector, const Type::Type* returnType):
            PrologueFunctionCall(provider, "RANDOM_MATRIX", vector, returnType) {}

        void execute(VirtualMachine*) override;

        [[nodiscard]] std::string toString() const override {
            return "RandomMatrixFunctionCall<>";
        }
    };

    class RandomMatrixFunction : public PrologueFunction {
    public:
        explicit RandomMatrixFunction(IProvider* provider) : PrologueFunction("RANDOM_MATRIX", provider) {}

        [[nodiscard]] FormalTypes paramTypes() const override;

        [[nodiscard]] const Type::Type* returnType() const override;

        [[nodiscard]] PrologueFunctionCall* call(CallVector) const override;

        [[nodiscard]] std::string toString() const override {
            return "RandomMatrixFunction<>";
        }
    };

}

#endif //SWARMVM_RAND
