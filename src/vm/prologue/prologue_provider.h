#ifndef SWARMVM_PROLOGUE_PROVIDER
#define SWARMVM_PROLOGUE_PROVIDER

#include "../runtime/runtime_provider.h"
#include "../runtime/runtime_functions.h"

namespace swarmc::Runtime::Prologue {

    class PrologueFunctionCall : public IProviderFunctionCall {
    public:
        PrologueFunctionCall(IProvider* provider, CallVector vector, const Type::Type* returnType) :
                IProviderFunctionCall(vector, returnType), _provider(provider) {}

        IProvider* provider() const { return _provider; }
    protected:
        IProvider* _provider;
    };

    class PrologueFunction : public IProviderFunction {
    public:
        PrologueFunction(IProvider* provider) : _provider(provider) {}

        IProvider* provider() const override { return _provider; }

        PrologueFunctionCall* call(CallVector) const override = 0;

        PrologueFunctionCall* call() const override { return call(getCallVector()); }

        CallVector getCallVector() const override {
            return {};
        }

        IFunction* curry(ISA::Reference* ref) const override {
            return new CurriedFunction(ref, this);
        }

    protected:
        IProvider* _provider;
    };

    class Provider : public IProvider {
        PrologueFunction* loadFunction(std::string name) override;

        void call(IProviderFunctionCall* call) override;

        std::string toString() const override {
            return "Prologue::Provider<>";
        }
    };

}

#endif //SWARMVM_PROLOGUE_PROVIDER
