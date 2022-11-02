#ifndef SWARMVM_RUNTIME_PROVIDER
#define SWARMVM_RUNTIME_PROVIDER

#include "../../shared/IStringable.h"
#include "interfaces.h"
#include "runtime_functions.h"

namespace swarmc::Runtime {

    class IProvider;


    class IProviderFunctionCall : public IFunctionCall {
    public:
        IProviderFunctionCall(CallVector vector, const Type::Type* returnType):
                IFunctionCall(FunctionBackend::PROVIDER, vector, returnType) {}

        virtual IProvider* provider() const = 0;
    };


    class IProviderFunction : public IFunction {
    public:
        virtual IProvider* provider() const = 0;

        FunctionBackend backend() const override { return FunctionBackend::PROVIDER; }

        IProviderFunctionCall* call(CallVector) const override = 0;

        IProviderFunctionCall* call() const override { return call(getCallVector()); }
    };


    class IProvider : public IStringable {
    public:
        virtual ~IProvider() = default;

        virtual IProviderFunction* loadFunction(std::string name) = 0;

        virtual void call(IProviderFunctionCall* call) = 0;
    };

}

#endif //SWARMVM_RUNTIME_PROVIDER
