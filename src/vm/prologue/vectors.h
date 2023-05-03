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

    class SubVectorFunctionCall : public PrologueFunctionCall {
    public:
        SubVectorFunctionCall(IProvider* provider, const CallVector& vector, Type::Type* returnType) :
            PrologueFunctionCall(provider, "SUBVECTOR", vector, returnType) {}

        void execute(VirtualMachine*) override;

        [[nodiscard]] std::string toString() const override {
            return "SubVectorFunctionCall<>";
        }
    };

    class SubVectorFunction : public PrologueFunction {
    public:
        explicit SubVectorFunction(IProvider* provider) : PrologueFunction("SUBVECTOR", provider) {}

        [[nodiscard]] FormalTypes paramTypes() const override {
            return {
                Type::Primitive::of(Type::Intrinsic::NUMBER),  // startAt
                Type::Primitive::of(Type::Intrinsic::NUMBER),  // length
                new Type::Enumerable(Type::Primitive::of(Type::Intrinsic::NUMBER)),  // vector
            };
        }

        [[nodiscard]] Type::Type* returnType() const override {
            return new Type::Enumerable(Type::Primitive::of(Type::Intrinsic::NUMBER));
        }

        [[nodiscard]] PrologueFunctionCall* call(CallVector) const override;

        [[nodiscard]] std::string toString() const override {
            return "SubVectorFunction<>";
        }
    };

    class SubMatrixFunctionCall : public PrologueFunctionCall {
    public:
        SubMatrixFunctionCall(IProvider* provider, const CallVector& vector, Type::Type* returnType) :
            PrologueFunctionCall(provider, "SUBMATRIX", vector, returnType) {}

        void execute(VirtualMachine*) override;

        [[nodiscard]] std::string toString() const override {
            return "SubMatrixFunctionCall<>";
        }
    };

    class SubMatrixFunction : public PrologueFunction {
    public:
        explicit SubMatrixFunction(IProvider* provider) : PrologueFunction("SUBMATRIX", provider) {}

        [[nodiscard]] FormalTypes paramTypes() const override {
            return {
                Type::Primitive::of(Type::Intrinsic::NUMBER),  // x0
                Type::Primitive::of(Type::Intrinsic::NUMBER),  // x1
                Type::Primitive::of(Type::Intrinsic::NUMBER),  // y0
                Type::Primitive::of(Type::Intrinsic::NUMBER),  // y1
                new Type::Enumerable(new Type::Enumerable(Type::Primitive::of(Type::Intrinsic::NUMBER))),  // matrix
            };
        }

        [[nodiscard]] Type::Type* returnType() const override {
            return new Type::Enumerable(new Type::Enumerable(Type::Primitive::of(Type::Intrinsic::NUMBER)));
        }

        [[nodiscard]] PrologueFunctionCall* call(CallVector) const override;

        [[nodiscard]] std::string toString() const override {
            return "SubMatrixFunction<>";
        }
    };

}

#endif //SWARMVM_VECTORS
