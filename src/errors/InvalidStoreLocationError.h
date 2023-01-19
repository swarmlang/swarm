#ifndef SWARM_INVALIDSTORELOCATIONERROR_H
#define SWARM_INVALIDSTORELOCATIONERROR_H

#include "SwarmError.h"

namespace swarmc::Errors {

    class InvalidStoreLocationError : public SwarmError {
    public:
        InvalidStoreLocationError(const std::string& storeLocName, const std::string& storeName)
            : SwarmError("Attempted to load undefined location (" + storeLocName + ") from store (" + storeName + ")") {}
    };

}

#endif //SWARM_INVALIDSTORELOCATIONERROR_H
