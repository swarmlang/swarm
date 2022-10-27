#include "runtime_functions.h"
#include "../isa_meta.h"

namespace swarmc::Runtime {

    std::string CurriedFunction::toString() const {
        return "CurriedFunction<f: " + _upstream->toString() + ", ref: " + _ref->toString() + ">";
    }

    std::string InlineFunction::toString() const {
        return "InlineFunction<f:" + _name + ", rt: " + _returnType->toString() + ">";
    }

    std::string BuiltinFunction::toString() const {
        return "BuiltinFunction<f:" + tagToString(_tag) + ">";
    }

    std::string BuiltinFunction::tagToString(BuiltinFunctionTag tag) {
        if ( tag == BuiltinFunctionTag::NUMBER_TO_STRING ) return "NUMBER_TO_STRING";
        if ( tag == BuiltinFunctionTag::BOOLEAN_TO_STRING ) return "BOOLEAN_TO_STRING";
        if ( tag == BuiltinFunctionTag::SIN ) return "SIN";
        if ( tag == BuiltinFunctionTag::COS ) return "COS";
        if ( tag == BuiltinFunctionTag::TAN ) return "TAN";
        if ( tag == BuiltinFunctionTag::RANDOM ) return "RANDOM";
        if ( tag == BuiltinFunctionTag::RANDOM_VECTOR ) return "RANDOM_VECTOR";
        if ( tag == BuiltinFunctionTag::RANDOM_MATRIX ) return "RANDOM_MATRIX";
        if ( tag == BuiltinFunctionTag::RANGE ) return "RANGE";
        if ( tag == BuiltinFunctionTag::ENUMERATE ) return "ENUMERATE";
        return "UNKNOWN";
    }

    std::string BuiltinFunctionCall::toString() const {
        return "BuiltinFunctionCall<f:" + BuiltinFunction::tagToString(_tag) + ">";
    }
}
