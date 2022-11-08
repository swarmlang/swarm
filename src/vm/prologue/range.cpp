#include <cmath>
#include <iostream>
#include "range.h"

namespace swarmc::Runtime::Prologue {

    void RangeFunctionCall::execute() {
        auto startNum = (ISA::NumberReference*) _vector.at(0).second;
        auto endNum = (ISA::NumberReference*) _vector.at(1).second;
        auto stepSize = (ISA::NumberReference*) _vector.at(2).second;

        auto len = static_cast<size_t>(floor(endNum->value() - startNum->value() / stepSize->value()));

        auto enumeration = new ISA::EnumerationReference(Type::Primitive::of(Type::Intrinsic::NUMBER));
        enumeration->reserve(len);

        for ( auto i = startNum->value(); i <= endNum->value(); i += stepSize->value() ) {
            enumeration->append(new ISA::NumberReference(i));
        }

        setReturn(enumeration);
    }

    FormalTypes RangeFunction::paramTypes() const {
        return {
            Type::Primitive::of(Type::Intrinsic::NUMBER),
            Type::Primitive::of(Type::Intrinsic::NUMBER),
            Type::Primitive::of(Type::Intrinsic::NUMBER)
        };
    }

    CType RangeFunction::returnType() const {
        return new Type::Enumerable(Type::Primitive::of(Type::Intrinsic::NUMBER));
    }

    PrologueFunctionCall* RangeFunction::call(CallVector vector) const {
        return new RangeFunctionCall(_provider, vector, returnType());
    }

}
