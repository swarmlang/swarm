#ifndef SWARMVM_MAPS
#define SWARMVM_MAPS

#include "../ISA.h"

namespace swarmc::ISA {

    class MapInit : public UnaryInstruction<Reference> {
    public:
        explicit MapInit(Reference* type) :
            UnaryInstruction<Reference>(Tag::MAPINIT, useref(type)) {}
        ~MapInit() override { freeref(_first); }
        [[nodiscard]] MapInit* copy() const override {
            return new MapInit(_first->copy());
        }
    };

    class MapSet : public TrinaryInstruction<Reference, Reference, LocationReference> {
    public:
        MapSet(Reference* key, Reference* value, LocationReference* map) :
            TrinaryInstruction<Reference, Reference, LocationReference>(Tag::MAPSET, useref(key), useref(value), useref(map)) {}
        ~MapSet() override {
            freeref(_first);
            freeref(_second);
            freeref(_third);
        }
        [[nodiscard]] MapSet* copy() const override {
            return new MapSet(_first->copy(), _second->copy(), _third->copy());
        }
    };

    class MapGet : public BinaryInstruction<Reference, LocationReference> {
    public:
        MapGet(Reference* key, LocationReference* map) :
            BinaryInstruction<Reference, LocationReference>(Tag::MAPGET, useref(key), useref(map)) {}
        ~MapGet() override {
            freeref(_first);
            freeref(_second);
        }
        [[nodiscard]] MapGet* copy() const override {
            return new MapGet(_first->copy(), _second->copy());
        }
    };

    class MapLength : public UnaryInstruction<LocationReference> {
    public:
        explicit MapLength(LocationReference* map) :
            UnaryInstruction<LocationReference>(Tag::MAPLENGTH, useref(map)) {}
        ~MapLength() override { freeref(_first); }
        [[nodiscard]] MapLength* copy() const override {
            return new MapLength(_first->copy());
        }
    };

    class MapKeys : public UnaryInstruction<LocationReference> {
    public:
        explicit MapKeys(LocationReference* map) :
            UnaryInstruction<LocationReference>(Tag::MAPKEYS, useref(map)) {}
        ~MapKeys() override { freeref(_first); }
        [[nodiscard]] MapKeys* copy() const override {
            return new MapKeys(_first->copy());
        }
    };

}

#endif //SWARMVM_MAPS
