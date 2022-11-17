#ifndef SWARMVM_RUNTIME_FUNCTIONS
#define SWARMVM_RUNTIME_FUNCTIONS

#include <utility>
#include <vector>
#include "../../shared/IStringable.h"
#include "../../shared/util/Console.h"
#include "../../errors/SwarmError.h"

namespace swarmc::Type {
    class Type;
}

namespace swarmc::ISA {
    class Reference;
}

namespace swarmc::Runtime {

    using FormalTypes = std::vector<const Type::Type*>;
    using CallVector = std::vector<std::pair<const Type::Type*, ISA::Reference*>>;

    /** The VM mechanisms used to perform a function call. */
    enum class FunctionBackend {
        INLINE,  // jumps to an inline function body in the program
        PROVIDER,  // calls an external, native function implementation
    };


    /**
     * Represents a function call (which may be in-progress or completed).
     */
    class IFunctionCall : public IStringable {
    public:
        IFunctionCall(FunctionBackend backend, CallVector vector, const Type::Type* returnType):
            _backend(backend), _vector(std::move(vector)), _returnType(returnType) {}

        /** Determines how the function call is performed. */
        virtual FunctionBackend backend() { return _backend; }

        /** Get the parameters already applied to this function call, paired with thier types. */
        virtual CallVector vector() { return _vector; }

        /** Get the return type of this function call. */
        virtual const Type::Type* returnType() { return _returnType; }

        /** Set the value returned by the execution of this call. */
        virtual void setReturn(ISA::Reference* value) { _returnValue = value; }

        /** Get the value returned by the execution of this call. `nullptr` if the function has not returned. */
        virtual ISA::Reference* getReturn() const { return _returnValue; }

        /** Mark the function call as completed. */
        virtual void setReturned() { _returned = true; }

        /** Returns true if the function call has completed. */
        virtual bool hasReturned() const { return _returned; }

        /**
         * Get the next parameter curried with this function and advance the iterator.
         * This is meant to be used by whatever is executing the function call.
         */
        virtual std::pair<const Type::Type*, ISA::Reference*> popParam() {
            if ( !hasParamsRemaining() ) throw Errors::SwarmError("Cannot pop param from function call: index out of bounds");
            return _vector[_paramIndex++];
        }

        /** Returns true if the param iterator has not yet run out of curried params. */
        virtual bool hasParamsRemaining() {
            return _paramIndex < _vector.size();
        }

    protected:
        FunctionBackend _backend;
        CallVector _vector;
        const Type::Type* _returnType;
        ISA::Reference* _returnValue = nullptr;
        size_t _paramIndex = 0;
        bool _returned = false;
    };

    /**
     * Represents a function call which jumps to a function body defined inline in the SVI.
     */
    class InlineFunctionCall : public IFunctionCall {
    public:
        InlineFunctionCall(std::string name, CallVector vector, const Type::Type* returnType) :
            IFunctionCall(FunctionBackend::INLINE, std::move(vector), returnType), _name(std::move(name)) {}

        /**
         * Get the identifier name of the function.
         * e.g. if you want to jump to `beginfn f:MY_FN`, `name` is `MY_FN`.
         */
        std::string name() const { return _name; }

        std::string toString() const override {
            return "InlineFunctionCall<f:" + _name + ">";
        }

    protected:
        std::string _name;
    };


    /**
     * Represents a function which may be called by the runtime.
     */
    class IFunction : public IStringable {
    public:
        virtual ~IFunction() = default;

        /** Get a list of parameters (as types) which must be provided to a call of this function. */
        virtual FormalTypes paramTypes() const = 0;

        /** Get the return type of this function. */
        virtual const Type::Type* returnType() const = 0;

        /** Get a new IFunction which contains the given parameter, curried into a partial application. */
        virtual IFunction* curry(ISA::Reference*) const = 0;

        /** Get a list of the parameters which have been curried thusfar, along with their types. */
        virtual CallVector getCallVector() const = 0;

        /** Begin a function call with the given parameters. */
        virtual IFunctionCall* call(CallVector) const = 0;

        /** Begin a function call with the curried parameters. */
        virtual IFunctionCall* call() const { return call(getCallVector()); }

        /** Get the mechanism the VM should use to execute calls to this function. */
        virtual FunctionBackend backend() const = 0;
    };


    /**
     * A wrapper for other IFunction instances which curries a parameter to the function.
     */
    class CurriedFunction : public IFunction {
    public:
        CurriedFunction(ISA::Reference* ref, const IFunction* upstream) : _ref(ref), _upstream(upstream) {}

        FormalTypes paramTypes() const override {
            auto upstream = _upstream->paramTypes();
            return FormalTypes(upstream.begin() + 1, upstream.end());
        }

        const Type::Type* returnType() const override {
            return _upstream->returnType();
        }

        IFunction* curry(ISA::Reference* ref) const override {
            return new CurriedFunction(ref, this);
        }

        CallVector getCallVector() const override {
            // FIXME: validate type/param indices
            auto type = _upstream->paramTypes()[0];
            auto upstream = _upstream->getCallVector();
            upstream.push_back({type, _ref});
            return upstream;
        }

        IFunctionCall* call(CallVector vector) const override {
            return _upstream->call(vector);
        }

        FunctionBackend backend() const override {
            return _upstream->backend();
        }

        std::string toString() const override;

    protected:
        ISA::Reference* _ref;
        const IFunction* _upstream;
    };


    /**
     * A function which is defined inline in the SVI program.
     * These are referenced by name. e.g. if you want `beginfn f:MY_FN ...`, the name is `MY_FN`.
     */
    class InlineFunction : public IFunction {
    public:
        InlineFunction(std::string name, FormalTypes types, const Type::Type* returnType)
            : _name(name), _types(types), _returnType(returnType) {}

        FormalTypes paramTypes() const override {
            return _types;
        }

        const Type::Type* returnType() const override {
            return _returnType;
        };

        IFunction* curry(ISA::Reference* ref) const override {
            return new CurriedFunction(ref, this);
        }

        CallVector getCallVector() const override {
            return {};
        }

        IFunctionCall* call(CallVector vector) const override {
            return new InlineFunctionCall(_name, vector, _returnType);
        }

        FunctionBackend backend() const override {
            return FunctionBackend::INLINE;
        }

        std::string toString() const override;

    protected:
        std::string _name;
        FormalTypes _types;
        const Type::Type* _returnType;
    };

}

#endif //SWARMVM_RUNTIME_FUNCTIONS
