#ifndef SWARM_FILERESOURCE_H
#define SWARM_FILERESOURCE_H

#include "prologue_provider.h"
#include "../runtime/fabric.h"
#include "../../lang/Type.h"

namespace swarmc::Runtime::Prologue {

    inline Type::Opaque* fileType() {
        return Type::Opaque::of("PROLOGUE::FILE");
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

        [[nodiscard]] Type::Type* innerType() const override {
            return fileType();
        }

        ResourceOperationFrame performOperation(VirtualMachine*, OperationName, ResourceOperationFrame) override;

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
        ReadFileFunctionCall(IProvider* provider, const CallVector& vector, Type::Type* returnType):
            PrologueFunctionCall(provider, "READ_FILE", vector, returnType) {}

        void execute(VirtualMachine*) override;

        [[nodiscard]] Resources needsResources() const override;

        [[nodiscard]] std::string toString() const override {
            return "ReadFileFunctionCall<>";
        }
    };

    class ReadFileFunction : public PrologueFunction {
    public:
        explicit ReadFileFunction(IProvider* provider) : PrologueFunction("READ_FILE", provider) {}

        [[nodiscard]] FormalTypes paramTypes() const override;

        [[nodiscard]] Type::Type* returnType() const override;

        [[nodiscard]] PrologueFunctionCall* call(CallVector) const override;

        [[nodiscard]] std::string toString() const override {
            return "ReadFileFunction<>";
        }
    };


    class WriteFileFunctionCall : public PrologueFunctionCall {
    public:
        WriteFileFunctionCall(IProvider* provider, const CallVector& vector, Type::Type* returnType):
            PrologueFunctionCall(provider, "WRITE_FILE", vector, returnType) {}

        void execute(VirtualMachine*) override;

        [[nodiscard]] Resources needsResources() const override;

        [[nodiscard]] std::string toString() const override {
            return "WriteFileFunctionCall<>";
        }
    };

    class WriteFileFunction : public PrologueFunction {
    public:
        explicit WriteFileFunction(IProvider* provider) : PrologueFunction("WRITE_FILE", provider) {}

        [[nodiscard]] FormalTypes paramTypes() const override;

        [[nodiscard]] Type::Type* returnType() const override;

        [[nodiscard]] PrologueFunctionCall* call(CallVector) const override;

        [[nodiscard]] std::string toString() const override {
            return "WriteFileFunction<>";
        }
    };


    class AppendFileFunctionCall : public PrologueFunctionCall {
    public:
        AppendFileFunctionCall(IProvider* provider, const CallVector& vector, Type::Type* returnType):
            PrologueFunctionCall(provider, "APPEND_FILE", vector, returnType) {}

        void execute(VirtualMachine*) override;

        [[nodiscard]] Resources needsResources() const override;

        [[nodiscard]] std::string toString() const override {
            return "AppendFileFunctionCall<>";
        }
    };

    class AppendFileFunction : public PrologueFunction {
    public:
        explicit AppendFileFunction(IProvider* provider) : PrologueFunction("APPEND_FILE", provider) {}

        [[nodiscard]] FormalTypes paramTypes() const override;

        [[nodiscard]] Type::Type* returnType() const override;

        [[nodiscard]] PrologueFunctionCall* call(CallVector) const override;

        [[nodiscard]] std::string toString() const override {
            return "AppendFileFunction<>";
        }
    };


    class OpenFileFunctionCall : public PrologueFunctionCall {
    public:
        OpenFileFunctionCall(IProvider* provider, const CallVector& vector, Type::Type* returnType):
            PrologueFunctionCall(provider, "OPEN_FILE", vector, returnType) {}

        void execute(VirtualMachine*) override;

        [[nodiscard]] std::string toString() const override {
            return "OpenFileFunctionCall<>";
        }
    };

    class OpenFileFunction : public PrologueFunction {
    public:
        explicit OpenFileFunction(IProvider* provider) : PrologueFunction("OPEN_FILE", provider) {}

        [[nodiscard]] FormalTypes paramTypes() const override;

        [[nodiscard]] Type::Type* returnType() const override;

        [[nodiscard]] PrologueFunctionCall* call(CallVector) const override;

        [[nodiscard]] std::string toString() const override {
            return "OpenFileFunction<>";
        }
    };

    class FileTFunctionCall : public PrologueFunctionCall {
    public:
        FileTFunctionCall(IProvider* provider, const CallVector& vector, Type::Type* returnType):
            PrologueFunctionCall(provider, "FILE_T", vector, returnType) {}

        void execute(VirtualMachine*) override;

        [[nodiscard]] std::string toString() const override {
            return "FileTFunctionCall<>";
        }
    };

    class FileTFunction : public PrologueFunction {
    public:
        explicit FileTFunction(IProvider* provider) : PrologueFunction("FILE_T", provider) {}

        [[nodiscard]] FormalTypes paramTypes() const override;

        [[nodiscard]] Type::Type* returnType() const override;

        [[nodiscard]] PrologueFunctionCall* call(CallVector) const override;

        [[nodiscard]] std::string toString() const override {
            return "FileTFunction<>";
        }
    };
}

#endif //SWARM_FILERESOURCE_H
