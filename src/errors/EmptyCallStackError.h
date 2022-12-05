#ifndef SWARMC_EMPTYCALLSTACKERROR_H
#define SWARMC_EMPTYCALLSTACKERROR_H

#include "SwarmError.h"

namespace swarmc::Errors {

    class EmptyCallStackError : public SwarmError {
    public:
        EmptyCallStackError() : SwarmError("Cannot make return jump: the call stack is empty") {}
    };

}

#endif
