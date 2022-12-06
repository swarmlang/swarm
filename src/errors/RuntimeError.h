#ifndef SWARMC_RUNTIMEERROR_H
#define SWARMC_RUNTIMEERROR_H

#include "../shared/nslib.h"
#include "SwarmError.h"

namespace swarmc::Errors {

    enum class RuntimeExCode : size_t {
        InvalidArgumentType = 5,
        TypeError = 6,
        InvalidReferenceImplementation = 7,
        DivisionByZero = 8,
        WhileCallbackTypeInvalid = 9,
        WithCallbackTypeInvalid = 10,
        EnumIndexOutOfBounds = 11,
        EnumerateCallbackTypeInvalid = 12,
        FnParamOutsideCall = 13,
        ReturnOutsideCall = 14,
        InvalidMapKey = 15,
        InvalidAssignEval = 16,
        StreamNotOpen = 17,
        StreamEmpty = 18,
        ResumeOutsideExHandler = 19,
    };

}

namespace nslib {

    inline std::string s(swarmc::Errors::RuntimeExCode v) {
        if ( v == swarmc::Errors::RuntimeExCode::InvalidArgumentType ) return "RuntimeExCode(InvalidArgumentType, code: 5)";
        if ( v == swarmc::Errors::RuntimeExCode::TypeError ) return "RuntimeExCode(TypeError, code: 6)";
        if ( v == swarmc::Errors::RuntimeExCode::InvalidReferenceImplementation ) return "RuntimeExCode(InvalidReferenceImplementation, code: 7)";
        if ( v == swarmc::Errors::RuntimeExCode::DivisionByZero ) return "RuntimeExCode(DivisionByZero, code: 8)";
        if ( v == swarmc::Errors::RuntimeExCode::WhileCallbackTypeInvalid ) return "RuntimeExCode(WhileCallbackTypeInvalid, code: 9)";
        if ( v == swarmc::Errors::RuntimeExCode::WithCallbackTypeInvalid ) return "RuntimeExCode(WithCallbackTypeInvalid, code: 10)";
        if ( v == swarmc::Errors::RuntimeExCode::EnumIndexOutOfBounds ) return "RuntimeExCode(EnumIndexOutOfBounds, code: 11)";
        if ( v == swarmc::Errors::RuntimeExCode::EnumerateCallbackTypeInvalid ) return "RuntimeExCode(EnumerateCallbackTypeInvalid, code: 12)";
        if ( v == swarmc::Errors::RuntimeExCode::FnParamOutsideCall ) return "RuntimeExCode(FnParamOutsideCall, code: 13)";
        if ( v == swarmc::Errors::RuntimeExCode::ReturnOutsideCall ) return "RuntimeExCode(ReturnOutsideCall, code: 14)";
        if ( v == swarmc::Errors::RuntimeExCode::InvalidMapKey ) return "RuntimeExCode(InvalidMapKey, code: 15)";
        if ( v == swarmc::Errors::RuntimeExCode::InvalidAssignEval ) return "RuntimeExCode(InvalidAssignEval, code: 16)";
        if ( v == swarmc::Errors::RuntimeExCode::StreamNotOpen ) return "RuntimeExCode(StreamNotOpen, code: 17)";
        if ( v == swarmc::Errors::RuntimeExCode::StreamEmpty ) return "RuntimeExCode(StreamEmpty, code: 18)";
        if ( v == swarmc::Errors::RuntimeExCode::ResumeOutsideExHandler ) return "RuntimeExCode(ResumeOutsideExHandler, code: 19)";
        return "RuntimeExCode(UNKNOWN" + s((size_t) v) + ")";
    }

}

namespace swarmc::Errors {

    class RuntimeError : public SwarmError {
    public:
        RuntimeError(RuntimeExCode code, const std::string& message) :
            SwarmError("Runtime error: " + message + " (" + nslib::s(code) + ")"), _code(code) {}

        RuntimeExCode code() {
            return _code;
        }

    protected:
        RuntimeExCode _code;
    };

}

#endif
