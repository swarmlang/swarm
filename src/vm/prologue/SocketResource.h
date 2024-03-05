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

    class SocketConnectionResource;

    inline Type::Opaque* socketType() {
        return Type::Opaque::of("PROLOGUE::SOCKET");
    }

    class SocketResource : public IResource {
    public:
        SocketResource(NodeID owner, std::string id, double port, double pendingConnectionLimit)
            : _owner(std::move(owner)), _id(std::move(id)), _socket(0), _port(port), _pendingConnectionLimit(pendingConnectionLimit) {
            _config.sin_family = AF_INET;
            _config.sin_addr.s_addr = INADDR_ANY;
            _config.sin_port = htons(floor(_port));
            memset(_config.sin_zero, '\0', sizeof _config.sin_zero);
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
        int _socket;
        std::unordered_map<std::string, SocketConnectionResource*> _connections;
        double _port;
        double _pendingConnectionLimit;
        sockaddr_in _config {};
    };

    inline Type::Opaque* socketConnectionType() {
        return Type::Opaque::of("PROLOGUE::SOCKET::CONNECTION");
    }

    class SocketConnectionResource : public IResource {
    public:
        SocketConnectionResource(NodeID owner, std::string id)
            : _owner(std::move(owner)), _id(std::move(id)), _client(0), _addrLen(sizeof(_addr)) {}

        [[nodiscard]] ResourceCategory category() const override {
            return ResourceCategory::TUNNELED;
        }

        [[nodiscard]] std::string id() const override { return _id; }

        [[nodiscard]] NodeID owner() const override { return _owner; }

        [[nodiscard]] std::string name() const override { return "PROLOGUE::SOCKET::CONNECTION"; }

        void acceptFromServer(int serverDescriptor);

        [[nodiscard]] Type::Type* innerType() const override {
            return socketConnectionType();
        }

        ResourceOperationFrame performOperation(VirtualMachine*, OperationName, ResourceOperationFrame) override;

        [[nodiscard]] std::string toString() const override {
            return "Prologue::SocketConnectionResource<owner: " + s(_owner) + ", id: " + _id + ">";
        }

    protected:
        NodeID _owner;
        std::string _id;
        int _client;
        struct sockaddr_in _addr {};
        socklen_t _addrLen;
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

    class OpenSocketFunctionCall : public PrologueFunctionCall {
    public:
        OpenSocketFunctionCall(IProvider* provider, const CallVector& vector, Type::Type* returnType):
            PrologueFunctionCall(provider, "OPEN_SOCKET", vector, returnType) {}

        void execute(VirtualMachine*) override;

        [[nodiscard]] std::string toString() const override {
            return "OpenSocketFunctionCall<>";
        }
    };

    class OpenSocketFunction : public PrologueFunction {
    public:
        explicit OpenSocketFunction(IProvider* provider) : PrologueFunction("OPEN_SOCKET", provider) {}

        [[nodiscard]] FormalTypes paramTypes() const override;

        [[nodiscard]] Type::Type* returnType() const override;

        [[nodiscard]] PrologueFunctionCall* call(CallVector) const override;

        [[nodiscard]] std::string toString() const override {
            return "OpenSocketFunction<>";
        }
    };

    class AcceptSocketConnectionFunctionCall : public PrologueFunctionCall {
    public:
        AcceptSocketConnectionFunctionCall(IProvider* provider, const CallVector& vector, Type::Type* returnType):
            PrologueFunctionCall(provider, "ACCEPT_SOCKET_CONNECTION", vector, returnType) {}

        void execute(VirtualMachine*) override;

        [[nodiscard]] std::string toString() const override {
            return "AcceptSocketConnectionFunctionCall<>";
        }
    };

    class AcceptSocketConnectionFunction : public PrologueFunction {
    public:
        explicit AcceptSocketConnectionFunction(IProvider* provider) : PrologueFunction("ACCEPT_SOCKET_CONNECTION", provider) {}

        [[nodiscard]] FormalTypes paramTypes() const override;

        [[nodiscard]] Type::Type* returnType() const override;

        [[nodiscard]] PrologueFunctionCall* call(CallVector) const override;

        [[nodiscard]] std::string toString() const override {
            return "AcceptSocketConnectionFunction<>";
        }
    };

    class ReadFromConnectionFunctionCall : public PrologueFunctionCall {
    public:
        ReadFromConnectionFunctionCall(IProvider* provider, const CallVector& vector, Type::Type* returnType):
                PrologueFunctionCall(provider, "READ_FROM_CONNECTION", vector, returnType) {}

        void execute(VirtualMachine*) override;

        [[nodiscard]] std::string toString() const override {
            return "ReadFromConnectionFunctionCall<>";
        }
    };

    class ReadFromConnectionFunction : public PrologueFunction {
    public:
        explicit ReadFromConnectionFunction(IProvider* provider) : PrologueFunction("READ_FROM_CONNECTION", provider) {}

        [[nodiscard]] FormalTypes paramTypes() const override;

        [[nodiscard]] Type::Type* returnType() const override;

        [[nodiscard]] PrologueFunctionCall* call(CallVector) const override;

        [[nodiscard]] std::string toString() const override {
            return "ReadFromConnectionFunction<>";
        }
    };

    class SocketTFunctionCall : public PrologueFunctionCall {
    public:
        SocketTFunctionCall(IProvider* provider, const CallVector& vector, Type::Type* returnType):
                PrologueFunctionCall(provider, "SOCKET_T", vector, returnType) {}

        void execute(VirtualMachine*) override;

        [[nodiscard]] std::string toString() const override {
            return "SocketTFunctionCall<>";
        }
    };

    class SocketTFunction : public PrologueFunction {
    public:
        explicit SocketTFunction(IProvider* provider) : PrologueFunction("SOCKET_T", provider) {}

        [[nodiscard]] FormalTypes paramTypes() const override;

        [[nodiscard]] Type::Type* returnType() const override;

        [[nodiscard]] PrologueFunctionCall* call(CallVector) const override;

        [[nodiscard]] std::string toString() const override {
            return "SocketTFunction<>";
        }
    };

}

#endif //SWARM_SOCKETRESOURCE_H
