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

        // Read the contents of the file from the path
        // FIXME: raise exception if fh is bad
        std::ifstream fh(file->path());
        auto content = nslib::stl::readStreamContents(fh);

        // Set the contents as the return value of the call
        setReturn(new ISA::StringReference(content));
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

}
