#ifndef SWARMVM_RAND
#define SWARMVM_RAND

#include "prologue_provider.h"
#include "../../lang/Type.h"

namespace swarmc::Runtime::Prologue {

    class RandomFunctionCall : public PrologueFunctionCall {
    public:
        RandomFunctionCall(IProvider* provider, CallVector vector, const Type::Type* returnType):
            PrologueFunctionCall(provider, vector, returnType) {}

        void execute() override;

        std::string toString() const override {
            return "RandomFunctionCall<>";
        }
    };

    class RandomFunction : public PrologueFunction {
    public:
        RandomFunction(IProvider* provider) : PrologueFunction(provider) {}

        FormalTypes paramTypes() const override { return {}; }

        const Type::Type* returnType() const override;

        PrologueFunctionCall* call(CallVector) const override;

        std::string toString() const override {
            return "RandomFunction<>";
        }
    };

    class RandomVectorFunctionCall : public PrologueFunctionCall {
    public:
        RandomVectorFunctionCall(IProvider* provider, CallVector vector, const Type::Type* returnType):
            PrologueFunctionCall(provider, vector, returnType) {}

        void execute() override;

        std::string toString() const override {
            return "RandomVectorFunctionCall<>";
        }
    };

    class RandomVectorFunction : public PrologueFunction {
    public:
        RandomVectorFunction(IProvider* provider) : PrologueFunction(provider) {}

        FormalTypes paramTypes() const override;

        const Type::Type* returnType() const override;

        PrologueFunctionCall* call(CallVector) const override;

        std::string toString() const override {
            return "RandomVectorFunction<>";
        }
    };

    class RandomMatrixFunctionCall : public PrologueFunctionCall {
    public:
        RandomMatrixFunctionCall(IProvider* provider, CallVector vector, const Type::Type* returnType):
            PrologueFunctionCall(provider, vector, returnType) {}

        void execute() override;

        std::string toString() const override {
            return "RandomMatrixFunctionCall<>";
        }
    };

    class RandomMatrixFunction : public PrologueFunction {
    public:
        RandomMatrixFunction(IProvider* provider) : PrologueFunction(provider) {}

        FormalTypes paramTypes() const override;

        const Type::Type* returnType() const override;

        PrologueFunctionCall* call(CallVector) const override;

        std::string toString() const override {
            return "RandomMatrixFunction<>";
        }
    };

}

#endif //SWARMVM_RAND
