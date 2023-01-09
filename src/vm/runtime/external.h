#ifndef SWARM_EXTERNAL_H
#define SWARM_EXTERNAL_H

#include "interfaces.h"
#include "runtime_provider.h"
#include "../../lang/Type.h"
#include "../isa_meta.h"

namespace swarmc::Runtime {

    class ProviderModule {
    public:
        [[nodiscard]] virtual IProvider* build(IGlobalServices*) = 0;
    };

}

#endif //SWARM_EXTERNAL_H
