#include <cassert>
#include <fstream>
#include "../../shared/nslib.h"
#include "FileResource.h"
#include "../isa_meta.h"

namespace swarmc::Runtime::Prologue {

    void ReadFileFunctionCall::execute(VirtualMachine* vm) {
        // Load the resource and make a few sanity checks. These should be guaranteed by the VM, but just in case.
        auto resource = (ISA::ResourceReference*) _vector.at(0).second;
        assert(resource->resource()->name() == "PROLOGUE::FILE");

        auto file = (FileResource*) resource->resource();

        // Try to open the file
        std::ifstream fh(file->path());
        if ( !fh ) {
            throw Errors::RuntimeError(
                Errors::RuntimeExCode::InvalidOrMissingFilePath,
                "Unable to open path to file: " + file->path() + "(" + strerror(errno) + ")"
            );
        }

        // Read the contents of the file from the path
        auto content = nslib::stl::readStreamContents(fh);

        // Set the contents as the return value of the call
        setReturn(new ISA::StringReference(content));
    }

    Resources ReadFileFunctionCall::needsResources() const {
        auto resource = (ISA::ResourceReference*) _vector.at(0).second;
        return {resource->resource()};
    }


    FormalTypes ReadFileFunction::paramTypes() const {
        return {Type::Resource::of(fileType())};
    }

    const Type::Type* ReadFileFunction::returnType() const {
        return Type::Primitive::of(Type::Intrinsic::STRING);
    }

    PrologueFunctionCall* ReadFileFunction::call(CallVector v) const {
        return new ReadFileFunctionCall(_provider, v, returnType());
    }


    void OpenFileFunctionCall::execute(VirtualMachine*) {
        auto global = _provider->global();
        auto path = (ISA::StringReference*) _vector.at(0).second;
        auto resource = new FileResource(global->getNodeId(), global->getUuid(), path->value());
        setReturn(new ISA::ResourceReference(resource));
    }

    FormalTypes OpenFileFunction::paramTypes() const {
        return {Type::Primitive::of(Type::Intrinsic::STRING)};
    }

    const Type::Type* OpenFileFunction::returnType() const {
        return Type::Resource::of(fileType());
    }

    PrologueFunctionCall* OpenFileFunction::call(CallVector v) const {
        return new OpenFileFunctionCall(_provider, v, returnType());
    }


    void FileTFunctionCall::execute(VirtualMachine*) {
        setReturn(new ISA::TypeReference(Type::Opaque::of("PROLOGUE::FILE")));
    }

    FormalTypes FileTFunction::paramTypes() const {
        return {};
    }

    const Type::Type* FileTFunction::returnType() const {
        return Type::Primitive::of(Type::Intrinsic::TYPE);
    }

    PrologueFunctionCall* FileTFunction::call(CallVector v) const {
        return new FileTFunctionCall(_provider, v, returnType());
    }

}
