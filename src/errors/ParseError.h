#ifndef SWARMC_PARSEERROR_H
#define SWARMC_PARSEERROR_H

#include "SwarmError.h"

namespace swarmc::Errors {

    class ParseError : public SwarmError {
    public:
        const int exitCode;

        ParseError(int inExitCode = 1) : SwarmError("Unable to parse input."), exitCode(inExitCode) {}
    };

}

#endif
