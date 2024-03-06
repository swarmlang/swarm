#include <cmath>
#include <iostream>
#include "range.h"

namespace swarmc::Runtime::Prologue {

    void RangeFunctionCall::execute(VirtualMachine*) {
        auto startNum = (ISA::NumberReference*) _vector.at(0).second;
        auto endNum = (ISA::NumberReference*) _vector.at(1).second;
        auto stepSize = (ISA::NumberReference*) _vector.at(2).second;

        auto step = stepSize->value();
        auto start = startNum->value();
        auto end = endNum->value();
        auto len = static_cast<std::size_t>(floor((std::fabs(end - start)) / std::fabs(step)));
        if ( (start > end && step > 0) || (start < end && step < 0) ) len = 0;

        auto enumeration = new ISA::EnumerationReference(Type::Primitive::of(Type::Intrinsic::NUMBER));
        enumeration->reserve(len);

        if ( len != 0 ) {
            auto n = start;
            for ( auto i = 0; i < len; i++ ) {
                enumeration->append(new ISA::NumberReference(static_cast<double>(n)));
                n += step;
            }
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

    Type::Type* RangeFunction::returnType() const {
        return new Type::Enumerable(Type::Primitive::of(Type::Intrinsic::NUMBER));
    }

    PrologueFunctionCall* RangeFunction::call(CallVector vector) const {
        return new RangeFunctionCall(_provider, vector, returnType());
    }

}
