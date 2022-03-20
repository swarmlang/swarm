#ifndef SWARMC_QUEUEEXECUTIONERROR_H
#define SWARMC_QUEUEEXECUTIONERROR_H

#include "SwarmError.h"

namespace swarmc {
namespace Errors {

    class QueueExecutionError : public SwarmError {
    public:
        QueueExecutionError(std::string msg) : SwarmError("Exception during queue execution: " + msg) {}
    };

}
}

#endif
