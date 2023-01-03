#include <cmath>
#include <iostream>
#include "range.h"

namespace swarmc::Runtime::Prologue {

    void RangeFunctionCall::execute(VirtualMachine*) {
        auto startNum = (ISA::NumberReference*) _vector.at(0).second;
        auto endNum = (ISA::NumberReference*) _vector.at(1).second;
        auto stepSize = (ISA::NumberReference*) _vector.at(2).second;

        auto step = static_cast<std::size_t>(stepSize->value());
        auto start = static_cast<std::size_t>(startNum->value());
        auto end = static_cast<std::size_t>(endNum->value());
        auto len = static_cast<std::size_t>(floor(endNum->value() - startNum->value() / stepSize->value()));

        auto enumeration = new ISA::EnumerationReference(Type::Primitive::of(Type::Intrinsic::NUMBER));
        enumeration->reserve(len);

        for ( auto i = start; i <= end; i += step ) {
            enumeration->append(new ISA::NumberReference(static_cast<double>(i)));
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

    const Type::Type* RangeFunction::returnType() const {
        return new Type::Enumerable(Type::Primitive::of(Type::Intrinsic::NUMBER));
    }

    PrologueFunctionCall* RangeFunction::call(CallVector vector) const {
        return new RangeFunctionCall(_provider, vector, returnType());
    }

}
