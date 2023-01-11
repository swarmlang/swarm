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
        PrologueFunctionCall(IProvider* provider, std::string name, const CallVector& vector, const Type::Type* returnType) :
            IProviderFunctionCall(vector, std::move(name), returnType), _provider(useref(provider)) {}

        ~PrologueFunctionCall() override {
            freeref(_provider);
        }

        [[nodiscard]] IProvider* provider() const override { return _provider; }
    protected:
        IProvider* _provider;
    };

    class PrologueFunction : public IProviderFunction {
    public:
        PrologueFunction(std::string name, IProvider* provider) : IProviderFunction(std::move(name)), _provider(useref(provider)) {}

        ~PrologueFunction() override {
            freeref(_provider);
        }

        [[nodiscard]] IProvider* provider() const override { return _provider; }

        [[nodiscard]] PrologueFunctionCall* call(CallVector) const override = 0;

        [[nodiscard]] PrologueFunctionCall* call() const override { return call(getCallVector()); }

        [[nodiscard]] CallVector getCallVector() const override {
            return {};
        }

        IFunction* curry(ISA::Reference* ref) override {
            return new CurriedFunction(ref, this);
        }

    protected:
        IProvider* _provider;
    };

    class Provider : public IProvider {
    public:
        explicit Provider(IGlobalServices* global) : _global(useref(global)) {}

        ~Provider() override {
            freeref(_global);
        }

        PrologueFunction* loadFunction(std::string name) override;

        void call(VirtualMachine* vm, IProviderFunctionCall* call) override;

        [[nodiscard]] IGlobalServices* global() const override {
            return _global;
        }

        [[nodiscard]] std::string toString() const override {
            return "Prologue::Provider<>";
        }

    protected:
        IGlobalServices* _global;
    };

}

#endif //SWARMVM_PROLOGUE_PROVIDER
