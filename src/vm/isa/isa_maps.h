#ifndef SWARMVM_MAPS
#define SWARMVM_MAPS

#include "../ISA.h"

namespace swarmc::ISA {

    class MapInit : public UnaryInstruction<Reference> {
    public:
        explicit MapInit(Reference* type) :
            UnaryInstruction<Reference>(Tag::MAPINIT, type) {}
        [[nodiscard]] MapInit* copy() const override {
            return new MapInit(_first->copy());
        }
    };

    class MapSet : public TrinaryInstruction<Reference, Reference, LocationReference> {
    public:
        MapSet(Reference* key, Reference* value, LocationReference* map) :
            TrinaryInstruction<Reference, Reference, LocationReference>(Tag::MAPSET, key, value, map) {}
        [[nodiscard]] MapSet* copy() const override {
            return new MapSet(_first->copy(), _second->copy(), _third->copy());
        }
    };

    class MapGet : public BinaryInstruction<Reference, LocationReference> {
    public:
        MapGet(Reference* key, LocationReference* map) :
            BinaryInstruction<Reference, LocationReference>(Tag::MAPGET, key, map) {}
        [[nodiscard]] MapGet* copy() const override {
            return new MapGet(_first->copy(), _second->copy());
        }
    };

    class MapLength : public UnaryInstruction<LocationReference> {
    public:
        explicit MapLength(LocationReference* map) :
            UnaryInstruction<LocationReference>(Tag::MAPLENGTH, map) {}
        [[nodiscard]] MapLength* copy() const override {
            return new MapLength(_first->copy());
        }
    };

    class MapKeys : public UnaryInstruction<LocationReference> {
    public:
        explicit MapKeys(LocationReference* map) :
            UnaryInstruction<LocationReference>(Tag::MAPKEYS, map) {}
        [[nodiscard]] MapKeys* copy() const override {
            return new MapKeys(_first->copy());
        }
    };

}

#endif //SWARMVM_MAPS
