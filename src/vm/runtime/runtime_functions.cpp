#include "runtime_functions.h"
#include "../isa_meta.h"

namespace nslib {
    std::string s(swarmc::Runtime::FunctionBackend v) {
        if ( v == swarmc::Runtime::FunctionBackend::FB_INLINE ) return "FunctionBackend(INLINE)";
        if ( v == swarmc::Runtime::FunctionBackend::FB_PROVIDER ) return "FunctionBackend(PROVIDER)";
        return "FunctionBackend(UNKNOWN)";
    }
}

namespace swarmc::Runtime {

    IFunctionCall::IFunctionCall(
        FunctionBackend backend, std::string name, CallVector vector, const Type::Type* returnType):
            _backend(backend), _name(std::move(name)), _vector(std::move(vector)), _returnType(returnType) {

        for ( auto elem : _vector ) {
            useref(elem.second);
        }
    }

    IFunctionCall::~IFunctionCall() noexcept {
        for ( auto elem : _vector ) {
            freeref(elem.second);
        }

        freeref(_returnValue);
    }

    void IFunctionCall::setReturn(ISA::Reference* value) {
        _returnValue = useref(value);
    }

    CurriedFunction::CurriedFunction(ISA::Reference* ref, IFunction* upstream):
        _ref(useref(ref)), _upstream(useref(upstream)) {}

    CurriedFunction::~CurriedFunction() noexcept {
        freeref(_ref);
        freeref(_upstream);
    }

    std::string CurriedFunction::toString() const {
        return "CurriedFunction<f: " + _upstream->toString() + ", ref: " + _ref->toString() + ">";
    }

    std::string InlineFunction::toString() const {
        return "InlineFunction<f:" + _name + ", rt: " + _returnType->toString() + ">";
    }

}
