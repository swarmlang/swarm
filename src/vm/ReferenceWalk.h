#ifndef SWARMVM_REFERENCEWALK
#define SWARMVM_REFERENCEWALK

#include "../shared/nslib.h"
#include "../errors/SwarmError.h"
#include "ISA.h"

using namespace nslib;

namespace swarmc::ISA {

    /** Base class for walks which perform an operation over ISA::Reference instances. */
    template <typename TReturn>
    class ReferenceWalk : public IStringable, public IUsesConsole {
    public:
        ReferenceWalk() : IUsesConsole() {}
        ~ReferenceWalk() override = default;

        virtual TReturn walk(Reference* ref) {
            if ( ref->tag() == ReferenceTag::LOCATION ) return walkLocationReference((LocationReference*) ref);
            if ( ref->tag() == ReferenceTag::TYPE ) return walkTypeReference((TypeReference*) ref);
            if ( ref->tag() == ReferenceTag::STRING ) return walkStringReference((StringReference*) ref);
            if ( ref->tag() == ReferenceTag::NUMBER ) return walkNumberReference((NumberReference*) ref);
            if ( ref->tag() == ReferenceTag::BOOLEAN ) return walkBooleanReference((BooleanReference*) ref);
            if ( ref->tag() == ReferenceTag::FUNCTION ) return walkFunctionReference((FunctionReference*) ref);
            if ( ref->tag() == ReferenceTag::STREAM ) return walkStreamReference((StreamReference*) ref);

            throw Errors::SwarmError("Unknown reference tag: " + ref->toString());
        }

        // FIXME: need to add the enumeration/map/other new reference types.
    protected:
        virtual TReturn walkLocationReference(LocationReference*) = 0;
        virtual TReturn walkTypeReference(TypeReference*) = 0;
        virtual TReturn walkStringReference(StringReference*) = 0;
        virtual TReturn walkNumberReference(NumberReference*) = 0;
        virtual TReturn walkBooleanReference(BooleanReference*) = 0;
        virtual TReturn walkFunctionReference(FunctionReference*) = 0;
        virtual TReturn walkStreamReference(StreamReference*) = 0;
    };

}

#endif //SWARMVM_REFERENCEWALK
