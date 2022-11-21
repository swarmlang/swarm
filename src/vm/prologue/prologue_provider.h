#ifndef SWARMVM_PROLOGUE_PROVIDER
#define SWARMVM_PROLOGUE_PROVIDER

#include <utility>

#include "../runtime/runtime_provider.h"
#include "../runtime/runtime_functions.h"

/*
 * This file contains implementations of the modular runtime provider interfaces
 * which provide Swarm's Prologue standard library.
 */

namespace swarmc::Runtime::Prologue {

    class PrologueFunctionCall : public IProviderFunctionCall {
    public:
        PrologueFunctionCall(IProvider* provider, CallVector vector, const Type::Type* returnType) :
            IProviderFunctionCall(std::move(vector), returnType), _provider(provider) {}

        IProvider* provider() const override { return _provider; }
    protected:
        IProvider* _provider;
    };

    class PrologueFunction : public IProviderFunction {
    public:
        PrologueFunction(std::string name, IProvider* provider) : IProviderFunction(std::move(name)), _provider(provider) {}

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
    public:
        Provider(IGlobalServices* global) : _global(global) {}

        PrologueFunction* loadFunction(std::string name) override;

        void call(IProviderFunctionCall* call) override;

        IGlobalServices* global() const override {
            return _global;
        }

        std::string toString() const override {
            return "Prologue::Provider<>";
        }

    protected:
        IGlobalServices* _global;
    };

}

#endif //SWARMVM_PROLOGUE_PROVIDER
