#include <cassert>
#include <fstream>
#include "../../shared/nslib.h"
#include "FileResource.h"
#include "../isa_meta.h"
#include "../VirtualMachine.h"
#include "../runtime/fabric.h"


namespace swarmc::Runtime::Prologue {

    ResourceOperationFrame FileResource::performOperation(VirtualMachine* vm, OperationName op, ResourceOperationFrame params) {
        if ( op == "PROLOGUE::OP::FILE::READ" ) {
            // Try to open the file
            std::ifstream fh(path());
            if ( !fh ) {
                throw Errors::RuntimeError(
                    Errors::RuntimeExCode::InvalidOrMissingFilePath,
                    "Unable to open path to file: " + path() + "(" + strerror(errno) + ")"
                );
            }

            // Read the contents of the file from the path
            auto content = nslib::stl::readStreamContents(fh);

            // Set the contents as the return value of the operation
            return {new ISA::StringReference(content)};
        }


        if ( op == "PROLOGUE::OP::FILE::WRITE" ) {
            // Try to get the contents
            assert(!params.empty());
            assert(params.at(0)->tag() == ISA::ReferenceTag::STRING);
            auto content = dynamic_cast<ISA::StringReference*>(params.at(0));
            GC_LOCAL_REF(content)

            // Try to open the file
            std::ofstream fh(path());
            if ( !fh ) {
                throw Errors::RuntimeError(
                    Errors::RuntimeExCode::InvalidOrMissingFilePath,
                    "Unable to open path to file: " + path() + "(" + strerror(errno) + ")"
                );
            }

            // Write the contents to the file
            fh << content->value();
            return {};
        }


        if ( op == "PROLOGUE::OP::FILE::APPEND" ) {
            // Try to get the contents
            assert(!params.empty());
            assert(params.at(0)->tag() == ISA::ReferenceTag::STRING);
            auto content = dynamic_cast<ISA::StringReference*>(params.at(0));
            GC_LOCAL_REF(content)

            // Try to open the file
            std::ofstream fh(path(), std::ios_base::app);
            if ( !fh ) {
                throw Errors::RuntimeError(
                    Errors::RuntimeExCode::InvalidOrMissingFilePath,
                    "Unable to open path to file: " + path() + "(" + strerror(errno) + ")"
                );
            }

            // Write the contents to the file
            fh << content->value();
            return {};
        }


        throw InvalidResourceOperation(s(this), op);
    }

    void ReadFileFunctionCall::execute(VirtualMachine* vm) {
        // Load the resource and make a few sanity checks. These should be guaranteed by the VM, but just in case.
        auto resource = (ISA::ResourceReference*) _vector.at(0).second;
        assert(resource->resource()->name() == "PROLOGUE::FILE");

        // Perform the read operation on the resource
        auto file = resource->resource();
        auto result = file->performOperation(vm, "PROLOGUE::OP::FILE::READ", {});

        // Get the result and set it as the return value of the call
        assert(!result.empty());
        assert(result.at(0)->tag() == ISA::ReferenceTag::STRING);

        setReturn(dynamic_cast<ISA::StringReference*>(result.at(0)));
    }

    Resources ReadFileFunctionCall::needsResources() const {
        auto resource = (ISA::ResourceReference*) _vector.at(0).second;
        return {resource->resource()};
    }


    FormalTypes ReadFileFunction::paramTypes() const {
        return {Type::Resource::of(fileType())};
    }

    Type::Type* ReadFileFunction::returnType() const {
        return Type::Primitive::of(Type::Intrinsic::STRING);
    }

    PrologueFunctionCall* ReadFileFunction::call(CallVector v) const {
        return new ReadFileFunctionCall(_provider, v, returnType());
    }


    void WriteFileFunctionCall::execute(VirtualMachine* vm) {
        // Load the resource and make a few sanity checks. These should be guaranteed by the VM, but just in case.
        auto resource = (ISA::ResourceReference*) _vector.at(0).second;
        assert(resource->resource()->name() == "PROLOGUE::FILE");

        // Perform the write operation on the resource
        auto content = (ISA::StringReference*) _vector.at(1).second;
        auto file = resource->resource();
        file->performOperation(vm, "PROLOGUE::OP::FILE::WRITE", {content});
    }

    Resources WriteFileFunctionCall::needsResources() const {
        auto resource = (ISA::ResourceReference*) _vector.at(0).second;
        return {resource->resource()};
    }


    FormalTypes WriteFileFunction::paramTypes() const {
        return {Type::Resource::of(fileType()), Type::Primitive::of(Type::Intrinsic::STRING)};
    }

    Type::Type* WriteFileFunction::returnType() const {
        return Type::Primitive::of(Type::Intrinsic::VOID);
    }

    PrologueFunctionCall* WriteFileFunction::call(CallVector v) const {
        return new WriteFileFunctionCall(_provider, v, returnType());
    }


    void AppendFileFunctionCall::execute(VirtualMachine* vm) {
        // Load the resource and make a few sanity checks. These should be guaranteed by the VM, but just in case.
        auto resource = (ISA::ResourceReference*) _vector.at(0).second;
        assert(resource->resource()->name() == "PROLOGUE::FILE");

        // Perform the write operation on the resource
        auto content = (ISA::StringReference*) _vector.at(1).second;
        auto file = resource->resource();
        file->performOperation(vm, "PROLOGUE::OP::FILE::APPEND", {content});
    }

    Resources AppendFileFunctionCall::needsResources() const {
        auto resource = (ISA::ResourceReference*) _vector.at(0).second;
        return {resource->resource()};
    }


    FormalTypes AppendFileFunction::paramTypes() const {
        return {Type::Resource::of(fileType()), Type::Primitive::of(Type::Intrinsic::STRING)};
    }

    Type::Type* AppendFileFunction::returnType() const {
        return Type::Primitive::of(Type::Intrinsic::VOID);
    }

    PrologueFunctionCall* AppendFileFunction::call(CallVector v) const {
        return new AppendFileFunctionCall(_provider, v, returnType());
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

    Type::Type* OpenFileFunction::returnType() const {
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

    Type::Type* FileTFunction::returnType() const {
        return Type::Primitive::of(Type::Intrinsic::TYPE);
    }

    PrologueFunctionCall* FileTFunction::call(CallVector v) const {
        return new FileTFunctionCall(_provider, v, returnType());
    }

}
