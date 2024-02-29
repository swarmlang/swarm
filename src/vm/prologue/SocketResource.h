#ifndef SWARM_SOCKETRESOURCE_H
#define SWARM_SOCKETRESOURCE_H

#include <cstdio>
#include <utility>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include "prologue_provider.h"
#include "../runtime/fabric.h"
#include "../../lang/Type.h"

namespace swarmc::Runtime::Prologue {

    inline Type::Opaque* socketType() {
        return Type::Opaque::of("PROLOGUE::SOCKET");
    }

    class SocketResource : public IResource {
    public:
        SocketResource(NodeID owner, std::string id) : _owner(std::move(owner)), _id(std::move(id)) {
            _config.sin_family = AF_INET;
            _config.sin_addr.s_addr = INADDR_ANY;
        }

        [[nodiscard]] ResourceCategory category() const override {
            return ResourceCategory::TUNNELED;
        }

        [[nodiscard]] std::string id() const override { return _id; }

        [[nodiscard]] NodeID owner() const override { return _owner; }

        [[nodiscard]] std::string name() const override { return "PROLOGUE::SOCKET"; }

        [[nodiscard]] Type::Type* innerType() const override {
            return socketType();
        }

        ResourceOperationFrame performOperation(VirtualMachine*, OperationName, ResourceOperationFrame) override;

        [[nodiscard]] std::string toString() const override {
            return "Prologue::SocketResource<owner: " + s(_owner) + ", id: " + _id + ">";
        }

    protected:
        NodeID _owner;
        std::string _id;
        std::optional<int> _socket;
        sockaddr_in _config {};
    };

    class SocketFunctionCall : public PrologueFunctionCall {
    public:
        SocketFunctionCall(IProvider* provider, const CallVector& vector, Type::Type* returnType):
            PrologueFunctionCall(provider, "SOCKET", vector, returnType) {}

        void execute(VirtualMachine*) override;

        [[nodiscard]] std::string toString() const override {
            return "SocketFunctionCall<>";
        }
    };

    class SocketFunction : public PrologueFunction {
    public:
        explicit SocketFunction(IProvider* provider) : PrologueFunction("SOCKET", provider) {}

        [[nodiscard]] FormalTypes paramTypes() const override;

        [[nodiscard]] Type::Type* returnType() const override;

        [[nodiscard]] PrologueFunctionCall* call(CallVector) const override;

        [[nodiscard]] std::string toString() const override {
            return "SocketFunction<>";
        }
    };

    class SocketTFunctionCall : public PrologueFunctionCall {
    public:
        SocketTFunctionCall(IProvider* provider, const CallVector& vector, Type::Type* returnType):
                PrologueFunctionCall(provider, "FILE_T", vector, returnType) {}

        void execute(VirtualMachine*) override;

        [[nodiscard]] std::string toString() const override {
            return "SocketTFunctionCall<>";
        }
    };

    class SocketTFunction : public PrologueFunction {
    public:
        explicit SocketTFunction(IProvider* provider) : PrologueFunction("FILE_T", provider) {}

        [[nodiscard]] FormalTypes paramTypes() const override;

        [[nodiscard]] Type::Type* returnType() const override;

        [[nodiscard]] PrologueFunctionCall* call(CallVector) const override;

        [[nodiscard]] std::string toString() const override {
            return "SocketTFunction<>";
        }
    };

}

#endif //SWARM_SOCKETRESOURCE_H
