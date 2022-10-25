#ifndef SWARMVM_RUNTIME_FUNCTIONS
#define SWARMVM_RUNTIME_FUNCTIONS

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

    class IFunction : public IStringable {
    public:
        virtual ~IFunction() = default;

        virtual FormalTypes paramTypes() const = 0;

        virtual const Type::Type* returnType() const = 0;

        virtual IFunction* curry(ISA::Reference*) const = 0;

        virtual CallVector getCallVector() const = 0;
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

        std::string toString() const override;

    protected:
        ISA::Reference* _ref;
        const IFunction* _upstream;
    };


    class InlineFunction : public IFunction {
    public:
        InlineFunction(std::string name, FormalTypes types, Type::Type* returnType)
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
            return CallVector();
        }

        std::string toString() const override;

    protected:
        std::string _name;
        FormalTypes _types;
        Type::Type* _returnType;
    };

}

#endif //SWARMVM_RUNTIME_FUNCTIONS
