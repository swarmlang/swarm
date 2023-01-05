#ifndef SWARM_TAGRESOURCE_H
#define SWARM_TAGRESOURCE_H

#include "prologue_provider.h"
#include "../runtime/fabric.h"
#include "../../lang/Type.h"

namespace swarmc::Runtime::Prologue {

    inline const Type::Opaque* tagType() {
        static auto inst = Type::Opaque::of("PROLOGUE::TAG");
        return inst;
    }

    class TagResource : public IResource {
    public:
        TagResource(NodeID owner, std::string id, std::string key, std::string value):
            _id(std::move(id)), _key(std::move(key)), _value(std::move(value)), _owner(std::move(owner)) {}

        [[nodiscard]] ResourceCategory category() const override {
            return ResourceCategory::EXCLUSIVE;
        }

        [[nodiscard]] std::string id() const override { return _id; }

        [[nodiscard]] NodeID owner() const override { return _owner; }

        [[nodiscard]] virtual std::pair<std::string, std::string> filter() const { return {_key, _value}; }

        [[nodiscard]] std::string name() const override { return "PROLOGUE::TAG"; }

        [[nodiscard]] const Type::Type* innerType() const override { return tagType(); }

        [[nodiscard]] SchedulingFilters getSchedulingFilters() const override {
            return {filter()};
        }

        void acquire(VirtualMachine*) override;

        void release(VirtualMachine*) override;

        [[nodiscard]] std::string toString() const override {
            return "Prologue::TagResource<" + _key + " -> " + _value + ">";
        }
    protected:
        std::string _id;
        std::string _key;
        std::string _value;
        NodeID _owner;
        SchedulingFilters _old;
    };


    class TagFunctionCall : public PrologueFunctionCall {
    public:
        TagFunctionCall(IProvider* provider, const CallVector& vector, const Type::Type* returnType):
                PrologueFunctionCall(provider, "TAG", vector, returnType) {}

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


    class TagTFunctionCall : public PrologueFunctionCall {
    public:
        TagTFunctionCall(IProvider* provider, const CallVector& vector, const Type::Type* returnType):
                PrologueFunctionCall(provider, "TAG_T", vector, returnType) {}

        void execute(VirtualMachine*) override;

        [[nodiscard]] std::string toString() const override {
            return "TagTFunctionCall<>";
        }
    };

    class TagTFunction : public PrologueFunction {
    public:
        explicit TagTFunction(IProvider* provider) : PrologueFunction("TAG_T", provider) {}

        [[nodiscard]] FormalTypes paramTypes() const override;

        [[nodiscard]] const Type::Type* returnType() const override;

        [[nodiscard]] PrologueFunctionCall* call(CallVector) const override;

        [[nodiscard]] std::string toString() const override {
            return "TagTFunction<>";
        }
    };

}

#endif //SWARM_TAGRESOURCE_H
