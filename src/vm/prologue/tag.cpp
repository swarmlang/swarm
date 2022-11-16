#include "../runtime/interfaces.h"
#include "tag.h"

namespace swarmc::Runtime::Prologue {

    void TagFunctionCall::execute() {
        auto nameRef = (ISA::StringReference*) _vector[0].second;
        auto tag = new TagResource(_provider, nameRef->value());
        setReturn(new ISA::ResourceReference(tag));
    }

    FormalTypes TagFunction::paramTypes() const {
        return {Type::Primitive::of(Type::Intrinsic::STRING)};
    }

    const Type::Type* TagFunction::returnType() const {
        return Type::Resource::of(Type::Primitive::of(Type::Intrinsic::VOID));
    }

    PrologueFunctionCall* TagFunction::call(CallVector vector) const {
        return new TagFunctionCall(_provider, vector, returnType());
    }

    void TagResource::open() {
        _old = _provider->global()->getSchedulingFilters();
        _provider->global()->applySchedulingFilter("tag", _name);
    }

    void TagResource::close() {
        _provider->global()->applySchedulingFilters(_old);
    }

    bool TagResource::isOpen() {
        auto filters = _provider->global()->getContextFilters();
        return filters.find("tag") != filters.end() && filters["tag"] == _name;
    }

    const Type::Type* TagResource::innerType() {
        return Type::Primitive::of(Type::Intrinsic::VOID);
    }

    ISA::Reference* TagResource::innerValue() {
        return new ISA::VoidReference();
    }

    std::string TagResource::toString() const {
        return "TagResource<name: " + _name + ">";
    }

}
