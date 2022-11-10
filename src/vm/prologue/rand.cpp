#include "rand.h"
#include "../isa_meta.h"

namespace swarmc::Runtime::Prologue {

    void RandomFunctionCall::execute() {
        setReturn(new ISA::NumberReference(_provider->global()->random()));
    }

    const Type::Type* RandomFunction::returnType() const {
        return Type::Primitive::of(Type::Intrinsic::NUMBER);
    }

    PrologueFunctionCall* RandomFunction::call(CallVector vector) const {
        return new RandomFunctionCall(_provider, vector, returnType());
    }


    void RandomVectorFunctionCall::execute() {
        auto len = (ISA::NumberReference*) _vector.at(0).second;
        auto enumeration = new ISA::EnumerationReference(Type::Primitive::of(Type::Intrinsic::NUMBER));
        enumeration->reserve(len->value());

        for ( size_t i = 0; i < len->value(); i += 1 ) {
            enumeration->append(new ISA::NumberReference(_provider->global()->random()));
        }

        setReturn(enumeration);
    }

    FormalTypes RandomVectorFunction::paramTypes() const {
        return {Type::Primitive::of(Type::Intrinsic::NUMBER)};
    }

    const Type::Type* RandomVectorFunction::returnType() const {
        return new Type::Enumerable(Type::Primitive::of(Type::Intrinsic::NUMBER));
    }

    PrologueFunctionCall* RandomVectorFunction::call(CallVector vector) const {
        return new RandomVectorFunctionCall(_provider, vector, returnType());
    }


    void RandomMatrixFunctionCall::execute() {
        auto nRows = (ISA::NumberReference*) _vector.at(0).second;
        auto nCols = (ISA::NumberReference*) _vector.at(1).second;

        auto enumOfNumsType = new Type::Enumerable(Type::Primitive::of(Type::Intrinsic::NUMBER));
        auto matrix = new ISA::EnumerationReference(enumOfNumsType);
        matrix->reserve(nRows->value());

        for ( size_t i = 0; i < nRows->value(); i += 1 ) {
            auto row = new ISA::EnumerationReference(Type::Primitive::of(Type::Intrinsic::NUMBER));
            row->reserve(nCols->value());

            for ( size_t j = 0; j < nCols->value(); j += 1 ) {
                row->append(new ISA::NumberReference(_provider->global()->random()));
            }

            matrix->append(row);
        }

        setReturn(matrix);
    }

    FormalTypes RandomMatrixFunction::paramTypes() const {
        return {
            Type::Primitive::of(Type::Intrinsic::NUMBER),
            Type::Primitive::of(Type::Intrinsic::NUMBER)
        };
    }

    const Type::Type* RandomMatrixFunction::returnType() const {
        auto enumOfNumsType = new Type::Enumerable(Type::Primitive::of(Type::Intrinsic::NUMBER));
        return new Type::Enumerable(enumOfNumsType);
    }

    PrologueFunctionCall* RandomMatrixFunction::call(CallVector vector) const {
        return new RandomMatrixFunctionCall(_provider, vector, returnType());
    }

}
