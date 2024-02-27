#ifndef SWARM_TYPE_CONSTRUCTION_H
#define SWARM_TYPE_CONSTRUCTION_H

#include <utility>
#include "prologue_provider.h"
#include "../../lang/Type.h"

namespace swarmc::Runtime::Prologue {

    class Lambda0FunctionCall : public PrologueFunctionCall {
    public:
        Lambda0FunctionCall(IProvider* provider, const CallVector& vector, Type::Type* returnType):
                PrologueFunctionCall(provider, "LAMBDA0_T", vector, returnType) {}

        void execute(VirtualMachine*) override;

        [[nodiscard]] std::string toString() const override {
            return "Lambda0FunctionCall<>";
        }
    };

    class Lambda0Function : public PrologueFunction {
    public:
        explicit Lambda0Function(IProvider* provider) : PrologueFunction("LAMBDA0_T", provider) {}

        [[nodiscard]] FormalTypes paramTypes() const override;

        [[nodiscard]] Type::Type* returnType() const override;

        [[nodiscard]] PrologueFunctionCall* call(CallVector) const override;

        [[nodiscard]] std::string toString() const override {
            return "Lambda0Function<>";
        }
    };

    class Lambda1FunctionCall : public PrologueFunctionCall {
    public:
        Lambda1FunctionCall(IProvider* provider, const CallVector& vector, Type::Type* returnType):
                PrologueFunctionCall(provider, "LAMBDA1_T", vector, returnType) {}

        void execute(VirtualMachine*) override;

        [[nodiscard]] std::string toString() const override {
            return "Lambda1FunctionCall<>";
        }
    };

    class Lambda1Function : public PrologueFunction {
    public:
        explicit Lambda1Function(IProvider* provider) : PrologueFunction("LAMBDA1_T", provider) {}

        [[nodiscard]] FormalTypes paramTypes() const override;

        [[nodiscard]] Type::Type* returnType() const override;

        [[nodiscard]] PrologueFunctionCall* call(CallVector) const override;

        [[nodiscard]] std::string toString() const override {
            return "Lambda1Function<>";
        }
    };

    inline Type::Opaque* contextIdType() {
        return Type::Opaque::of("CONTEXT_ID");
    }

    class ContextIdFunctionCall : public PrologueFunctionCall {
    public:
        ContextIdFunctionCall(IProvider* provider, const CallVector& vector, Type::Type* returnType):
            PrologueFunctionCall(provider, "CONTEXT_ID_T", vector, returnType) {}

        void execute(VirtualMachine*) override;

        [[nodiscard]] std::string toString() const override {
            return "ContextIdFunctionCall<>";
        }
    };

    class ContextIdFunction : public PrologueFunction {
    public:
        explicit ContextIdFunction(IProvider* provider) : PrologueFunction("CONTEXT_ID_T", provider) {}

        [[nodiscard]] FormalTypes paramTypes() const override { return {}; }

        [[nodiscard]] Type::Type* returnType() const override;

        [[nodiscard]] PrologueFunctionCall* call(CallVector) const override;

        [[nodiscard]] std::string toString() const override {
            return "ContextIdFunction<>";
        }
    };

}

#endif //SWARM_TYPE_CONSTRUCTION_H
