#include "vectors.h"
#include "../isa_meta.h"

namespace swarmc::Runtime::Prologue {

    void ZeroVectorFunctionCall::execute(VirtualMachine*) {
        auto len = (ISA::NumberReference*) _vector.at(0).second;
        auto vector = new ISA::EnumerationReference(
            Type::Primitive::of(Type::Intrinsic::NUMBER)
        );

        auto size = static_cast<std::size_t>(len->value());
        vector->reserve(size);

        auto zero = new ISA::NumberReference(0);
        for ( std::size_t i = 0; i < size; i += 1 ) {
            vector->append(zero);
        }

        setReturn(vector);
    }

    PrologueFunctionCall* ZeroVectorFunction::call(CallVector vector) const {
        return new ZeroVectorFunctionCall(_provider, vector, returnType());
    }



    void ZeroMatrixFunctionCall::execute(VirtualMachine*) {
        auto nRows = (ISA::NumberReference*) _vector.at(0).second;
        auto nCols = (ISA::NumberReference*) _vector.at(1).second;

        auto enumOfNumsType = new Type::Enumerable(Type::Primitive::of(Type::Intrinsic::NUMBER));
        auto matrix = new ISA::EnumerationReference(enumOfNumsType);
        auto zero = new ISA::NumberReference(0);
        matrix->reserve(static_cast<std::size_t>(nRows->value()));

        for ( std::size_t i = 0; i < static_cast<std::size_t>(nRows->value()); i += 1 ) {
            auto row = new ISA::EnumerationReference(Type::Primitive::of(Type::Intrinsic::NUMBER));
            row->reserve(static_cast<std::size_t>(nCols->value()));

            for ( std::size_t j = 0; j < static_cast<std::size_t>(nCols->value()); j += 1 ) {
                row->append(zero);
            }

            matrix->append(row);
        }

        setReturn(matrix);
    }

    PrologueFunctionCall* ZeroMatrixFunction::call(CallVector vector) const {
        return new ZeroMatrixFunctionCall(_provider, vector, returnType());
    }


    void SubVectorFunctionCall::execute(VirtualMachine*) {
        auto startAt = static_cast<std::size_t>(((ISA::NumberReference*) _vector.at(0).second)->value());
        auto length = static_cast<std::size_t>(((ISA::NumberReference*) _vector.at(1).second)->value());
        auto vector = (ISA::EnumerationReference*) _vector.at(2).second;

        if ( startAt >= vector->length() ) {
            // FIXME: generate runtime exception
        }

        auto actualLength = length;
        if ( startAt + length > vector->length() ) {
            actualLength = vector->length() - startAt;
        }

        auto sliced = new ISA::EnumerationReference(Type::Primitive::of(Type::Intrinsic::NUMBER));
        sliced->reserve(actualLength);

        for ( std::size_t i = 0; i < actualLength; i += 1 ) {
            sliced->append(vector->get(i + startAt));
        }

        setReturn(sliced);
    }

    PrologueFunctionCall* SubVectorFunction::call(CallVector vector) const {
        return new SubVectorFunctionCall(_provider, vector, returnType());
    }


    void SubMatrixFunctionCall::execute(VirtualMachine*) {
        auto x0 = static_cast<std::size_t>(((ISA::NumberReference*) _vector.at(0).second)->value());
        auto x1 = static_cast<std::size_t>(((ISA::NumberReference*) _vector.at(1).second)->value());
        auto y0 = static_cast<std::size_t>(((ISA::NumberReference*) _vector.at(2).second)->value());
        auto y1 = static_cast<std::size_t>(((ISA::NumberReference*) _vector.at(3).second)->value());
        auto matrix = (ISA::EnumerationReference*) _vector.at(4).second;

        if ( x0 >= matrix->length() ) {
            // FIXME: generate runtime exception
        }

        auto actualXs = x1 - x0;
        if ( x1 > matrix->length() ) {
            actualXs = matrix->length() - x0;
        }

        auto sliced = new ISA::EnumerationReference(new Type::Enumerable(Type::Primitive::of(Type::Intrinsic::NUMBER)));
        sliced->reserve(actualXs);

        for ( std::size_t x = 0; x < actualXs; x += 1 ) {
            auto vector = (ISA::EnumerationReference*) matrix->get(x0 + x);

            if ( y0 >= vector->length() ) {
                // FIXME: generate runtime exception
            }

            auto actualYs = y1 - y0;
            if ( y1 > vector->length() ) {
                actualYs = vector->length() - y0;
            }

            auto slicedVector = new ISA::EnumerationReference(Type::Primitive::of(Type::Intrinsic::NUMBER));
            slicedVector->reserve(actualYs);

            for ( std::size_t y = 0; y < actualYs; y += 1 ) {
                auto item = (ISA::NumberReference*) vector->get(y0 + y);
                slicedVector->append(item);
            }

            sliced->append(slicedVector);
        }

        setReturn(sliced);
    }

    PrologueFunctionCall* SubMatrixFunction::call(CallVector vector) const {
        return new SubMatrixFunctionCall(_provider, vector, returnType());
    }

}
