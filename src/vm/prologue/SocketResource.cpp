#include <cassert>
#include "../../shared/nslib.h"
#include "SocketResource.h"
#include "../VirtualMachine.h"

namespace swarmc::Runtime::Prologue {

	ResourceOperationFrame SocketResource::performOperation(VirtualMachine* vm, OperationName op, ResourceOperationFrame params) {
		if ( op == "PROLOGUE::OP::SOCKET::OPEN" ) {
			// Open the socket
			_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			assert(_socket > 0);

			// Configure the socket
			int opt = 1;
			auto setsockoptResult = setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
			assert(setsockoptResult == 0);

			setsockoptResult = setsockopt(_socket, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));
			assert(setsockoptResult == 0);

			// Bind the socket
			auto bindResult = ::bind(_socket, (struct sockaddr *) &_config, sizeof(_config));
			assert(bindResult == 0);

			// Start listening for connections
			auto listenResult = listen(_socket, (int) floor(_pendingConnectionLimit));
			assert(listenResult == 0);

			// TODO: handle failures in this call

			return {};
		}

		if ( op == "PROLOGUE::OP::SOCKET::ACCEPT" ) {
			auto conn = new SocketConnectionResource(_owner, vm->global()->getUuid());
			conn->acceptFromServer(_socket);
			_connections[conn->id()] = conn;

			return {new ISA::ResourceReference(conn)};
		}

		throw InvalidResourceOperation(s(this), op);
	}

	ResourceOperationFrame SocketConnectionResource::performOperation(VirtualMachine* vm, OperationName op, ResourceOperationFrame params) {
		if ( op == "PROLOGUE::OP::SOCKET::CONNECTION::RECEIVE" ) {
			std::vector<char> buffer(Configuration::SOCKET_MAX_BUFFER_SIZE);
			std::string data;
			std::size_t bytesReceived = ::recv(_client, &buffer[0], Configuration::SOCKET_MAX_BUFFER_SIZE, 0);
			assert(bytesReceived >= 0);
			data.append(buffer.cbegin(), buffer.cend());
			return {new ISA::StringReference(data)};
		}

		throw InvalidResourceOperation(s(this), op);
	}

	void SocketConnectionResource::acceptFromServer(int serverDescriptor) {
		_client = ::accept4(serverDescriptor, (struct sockaddr*) &_addr, &_addrLen, 0);
		assert(_client > 0);
		// fixme: handle error
	}

	void SocketFunctionCall::execute(VirtualMachine* vm) {
		auto port = (ISA::NumberReference*) _vector.at(0).second;
		auto pcl = (ISA::NumberReference*) _vector.at(1).second;

		auto socket = new SocketResource(vm->global()->getNodeId(), vm->global()->getUuid(), port->value(), pcl->value());
		auto ref = new ISA::ResourceReference(socket);
		setReturn(ref);
	}

	FormalTypes SocketFunction::paramTypes() const {
		return {Type::Primitive::of(Type::Intrinsic::NUMBER), Type::Primitive::of(Type::Intrinsic::NUMBER)};
	}

	Type::Type* SocketFunction::returnType() const {
		return socketType();
	}

	PrologueFunctionCall* SocketFunction::call(CallVector vector) const {
		return new SocketFunctionCall(_provider, vector, returnType());
	}

	void OpenSocketFunctionCall::execute(VirtualMachine* vm) {
		// Load the resource and make some sanity checks
		auto resource = (ISA::ResourceReference*) _vector.at(0).second;
		assert(resource->resource()->name() == "PROLOGUE::SOCKET");

		// Perform the operation on the resource
		auto socket = resource->resource();
		socket->performOperation(vm, "PROLOGUE::OP::SOCKET::OPEN", {});
	}

	FormalTypes OpenSocketFunction::paramTypes() const {
		return {Type::Resource::of(socketType())};
	}

	Type::Type* OpenSocketFunction::returnType() const {
		return Type::Primitive::of(Type::Intrinsic::VOID);
	}

	PrologueFunctionCall* OpenSocketFunction::call(CallVector vector) const {
		return new OpenSocketFunctionCall(_provider, vector, returnType());
	}


	void AcceptSocketConnectionFunctionCall::execute(VirtualMachine* vm) {
		// Load the resource and make some sanity checks
		auto resource = (ISA::ResourceReference*) _vector.at(0).second;
		assert(resource->resource()->name() == "PROLOGUE::SOCKET");

		// Perform the operation on the resource
		auto socket = resource->resource();
		auto result = socket->performOperation(vm, "PROLOGUE::OP::SOCKET::ACCEPT", {});
		assert(result.at(0)->tag() == ISA::ReferenceTag::RESOURCE);

		setReturn((ISA::ResourceReference*) result.at(0));
	}

	FormalTypes AcceptSocketConnectionFunction::paramTypes() const {
		return {Type::Resource::of(socketType())};
	}

	Type::Type* AcceptSocketConnectionFunction::returnType() const {
		return Type::Resource::of(socketConnectionType());
	}

	[[nodiscard]] PrologueFunctionCall* AcceptSocketConnectionFunction::call(CallVector vector) const {
		return new AcceptSocketConnectionFunctionCall(_provider, vector, returnType());
	}


	void ReadFromConnectionFunctionCall::execute(VirtualMachine* vm) {
		// Load the resource and make some sanity checks
		auto resource = (ISA::ResourceReference*) _vector.at(0).second;
		assert(resource->resource()->name() == "PROLOGUE::SOCKET::CONNECTION");

		// Perform the operation on the resource
		auto conn = resource->resource();
		auto result = conn->performOperation(vm, "PROLOGUE::OP::SOCKET::CONNECTION::RECEIVE", {});
		assert(result.at(0)->tag() == ISA::ReferenceTag::STRING);

		setReturn((ISA::StringReference*) result.at(0));
	}

	FormalTypes ReadFromConnectionFunction::paramTypes() const {
		return {Type::Resource::of(socketConnectionType())};
	}

	Type::Type* ReadFromConnectionFunction::returnType() const {
		return Type::Primitive::of(Type::Intrinsic::STRING);
	}

	PrologueFunctionCall* ReadFromConnectionFunction::call(CallVector vector) const {
		return new ReadFromConnectionFunctionCall(_provider, vector, returnType());
	}


	void SocketTFunctionCall::execute(VirtualMachine*) {
		setReturn(new ISA::TypeReference(socketType()));
	}

	FormalTypes SocketTFunction::paramTypes() const {
		return {};
	}

	Type::Type* SocketTFunction::returnType() const {
		return Type::Primitive::of(Type::Intrinsic::TYPE);
	}

	PrologueFunctionCall* SocketTFunction::call(CallVector vector) const {
		return new SocketTFunctionCall(_provider, vector, returnType());
	}

}
