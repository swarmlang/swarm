#ifndef SWARMVM_RUNTIME_FUNCTIONS
#define SWARMVM_RUNTIME_FUNCTIONS

#include <utility>
#include <vector>
#include "../../shared/nslib.h"
#include "../../errors/SwarmError.h"

using namespace nslib;

namespace swarmc::Type {
    class Type;
}

namespace swarmc::ISA {
    class Reference;
}

namespace swarmc::Runtime {

    using FormalTypes = std::vector<const Type::Type *>;
    using CallVector = std::vector<std::pair<const Type::Type *, ISA::Reference *>>;

    /** The VM mechanisms used to perform a function call. */
    enum class FunctionBackend : std::size_t {
        FB_INLINE,  // jumps to an inline function body in the program
        FB_PROVIDER,  // calls an external, native function implementation
    };

}

namespace nslib {
    [[nodiscard]] std::string s(swarmc::Runtime::FunctionBackend v);
}

namespace swarmc::Runtime {

    /**
     * Represents a function call (which may be in-progress or completed).
     */
    class IFunctionCall : public IStringable, public serial::ISerializable, public IRefCountable {
    public:
        IFunctionCall(FunctionBackend backend, std::string name, CallVector vector, const Type::Type* returnType):
            _backend(backend), _name(std::move(name)), _vector(std::move(vector)), _returnType(returnType) {}

        /** Determines how the function call is performed. */
        [[nodiscard]] virtual FunctionBackend backend() const { return _backend; }

        /** Get the name of the function. Used to load the call from serialization. */
        [[nodiscard]] virtual std::string name() const { return _name; }

        [[nodiscard]] serial::tag_t getSerialKey() const override { return s(_backend); }

        /** Get the parameters already applied to this function call, paired with thier types. */
        virtual CallVector vector() const { return _vector; }

        /** Get the return type of this function call. */
        virtual const Type::Type* returnType() const { return _returnType; }

        /** Set the value returned by the execution of this call. */
        virtual void setReturn(ISA::Reference* value) { _returnValue = value; }

        /** Get the value returned by the execution of this call. `nullptr` if the function has not returned. */
        [[nodiscard]] virtual ISA::Reference* getReturn() const { return _returnValue; }

        /** Mark the function call as completed. */
        virtual void setReturned() { _returned = true; }

        /** Returns true if the function call has completed. */
        [[nodiscard]] virtual bool hasReturned() const { return _returned; }

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
        std::string _name;
        CallVector _vector;
        const Type::Type* _returnType;
        ISA::Reference* _returnValue = nullptr;
        std::size_t _paramIndex = 0;
        bool _returned = false;
    };

    /**
     * Represents a function call which jumps to a function body defined inline in the SVI.
     */
    class InlineFunctionCall : public IFunctionCall {
    public:
        InlineFunctionCall(std::string name, CallVector vector, const Type::Type* returnType) :
                IFunctionCall(FunctionBackend::FB_INLINE, std::move(name), std::move(vector), returnType) {}

        [[nodiscard]] std::string toString() const override {
            return "InlineFunctionCall<f:" + _name + ">";
        }
    };


    /**
     * Represents a function which may be called by the runtime.
     */
    class IFunction : public IStringable, public IRefCountable {
    public:
        ~IFunction() override = default;

        /** Get a list of parameters (as types) which must be provided to a call of this function. */
        [[nodiscard]] virtual FormalTypes paramTypes() const = 0;

        /** Get the return type of this function. */
        [[nodiscard]] virtual const Type::Type* returnType() const = 0;

        /** Get a new IFunction which contains the given parameter, curried into a partial application. */
        [[nodiscard]] virtual IFunction* curry(ISA::Reference*) const = 0;

        /** Get a list of the parameters which have been curried thusfar, along with their types. */
        [[nodiscard]] virtual CallVector getCallVector() const = 0;

        /** Begin a function call with the given parameters. */
        [[nodiscard]] virtual IFunctionCall* call(CallVector) const = 0;

        /** Begin a function call with the curried parameters. */
        [[nodiscard]] virtual IFunctionCall* call() const { return call(getCallVector()); }

        /** Get the mechanism the VM should use to execute calls to this function. */
        [[nodiscard]] virtual FunctionBackend backend() const = 0;

        /** Get the name of the function to be looked up from the backend. */
        [[nodiscard]] virtual std::string name() const = 0;
    };


    /**
     * A wrapper for other IFunction instances which curries a parameter to the function.
     */
    class CurriedFunction : public IFunction {
    public:
        CurriedFunction(ISA::Reference* ref, const IFunction* upstream) : _ref(ref), _upstream(upstream) {}

        [[nodiscard]] FormalTypes paramTypes() const override {
            auto upstream = _upstream->paramTypes();
            return {upstream.begin() + 1, upstream.end()};
        }

        [[nodiscard]] const Type::Type* returnType() const override {
            return _upstream->returnType();
        }

        IFunction* curry(ISA::Reference* ref) const override {
            return new CurriedFunction(ref, this);
        }

        [[nodiscard]] CallVector getCallVector() const override {
            // FIXME: validate type/param indices
            auto type = _upstream->paramTypes()[0];
            auto upstream = _upstream->getCallVector();
            upstream.push_back({type, _ref});
            return upstream;
        }

        [[nodiscard]] IFunctionCall* call(CallVector vector) const override {
            return _upstream->call(vector);
        }

        [[nodiscard]] FunctionBackend backend() const override {
            return _upstream->backend();
        }

        [[nodiscard]] std::string name() const override {
            return _upstream->name();
        }

        [[nodiscard]] std::string toString() const override;

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
            : _name(std::move(name)), _types(std::move(types)), _returnType(returnType) {}

        [[nodiscard]] FormalTypes paramTypes() const override {
            return _types;
        }

        [[nodiscard]] const Type::Type* returnType() const override {
            return _returnType;
        };

        IFunction* curry(ISA::Reference* ref) const override {
            return new CurriedFunction(ref, this);
        }

        [[nodiscard]] CallVector getCallVector() const override {
            return {};
        }

        [[nodiscard]] IFunctionCall* call(CallVector vector) const override {
            return new InlineFunctionCall(_name, vector, _returnType);
        }

        [[nodiscard]] FunctionBackend backend() const override {
            return FunctionBackend::FB_INLINE;
        }

        [[nodiscard]] std::string name() const override {
            return _name;
        }

        [[nodiscard]] std::string toString() const override;

    protected:
        std::string _name;
        FormalTypes _types;
        const Type::Type* _returnType;
    };

}

#endif //SWARMVM_RUNTIME_FUNCTIONS
