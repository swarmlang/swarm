#ifndef SWARMC_FREESYMBOLERROR_H
#define SWARMC_FREESYMBOLERROR_H

#include "SwarmError.h"

namespace swarmc::Errors {

    class FreeSymbolError : public SwarmError {
    public:
        FreeSymbolError(std::string name) : SwarmError("Tried to retrieve value of unbound symbol: " + name) {}
    };

}

#endif
