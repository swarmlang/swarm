#include "../runtime/interfaces.h"
#include "tag.h"

namespace swarmc::Runtime::Prologue {

    void TagFunctionCall::execute() {
        auto keyRef = (ISA::StringReference*) _vector[0].second;
        auto valueRef = (ISA::StringReference*) _vector[1].second;
        auto tag = new TagResource(_provider, keyRef->value(), valueRef->value());
        setReturn(new ISA::ResourceReference(tag));
    }

    FormalTypes TagFunction::paramTypes() const {
        return {
            Type::Primitive::of(Type::Intrinsic::STRING),
            Type::Primitive::of(Type::Intrinsic::STRING)
        };
    }

    const Type::Type* TagFunction::returnType() const {
        return Type::Resource::of(Type::Primitive::of(Type::Intrinsic::VOID));
    }

    PrologueFunctionCall* TagFunction::call(CallVector vector) const {
        ensureCallable();
        return new TagFunctionCall(_provider, vector, returnType());
    }

    void TagResource::open() {
        _old = _provider->global()->getSchedulingFilters();
        _provider->global()->applySchedulingFilter(_key, _value);
    }

    void TagResource::close() {
        _provider->global()->applySchedulingFilters(_old);
    }

    bool TagResource::isOpen() {
        auto filters = _provider->global()->getContextFilters();
        return filters.find(_key) != filters.end() && filters[_key] == _value;
    }

    const Type::Type* TagResource::innerType() {
        return Type::Primitive::of(Type::Intrinsic::VOID);
    }

    ISA::Reference* TagResource::innerValue() {
        return new ISA::VoidReference();
    }

    std::string TagResource::toString() const {
        return "TagResource<k: " + _key + ", v: " + _value + ">";
    }

}
