#ifndef SWARMVM_INTERFACES_
#define SWARMVM_INTERFACES_

#include "../../shared/IStringable.h"

namespace swarmc::Runtime {

    class IStorageInterface : public IStringable {
    public:
        virtual ~IStorageInterface() = default;


    };

}

#endif //SWARMVM_INTERFACES_
