#ifndef SWARMVM_TO_STRING
#define SWARMVM_TO_STRING

#include <utility>

#include "prologue_provider.h"
#include "../isa_meta.h"

namespace swarmc::Runtime::Prologue {

    class NumberToStringFunctionCall : public PrologueFunctionCall {
    public:
        NumberToStringFunctionCall(IProvider* provider, const CallVector& vector, Type::Type* returnType):
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

        [[nodiscard]] Type::Type* returnType() const override;

        [[nodiscard]] PrologueFunctionCall* call(CallVector) const override;

        [[nodiscard]] std::string toString() const override {
            return "NumberToStringFunction<>";
        }
    };

    class BooleanToStringFunctionCall : public PrologueFunctionCall {
    public:
        BooleanToStringFunctionCall(IProvider* provider, const CallVector& vector, Type::Type* returnType):
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

        [[nodiscard]] Type::Type* returnType() const override;

        [[nodiscard]] PrologueFunctionCall* call(CallVector) const override;

        [[nodiscard]] std::string toString() const override {
            return "BooleanToStringFunction<>";
        }
    };

    class VectorToStringFunctionCall : public PrologueFunctionCall {
    public:
        VectorToStringFunctionCall(IProvider* provider, const CallVector& vector, Type::Type* returnType):
            PrologueFunctionCall(provider, "VECTOR_TO_STRING", vector, returnType) {}

        void execute(VirtualMachine*) override;

        [[nodiscard]] std::string toString() const override {
            return "VectorToStringFunctionCall<>";
        }
    };

    class VectorToStringFunction : public PrologueFunction {
    public:
        explicit VectorToStringFunction(IProvider* provider) : PrologueFunction("VECTOR_TO_STRING", provider) {}

        [[nodiscard]] FormalTypes paramTypes() const override {
            return {new Type::Enumerable(Type::Primitive::of(Type::Intrinsic::NUMBER))};
        }

        [[nodiscard]] Type::Type* returnType() const override {
            return Type::Primitive::of(Type::Intrinsic::STRING);
        }

        [[nodiscard]] PrologueFunctionCall* call(CallVector) const override;

        [[nodiscard]] std::string toString() const override {
            return "VectorToStringFunction<>";
        }
    };

    class MatrixToStringFunctionCall : public PrologueFunctionCall {
    public:
        MatrixToStringFunctionCall(IProvider* provider, const CallVector& vector, Type::Type* returnType):
            PrologueFunctionCall(provider, "MATRIX_TO_STRING", vector, returnType) {}

        void execute(VirtualMachine*) override;

        [[nodiscard]] std::string toString() const override {
            return "MatrixToStringFunctionCall<>";
        }
    };

    class MatrixToStringFunction : public PrologueFunction {
    public:
        explicit MatrixToStringFunction(IProvider* provider) : PrologueFunction("MATRIX_TO_STRING", provider) {}

        [[nodiscard]] FormalTypes paramTypes() const override {
            return {new Type::Enumerable(new Type::Enumerable(Type::Primitive::of(Type::Intrinsic::NUMBER)))};
        }

        [[nodiscard]] Type::Type* returnType() const override {
            return Type::Primitive::of(Type::Intrinsic::STRING);
        }

        [[nodiscard]] PrologueFunctionCall* call(CallVector) const override;

        [[nodiscard]] std::string toString() const override {
            return "MatrixToStringFunction<>";
        }
    };

}

#endif //SWARMVM_TO_STRING
