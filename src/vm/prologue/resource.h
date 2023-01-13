#ifndef SWARM_RESOURCE_H
#define SWARM_RESOURCE_H

#include "prologue_provider.h"
#include "../isa_meta.h"

namespace swarmc::Runtime::Prologue {

    class ResourceTFunctionCall : public PrologueFunctionCall {
    public:
        ResourceTFunctionCall(IProvider* provider, const CallVector& vector, Type::Type* returnType):
            PrologueFunctionCall(provider, "RESOURCE_T", vector, returnType) {}

        void execute(VirtualMachine*) override;

        [[nodiscard]] std::string toString() const override {
            return "ResourceTFunctionCall<>";
        }
    };

    class ResourceTFunction : public PrologueFunction {
    public:
        explicit ResourceTFunction(IProvider* provider) : PrologueFunction("RESOURCE_T", provider) {}

        [[nodiscard]] FormalTypes paramTypes() const override;

        [[nodiscard]] Type::Type* returnType() const override;

        [[nodiscard]] PrologueFunctionCall* call(CallVector) const override;

        [[nodiscard]] std::string toString() const override {
            return "ResourceTFunction<>";
        }
    };

}

#endif //SWARM_RESOURCE_H
