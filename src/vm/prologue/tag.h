#ifndef SWARMVM_TAG
#define SWARMVM_TAG

#include <utility>

#include "prologue_provider.h"
#include "../isa_meta.h"

namespace swarmc::Runtime::Prologue {

    /*class TagFunctionCall : public PrologueFunctionCall {
    public:
        TagFunctionCall(IProvider* provider, CallVector vector, const Type::Type* returnType):
            PrologueFunctionCall(provider, std::move(vector), returnType) {}

        void execute(VirtualMachine*) override;

        [[nodiscard]] std::string toString() const override {
            return "TagFunctionCall<>";
        }
    };

    class TagFunction : public PrologueFunction {
    public:
        explicit TagFunction(IProvider* provider) : PrologueFunction("TAG", provider) {}

        [[nodiscard]] FormalTypes paramTypes() const override;

        [[nodiscard]] const Type::Type* returnType() const override;

        [[nodiscard]] PrologueFunctionCall* call(CallVector) const override;

        [[nodiscard]] std::string toString() const override {
            return "TagFunction<>";
        }
    };

    class TagResource : public IResource {
    public:
        TagResource(IProvider* provider, std::string key, std::string value):
            _provider(provider), _key(std::move(key)), _value(std::move(value)) {}

        void open() override;

        void close() override;

        bool isOpen() override;

        const Type::Type* innerType() override;

        ISA::Reference* innerValue() override;

        [[nodiscard]] std::string toString() const override;

    protected:
        IProvider* _provider;
        std::string _key;
        std::string _value;
        std::map<std::string, std::string> _old;
    };*/

}

#endif //SWARMVM_TAG
