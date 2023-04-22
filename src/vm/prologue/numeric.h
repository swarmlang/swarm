#ifndef SWARM_NUMERIC
#define SWARM_NUMERIC

#include "prologue_provider.h"
#include "../../lang/Type.h"

namespace swarmc::Runtime::Prologue {
    // sqrt, floor, ceil

    class FloorFunctionCall : public PrologueFunctionCall {
    public:
        FloorFunctionCall(IProvider* provider, const CallVector& vector, Type::Type* returnType) :
                PrologueFunctionCall(provider, "FLOOR", vector, returnType) {}

        void execute(VirtualMachine*) override;

        [[nodiscard]] std::string toString() const override {
            return "FloorFunctionCall<>";
        }
    };

    class FloorFunction : public PrologueFunction {
    public:
        explicit FloorFunction(IProvider* provider) : PrologueFunction("FLOOR", provider) {}

        [[nodiscard]] FormalTypes paramTypes() const override {
            return {Type::Primitive::of(Type::Intrinsic::NUMBER)};
        }

        [[nodiscard]] Type::Type* returnType() const override {
            return Type::Primitive::of(Type::Intrinsic::NUMBER);
        }

        [[nodiscard]] PrologueFunctionCall* call(CallVector) const override;

        [[nodiscard]] std::string toString() const override {
            return "FloorFunction<>";
        }
    };

    class CeilingFunctionCall : public PrologueFunctionCall {
    public:
        CeilingFunctionCall(IProvider* provider, const CallVector& vector, Type::Type* returnType) :
                PrologueFunctionCall(provider, "CEILING", vector, returnType) {}

        void execute(VirtualMachine*) override;

        [[nodiscard]] std::string toString() const override {
            return "CeilingFunctionCall<>";
        }
    };

    class CeilingFunction : public PrologueFunction {
    public:
        explicit CeilingFunction(IProvider* provider) : PrologueFunction("CEILING", provider) {}

        [[nodiscard]] FormalTypes paramTypes() const override {
            return {Type::Primitive::of(Type::Intrinsic::NUMBER)};
        }

        [[nodiscard]] Type::Type* returnType() const override {
            return Type::Primitive::of(Type::Intrinsic::NUMBER);
        }

        [[nodiscard]] PrologueFunctionCall* call(CallVector) const override;

        [[nodiscard]] std::string toString() const override {
            return "CeilingFunction<>";
        }
    };

    class NthRootFunctionCall : public PrologueFunctionCall {
    public:
        NthRootFunctionCall(IProvider* provider, const CallVector& vector, Type::Type* returnType) :
                PrologueFunctionCall(provider, "NTH_ROOT", vector, returnType) {}

        void execute(VirtualMachine*) override;

        [[nodiscard]] std::string toString() const override {
            return "NthRootFunctionCall<>";
        }
    };

    class NthRootFunction : public PrologueFunction {
    public:
        explicit NthRootFunction(IProvider* provider) : PrologueFunction("NTH_ROOT", provider) {}

        [[nodiscard]] FormalTypes paramTypes() const override {
            return {
                Type::Primitive::of(Type::Intrinsic::NUMBER),
                Type::Primitive::of(Type::Intrinsic::NUMBER)
            };
        }

        [[nodiscard]] Type::Type* returnType() const override {
            return Type::Primitive::of(Type::Intrinsic::NUMBER);
        }

        [[nodiscard]] PrologueFunctionCall* call(CallVector) const override;

        [[nodiscard]] std::string toString() const override {
            return "NthRootFunction<>";
        }
    };

}

#endif
