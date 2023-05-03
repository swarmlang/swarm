#ifndef SWARMVM_VECTORS
#define SWARMVM_VECTORS

#include "prologue_provider.h"
#include "../../lang/Type.h"

namespace swarmc::Runtime::Prologue {

    class ZeroVectorFunctionCall : public PrologueFunctionCall {
    public:
        ZeroVectorFunctionCall(IProvider* provider, const CallVector& vector, Type::Type* returnType) :
                PrologueFunctionCall(provider, "ZERO_VECTOR", vector, returnType) {}

        void execute(VirtualMachine*) override;

        [[nodiscard]] std::string toString() const override {
            return "ZeroVectorFunctionCall<>";
        }
    };

    class ZeroVectorFunction : public PrologueFunction {
    public:
        explicit ZeroVectorFunction(IProvider* provider) : PrologueFunction("ZERO_VECTOR", provider) {}

        [[nodiscard]] FormalTypes paramTypes() const override {
            return {Type::Primitive::of(Type::Intrinsic::NUMBER)};
        }

        [[nodiscard]] Type::Type* returnType() const override {
            return new Type::Enumerable(Type::Primitive::of(Type::Intrinsic::NUMBER));
        }

        [[nodiscard]] PrologueFunctionCall* call(CallVector) const override;

        [[nodiscard]] std::string toString() const override {
            return "ZeroVectorFunction<>";
        }
    };


    class ZeroMatrixFunctionCall : public PrologueFunctionCall {
    public:
        ZeroMatrixFunctionCall(IProvider* provider, const CallVector& vector, Type::Type* returnType) :
                PrologueFunctionCall(provider, "ZERO_MATRIX", vector, returnType) {}

        void execute(VirtualMachine*) override;

        [[nodiscard]] std::string toString() const override {
            return "ZeroMatrixFunctionCall<>";
        }
    };

    class ZeroMatrixFunction : public PrologueFunction {
    public:
        explicit ZeroMatrixFunction(IProvider* provider) : PrologueFunction("ZERO_MATRIX", provider) {}

        [[nodiscard]] FormalTypes paramTypes() const override {
            return {Type::Primitive::of(Type::Intrinsic::NUMBER), Type::Primitive::of(Type::Intrinsic::NUMBER)};
        }

        [[nodiscard]] Type::Type* returnType() const override {
            return new Type::Enumerable(new Type::Enumerable(Type::Primitive::of(Type::Intrinsic::NUMBER)));
        }

        [[nodiscard]] PrologueFunctionCall* call(CallVector) const override;

        [[nodiscard]] std::string toString() const override {
            return "ZeroMatrixFunction<>";
        }
    };

}

#endif //SWARMVM_VECTORS
