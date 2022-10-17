#ifndef SWARMVM_MAPS
#define SWARMVM_MAPS

#include "../ISA.h"

namespace swarmc::ISA {

    class MapInit : public UnaryInstruction<Reference> {
    public:
        MapInit(Reference* type) :
            UnaryInstruction<Reference>(Tag::MAPINIT, type) {}
    };

    class MapSet : public TrinaryInstruction<Reference, Reference, LocationReference> {
    public:
        MapSet(Reference* key, Reference* value, LocationReference* map) :
            TrinaryInstruction<Reference, Reference, LocationReference>(Tag::MAPSET, key, value, map) {}
    };

    class MapGet : public BinaryInstruction<Reference, LocationReference> {
    public:
        MapGet(Reference* key, LocationReference* map) :
            BinaryInstruction<Reference, LocationReference>(Tag::MAPGET, key, map) {}
    };

    class MapLength : public UnaryInstruction<LocationReference> {
    public:
        MapLength(LocationReference* map) :
            UnaryInstruction<LocationReference>(Tag::MAPLENGTH, map) {}
    };

    class MapKeys : public UnaryInstruction<LocationReference> {
    public:
        MapKeys(LocationReference* map) :
            UnaryInstruction<LocationReference>(Tag::MAPKEYS, map) {}
    };

}

#endif //SWARMVM_MAPS
