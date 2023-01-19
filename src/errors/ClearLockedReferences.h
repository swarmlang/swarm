#ifndef SWARM_CLEARLOCKEDREFERENCES_H
#define SWARM_CLEARLOCKEDREFERENCES_H

#include "SwarmError.h"

namespace swarmc::Errors {

    class ClearLockedReferences : public SwarmError {
    public:
        ClearLockedReferences() : SwarmError("Cannot clear storage containing locked references") {}
    };
}

#endif //SWARM_CLEARLOCKEDREFERENCES_H
