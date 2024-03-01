#include <cassert>
#include "../../shared/nslib.h"
#include "SocketResource.h"
#include "../VirtualMachine.h"

namespace swarmc::Runtime::Prologue {

	ResourceOperationFrame SocketResource::performOperation(VirtualMachine* vm, OperationName op, ResourceOperationFrame params) {
		if ( op == "PROLOGUE::OP::SOCKET::OPEN" ) {
			// Try to get port
			assert(params.size() == 2);
			assert(params.at(0)->tag() == ISA::ReferenceTag::NUMBER);
			auto port = dynamic_cast<ISA::NumberReference*>(params.at(0));
			GC_LOCAL_REF(port)

			assert(params.at(1)->tag() == ISA::ReferenceTag::NUMBER);
			auto maxConnections = dynamic_cast<ISA::NumberReference*>(params.at(1));
			GC_LOCAL_REF(maxConnections);

			// Configure the socket
			_config.sin_port = (int) floor(port->value());

			// Open the socket
			_socket = socket(AF_INET, SOCK_STREAM, 0);
			assert(_socket > 0);

			// Bind the socket
			bind(_socket, &_config, sizeof(_config));

			return {};
		}

		throw InvalidResourceOperation(s(this), op);
	}

	void SocketFunctionCall::execute(VirtualMachine* vm) {
		auto socket = new SocketResource(vm->global()->getNodeId(), vm->global()->getUuid());
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
