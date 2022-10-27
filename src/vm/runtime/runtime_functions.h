#ifndef SWARMVM_RUNTIME_FUNCTIONS
#define SWARMVM_RUNTIME_FUNCTIONS

#include <utility>
#include <vector>
#include "../../shared/IStringable.h"

namespace swarmc::Type {
    class Type;
}

namespace swarmc::ISA {
    class Reference;
}

namespace swarmc::Runtime {

    using FormalTypes = std::vector<const Type::Type*>;
    using CallVector = std::vector<std::pair<const Type::Type*, ISA::Reference*>>;

    enum class FunctionBackend {
        INLINE,
        BUILTIN,
    };

    enum class BuiltinFunctionTag {
        NUMBER_TO_STRING,
        BOOLEAN_TO_STRING,
        SIN,
        COS,
        TAN,
        RANDOM,
        RANDOM_VECTOR,
        RANDOM_MATRIX,
        RANGE,
        ENUMERATE,
    };


    class IFunctionCall : public IStringable {
    public:
        IFunctionCall(FunctionBackend backend, CallVector vector) : _backend(backend), _vector(std::move(vector)) {}

        virtual FunctionBackend backend() { return _backend; }

        virtual CallVector vector() { return _vector; }

    protected:
        FunctionBackend _backend;
        CallVector _vector;
    };

    class InlineFunctionCall : public IFunctionCall {
    public:
        InlineFunctionCall(size_t pc, CallVector vector) : IFunctionCall(FunctionBackend::INLINE, std::move(vector)), _pc(pc) {}

        virtual size_t jumpTo() const { return _pc; }

        std::string toString() const override {
            return "InlineFunctionCall<pc: " + std::to_string(_pc) + ">";
        }

    protected:
        size_t _pc;
    };

    class BuiltinFunctionCall : public IFunctionCall {
    public:
        BuiltinFunctionCall(BuiltinFunctionTag tag, CallVector vector) : IFunctionCall(FunctionBackend::BUILTIN, vector), _tag(tag) {}

        virtual BuiltinFunctionTag tag() const { return _tag; }

        std::string toString() const override;

    protected:
        BuiltinFunctionTag _tag;
    };


    class IFunction : public IStringable {
    public:
        virtual ~IFunction() = default;

        virtual FormalTypes paramTypes() const = 0;

        virtual const Type::Type* returnType() const = 0;

        virtual IFunction* curry(ISA::Reference*) const = 0;

        virtual CallVector getCallVector() const = 0;

        virtual IFunctionCall* call(CallVector) const = 0;

        virtual IFunctionCall* call() const { return call(getCallVector()); }

        virtual FunctionBackend backend() const = 0;
    };


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


    class InlineFunction : public IFunction {
    public:
        InlineFunction(size_t pc, std::string name, FormalTypes types, Type::Type* returnType)
            : _pc(pc), _name(name), _types(types), _returnType(returnType) {}

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
            return new InlineFunctionCall(_pc, vector);
        }

        FunctionBackend backend() const override {
            return FunctionBackend::INLINE;
        }

        std::string toString() const override;

    protected:
        size_t _pc;
        std::string _name;
        FormalTypes _types;
        Type::Type* _returnType;
    };


    class BuiltinFunction : public IFunction {
    public:
        explicit BuiltinFunction(BuiltinFunctionTag tag) : _tag(tag) {}

        static std::string tagToString(BuiltinFunctionTag);

        IFunction* curry(ISA::Reference* ref) const override {
            return new CurriedFunction(ref, this);
        }

        CallVector getCallVector() const override {
            return {};
        }

        IFunctionCall* call(CallVector vector) const override {
            return new BuiltinFunctionCall(_tag, vector);
        }

        FunctionBackend backend() const override {
            return FunctionBackend::BUILTIN;
        }

        std::string toString() const override;

    protected:
        BuiltinFunctionTag _tag;
    };

}

#endif //SWARMVM_RUNTIME_FUNCTIONS
