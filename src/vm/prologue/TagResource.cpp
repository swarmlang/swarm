#include "TagResource.h"
#include "../VirtualMachine.h"

namespace swarmc::Runtime::Prologue {

    void TagResource::acquire(VirtualMachine* vm) {
        _old = vm->global()->getSchedulingFilters();
        vm->global()->applySchedulingFilter(_key, _value);
    }

    void TagResource::release(VirtualMachine* vm) {
        vm->global()->applySchedulingFilters(_old);
        _old = {};
    }


    void TagFunctionCall::execute(VirtualMachine* vm) {
        auto global = _provider->global();
        auto key = (ISA::StringReference*) _vector.at(0).second;
        auto value = (ISA::StringReference*) _vector.at(1).second;
        auto resource = new TagResource(global->getNodeId(), global->getUuid(), key->value(), value->value());
        setReturn(new ISA::ResourceReference(resource));
    }

    FormalTypes TagFunction::paramTypes() const {
        return {
            Type::Primitive::of(Type::Intrinsic::STRING),
            Type::Primitive::of(Type::Intrinsic::STRING)
        };
    }

    const Type::Type* TagFunction::returnType() const {
        return Type::Resource::of(tagType());
    }

    PrologueFunctionCall* TagFunction::call(CallVector v) const {
        return new TagFunctionCall(_provider, v, returnType());
    }


    void TagTFunctionCall::execute(VirtualMachine* vm) {
        setReturn(new ISA::TypeReference(Type::Opaque::of("PROLOGUE::TAG")));
    }

    FormalTypes TagTFunction::paramTypes() const {
        return {};
    }

    const Type::Type* TagTFunction::returnType() const {
        return Type::Primitive::of(Type::Intrinsic::TYPE);
    }

    PrologueFunctionCall* TagTFunction::call(CallVector v) const {
        return new TagTFunctionCall(_provider, v, returnType());
    }

}
