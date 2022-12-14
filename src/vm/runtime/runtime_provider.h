#ifndef SWARMVM_RUNTIME_PROVIDER
#define SWARMVM_RUNTIME_PROVIDER

#include <utility>

#include "../../shared/nslib.h"
#include "interfaces.h"
#include "fabric.h"
#include "runtime_functions.h"

using namespace nslib;

/*
 * This file contains interface definitions for external code which provides
 * runtime resources (like native functions) to the VM. These plugins are called
 * providers.
 */

namespace swarmc::Runtime {

    class VirtualMachine;
    class IProvider;
    using Providers = std::vector<IProvider*>;

    /** An IFunctionCall returned by a provider which includes a reference to said provider. */
    class IProviderFunctionCall : public IFunctionCall {
    public:
        IProviderFunctionCall(CallVector vector, const Type::Type* returnType):
                IFunctionCall(FunctionBackend::FB_PROVIDER, std::move(vector), returnType) {}

        /** Get the IProvider responsible for this function call. */
        [[nodiscard]] virtual IProvider* provider() const = 0;

        /**
         * Execute the native function.
         * This should store the return value, if there is one, as an
         * `ISA::Reference*` using the `setReturn(...)` method.
         */
        virtual void execute(VirtualMachine*) = 0;
    };


    /** An IFunction returned by a provider which includes a reference to said provider. */
    class IProviderFunction : public IFunction {
    public:
        explicit IProviderFunction(std::string name) : _name(std::move(name)) {}

        /** Get the IProvider responsible for this function call. */
        [[nodiscard]] virtual IProvider* provider() const = 0;

        [[nodiscard]] FunctionBackend backend() const override { return FunctionBackend::FB_PROVIDER; }

        [[nodiscard]] IProviderFunctionCall* call(CallVector) const override = 0;

        [[nodiscard]] IProviderFunctionCall* call() const override { return call(getCallVector()); }

        [[nodiscard]] std::string name() const override { return _name; }

    protected:
        std::string _name;
    };


    /**
     * A plugin which provides external/native resources to the runtime VM.
     * At the moment, providers may add external functions.
     */
    class IProvider : public IStringable {
    public:
        ~IProvider() override = default;

        /**
         * Attempt to load a function backed by this provider by name.
         * If `name` is not the name of one of this provider's functions,
         * `nullptr` should be returned.
         *
         * e.g. if the VM is loading `call f:PROV_FN`, `name` will be `PROV_FN`
         *
         * @param name
         * @return
         */
        [[nodiscard]] virtual IProviderFunction* loadFunction(std::string name) = 0;

        /** Gives the provider access to the VM's IGlobalServices. */
        [[nodiscard]] virtual IGlobalServices* global() const = 0;

        /**
         * Execute a call to a function backed by this provider.
         * This is the entrypoint used by the VM to invoke functions
         * created by this provider.
         */
        virtual void call(VirtualMachine*, IProviderFunctionCall*) = 0;
    };

}

#endif //SWARMVM_RUNTIME_PROVIDER
