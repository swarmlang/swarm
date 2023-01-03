#ifndef SWARMVM_BINARYREFERENCEWALK
#define SWARMVM_BINARYREFERENCEWALK

#include "../../../mod/binn/src/binn.h"
#include "../isa_meta.h"
#include "binary_const.h"

namespace swarmc::Runtime {
    class VirtualMachine;
}

namespace swarmc::ISA {

    /**
     * Runtime reference walk which writes reference values as SVI code.
     */
    class BinaryReferenceWalk : public IStringable {
    public:
        explicit BinaryReferenceWalk(Runtime::VirtualMachine* vm = nullptr) : _vm(vm) {}

        [[nodiscard]] std::string toString() const override {
            return "BinaryReferenceWalk<>";
        }

        [[nodiscard]] Reference* walk(binn* obj) {
            auto tag = (ReferenceTag) binn_map_uint64(obj, BC_TAG);

            if ( tag == ReferenceTag::LOCATION ) return walkLocationReference(obj);
            if ( tag == ReferenceTag::TYPE ) return walkTypeReference(obj);
            if ( tag == ReferenceTag::FUNCTION ) return walkFunctionReference(obj);
            if ( tag == ReferenceTag::STREAM ) return walkStreamReference(obj);
            if ( tag == ReferenceTag::STRING ) return walkStringReference(obj);
            if ( tag == ReferenceTag::NUMBER ) return walkNumberReference(obj);
            if ( tag == ReferenceTag::BOOLEAN ) return walkBooleanReference(obj);

            throw Errors::SwarmError("Unable to deserialize binary reference with unknown or invalid reference tag.");
        }

    protected:
        Runtime::VirtualMachine* _vm;

        LocationReference* walkLocationReference(binn* obj) {
            return new LocationReference((Affinity) binn_map_uint64(obj, BC_AFFINITY), binn_map_str(obj, BC_NAME));
        }

        TypeReference* walkTypeReference(binn* obj) {
            return new TypeReference(walkType((binn*) binn_map_map(obj, BC_TYPE)));
        }

        FunctionReference* walkFunctionReference(binn* obj) {
            auto backend = binn_map_uint64(obj, BC_BACKEND);
            auto name = binn_map_str(obj, BC_NAME);
            std::vector<Reference*> params;

            auto list = binn_map_list(obj, BC_PARAMS);
            binn_iter iter;
            binn value;
            binn_list_foreach(list, value) {
                params.push_back(walk(&value));
            }

            return walkFunctionReference(backend, name, params);
        }

        FunctionReference* walkFunctionReference(std::size_t backend, const std::string& name, std::vector<Reference*> params);

        StreamReference* walkStreamReference(binn* obj) {
            auto id = binn_map_str(obj, BC_ID);
            auto type = walkType((binn*) binn_map_map(obj, BC_TYPE));
            return walkStreamReference(id, type);
        }

        StreamReference* walkStreamReference(const std::string& id, const Type::Type* type);

        StringReference* walkStringReference(binn* obj) {
            return new StringReference(binn_map_str(obj, BC_VALUE));
        }

        NumberReference* walkNumberReference(binn* obj) {
            return new NumberReference(binn_map_double(obj, BC_VALUE));
        }

        BooleanReference* walkBooleanReference(binn* obj) {
            return new BooleanReference(binn_map_bool(obj, BC_VALUE));
        }

        Type::Type* walkType(binn* obj) {
            auto intrinsic = (Type::Intrinsic) binn_map_uint64(obj, BC_INTRINSIC);

            if ( Type::Primitive::isPrimitive(intrinsic) )
                return walkPrimitiveType(obj);

            if ( intrinsic == Type::Intrinsic::AMBIGUOUS )
                return walkAmbiguousType(obj);

            if ( intrinsic == Type::Intrinsic::MAP )
                return walkMapType(obj);

            if ( intrinsic == Type::Intrinsic::ENUMERABLE )
                return walkEnumerableType(obj);

            if ( intrinsic == Type::Intrinsic::RESOURCE )
                return walkResourceType(obj);

            if ( intrinsic == Type::Intrinsic::STREAM )
                return walkStreamType(obj);

            if ( intrinsic == Type::Intrinsic::LAMBDA0 || intrinsic == Type::Intrinsic::LAMBDA1 )
                return walkLambdaType(obj);

            throw Errors::SwarmError("Cannot de-serialize unknown or invalid intrinsic type.");
        }

        Type::Primitive* walkPrimitiveType(binn* obj) {
            return Type::Primitive::of((Type::Intrinsic) binn_map_uint64(obj, BC_INTRINSIC));
        }

        Type::Ambiguous* walkAmbiguousType(binn*) {
            return new Type::Ambiguous;
        }

        Type::Map* walkMapType(binn* obj) {
            return new Type::Map(walkType((binn*) binn_map_map(obj, BC_TYPE)));  // fixme?
        }

        Type::Enumerable* walkEnumerableType(binn* obj) {
            return new Type::Enumerable(walkType((binn*) binn_map_map(obj, BC_TYPE)));  // fixme?
        }

        Type::Resource* walkResourceType(binn* obj) {
            return Type::Resource::of(walkType((binn*) binn_map_map(obj, BC_TYPE)));
        }

        Type::Stream* walkStreamType(binn* obj) {
            return Type::Stream::of(walkType((binn*) binn_map_map(obj, BC_TYPE)));
        }

        Type::Lambda* walkLambdaType(binn* obj) {
            Type::Lambda* l = new Type::Lambda0(walkType((binn*) binn_map_map(obj, BC_RETURNS)));

            binn* params = (binn*) binn_map_list(obj, BC_PARAMS);
            binn_iter iter;
            binn param;

            binn_list_foreach(params, param) {
                l = new Type::Lambda1(walkType(&param), l);
            }

            return l;
        }
    };

}

#endif //SWARMVM_BINARYREFERENCEWALK
