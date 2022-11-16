#ifndef SWARMVM_TAG
#define SWARMVM_TAG

#include <utility>

#include "prologue_provider.h"
#include "../isa_meta.h"

namespace swarmc::Runtime::Prologue {

    class TagFunctionCall : public PrologueFunctionCall {
    public:
        TagFunctionCall(IProvider* provider, CallVector vector, const Type::Type* returnType):
            PrologueFunctionCall(provider, std::move(vector), returnType) {}

        void execute() override;

        std::string toString() const override {
            return "TagFunctionCall<>";
        }
    };

    class TagFunction : public PrologueFunction {
    public:
        explicit TagFunction(IProvider* provider) : PrologueFunction(provider) {}

        FormalTypes paramTypes() const override;

        const Type::Type* returnType() const override;

        PrologueFunctionCall* call(CallVector) const override;

        std::string toString() const override {
            return "TagFunction<>";
        }
    };

    class TagResource : public IResource {
    public:
        TagResource(IProvider* provider, const std::string& name) : _provider(provider), _name(name) {}

        void open() override;

        void close() override;

        bool isOpen() override;

        const Type::Type* innerType() override;

        ISA::Reference* innerValue() override;

        std::string toString() const override;

    protected:
        IProvider* _provider;
        std::string _name;
        std::map<std::string, std::string> _old;
    };

}

#endif //SWARMVM_TAG
