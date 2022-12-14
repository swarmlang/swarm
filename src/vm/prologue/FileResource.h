#ifndef SWARM_FILERESOURCE_H
#define SWARM_FILERESOURCE_H

#include "prologue_provider.h"
#include "../runtime/fabric.h"
#include "../../lang/Type.h"

namespace swarmc::Runtime::Prologue {

    inline const Type::Opaque* fileType() {
        static auto inst = Type::Opaque::of("PROLOGUE::FILE");
        return inst;
    }

    class FileResource : public IResource {
    public:
        FileResource(NodeID owner, std::string id, std::string path):
            _id(std::move(id)), _path(std::move(path)), _owner(std::move(owner)) {}

        [[nodiscard]] ResourceCategory category() const override {
            return ResourceCategory::TUNNELED;
        }

        [[nodiscard]] std::string id() const override { return _id; }

        [[nodiscard]] NodeID owner() const override { return _owner; }

        [[nodiscard]] virtual std::string path() const { return _path; }

        [[nodiscard]] std::string name() const override { return "PROLOGUE::FILE"; }

        [[nodiscard]] const Type::Type* innerType() const override {
            return fileType();
        }

        [[nodiscard]] std::string toString() const override {
            return "Prologue::FileResource<path: " + _path + ", owner: " + _owner + ", id: " + _id + ">";
        }
    protected:
        std::string _id;
        std::string _path;
        NodeID _owner;
    };


    class ReadFileFunctionCall : public PrologueFunctionCall {
    public:
        ReadFileFunctionCall(IProvider* provider, CallVector vector, const Type::Type* returnType):
            PrologueFunctionCall(provider, std::move(vector), returnType) {}

        void execute(VirtualMachine*) override;

        [[nodiscard]] std::string toString() const override {
            return "ReadFileFunctionCall<>";
        }
    };

    class ReadFileFunction : public PrologueFunction {
    public:
        explicit ReadFileFunction(IProvider* provider) : PrologueFunction("READ_FILE", provider) {}

        [[nodiscard]] FormalTypes paramTypes() const override;

        [[nodiscard]] const Type::Type* returnType() const override;

        [[nodiscard]] PrologueFunctionCall* call(CallVector) const override;

        [[nodiscard]] std::string toString() const override {
            return "ReadFileFunction<>";
        }
    };


    class OpenFileFunctionCall : public PrologueFunctionCall {
    public:
        OpenFileFunctionCall(IProvider* provider, CallVector vector, const Type::Type* returnType):
            PrologueFunctionCall(provider, std::move(vector), returnType) {}

        void execute(VirtualMachine*) override;

        [[nodiscard]] std::string toString() const override {
            return "OpenFileFunctionCall<>";
        }
    };

    class OpenFileFunction : public PrologueFunction {
    public:
        explicit OpenFileFunction(IProvider* provider) : PrologueFunction("OPEN_FILE", provider) {}

        [[nodiscard]] FormalTypes paramTypes() const override;

        [[nodiscard]] const Type::Type* returnType() const override;

        [[nodiscard]] PrologueFunctionCall* call(CallVector) const override;

        [[nodiscard]] std::string toString() const override {
            return "OpenFileFunction<>";
        }
    };
}

#endif //SWARM_FILERESOURCE_H
