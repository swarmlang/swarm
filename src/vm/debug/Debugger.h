#ifndef SWARMVM_DEBUGGER
#define SWARMVM_DEBUGGER

#include "../../shared/IStringable.h"
#include "Metadata.h"

namespace swarmc::Runtime::Debug {

    class Debugger : public IStringable {

        Metadata* meta() const { return _meta; }

        void setMetadata(Metadata* meta) { _meta = meta; }

    protected:
        Metadata* _meta;
    };

}

#endif //SWARMVM_DEBUGGER
