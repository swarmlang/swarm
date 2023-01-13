#include "resource.h"
#include "../VirtualMachine.h"

namespace swarmc::Runtime::Prologue {

    void ResourceTFunctionCall::execute(VirtualMachine*) {
        auto innerType = (ISA::TypeReference*) _vector.at(0).second;
        setReturn(new ISA::TypeReference(Type::Resource::of(innerType->value())));
    }

    FormalTypes ResourceTFunction::paramTypes() const {
        return {
            Type::Primitive::of(Type::Intrinsic::TYPE)
        };
    }

    Type::Type* ResourceTFunction::returnType() const {
        return Type::Primitive::of(Type::Intrinsic::TYPE);
    }

    PrologueFunctionCall* ResourceTFunction::call(CallVector vector) const {
        return new ResourceTFunctionCall(_provider, vector, returnType());
    }
}
