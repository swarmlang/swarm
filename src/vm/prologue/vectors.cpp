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

}
