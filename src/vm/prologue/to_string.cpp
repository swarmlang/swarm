#include <sstream>
#include "to_string.h"
#include "../../lang/Type.h"
#include "../ISA.h"

namespace swarmc::Runtime::Prologue {

    void NumberToStringFunctionCall::execute(VirtualMachine*) {
        // At this point, the call should have been type-checked by the runtime
        // So, we're going to assume that the parameters are correct.
        auto opd = (ISA::NumberReference*) _vector.at(0).second;
        setReturn(new ISA::StringReference(std::to_string(opd->value())));
    }

    FormalTypes NumberToStringFunction::paramTypes() const {
        return {Type::Primitive::of(Type::Intrinsic::NUMBER)};
    }

    Type::Type* NumberToStringFunction::returnType() const {
        return Type::Primitive::of(Type::Intrinsic::STRING);
    }

    PrologueFunctionCall* NumberToStringFunction::call(CallVector vector) const {
        auto returnType = Type::Primitive::of(Type::Intrinsic::STRING);
        return new NumberToStringFunctionCall(_provider, vector, returnType);
    }

    void BooleanToStringFunctionCall::execute(VirtualMachine*) {
        auto opd = (ISA::BooleanReference*) _vector.at(0).second;
        if ( opd->value() ) setReturn(new ISA::StringReference("true"));
        else setReturn(new ISA::StringReference("false"));
    }

    FormalTypes BooleanToStringFunction::paramTypes() const {
        return {Type::Primitive::of(Type::Intrinsic::BOOLEAN)};
    }

    Type::Type* BooleanToStringFunction::returnType() const {
        return Type::Primitive::of(Type::Intrinsic::STRING);
    }

    PrologueFunctionCall* BooleanToStringFunction::call(CallVector vector) const {
        auto returnType = Type::Primitive::of(Type::Intrinsic::STRING);
        return new BooleanToStringFunctionCall(_provider, vector, returnType);
    }

    void VectorToStringFunctionCall::execute(VirtualMachine*) {
        std::stringstream s;
        auto vector = (ISA::EnumerationReference*) _vector.at(0).second;

        bool isFirst = true;
        s << "[";

        for ( std::size_t i = 0; i < vector->length(); i += 1 ) {
            auto item = (ISA::NumberReference*) vector->get(i);

            if ( !isFirst ) {
                s << ", ";
            }

            s << item->value();
            isFirst = false;
        }

        s << "]";
        setReturn(new ISA::StringReference(s.str()));
    }

    PrologueFunctionCall* VectorToStringFunction::call(CallVector vector) const {
        return new VectorToStringFunctionCall(_provider, vector, returnType());
    }

    void MatrixToStringFunctionCall::execute(VirtualMachine*) {
        std::stringstream s;
        auto vector = (ISA::EnumerationReference*) _vector.at(0).second;

        bool isFirstOuter = true;
        s << "[";

        for ( std::size_t i = 0; i < vector->length(); i += 1 ) {
            auto item = (ISA::EnumerationReference*) vector->get(i);

            if ( !isFirstOuter ) {
                s << ",\n";
            }

            s << "[";

            bool isFirstInner = true;
            for ( std::size_t j = 0; j < item->length(); j += 1 ) {
                auto elem = (ISA::NumberReference*) item->get(j);

                if ( !isFirstInner ) {
                    s << ", ";
                }

                s << elem->value();
                isFirstInner = false;
            }

            s << "]";
            isFirstOuter = false;
        }

        s << "]";
        setReturn(new ISA::StringReference(s.str()));
    }

    PrologueFunctionCall* MatrixToStringFunction::call(CallVector vector) const {
        return new MatrixToStringFunctionCall(_provider, vector, returnType());
    }

}
