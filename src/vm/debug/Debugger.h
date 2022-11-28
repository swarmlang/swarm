#ifndef SWARMVM_DEBUGGER
#define SWARMVM_DEBUGGER

#include "../../shared/nslib.h"
#include "Metadata.h"

using namespace nslib;

namespace swarmc::Runtime::Debug {

    class Debugger : public IStringable {

        [[nodiscard]] Metadata* meta() const { return _meta; }

        void setMetadata(Metadata* meta) { _meta = meta; }

    protected:
        Metadata* _meta;
    };

}

#endif //SWARMVM_DEBUGGER
