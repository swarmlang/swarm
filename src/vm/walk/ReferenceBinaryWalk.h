#ifndef SWARMVM_REFERENCEBINARYWALK
#define SWARMVM_REFERENCEBINARYWALK

#include <cstring>
#include "../../../mod/binn/src/binn.h"
#include "../../shared/util_string_helpers.h"
#include "../ReferenceWalk.h"
#include "binary_const.h"

namespace swarmc::ISA {

    /**
     * Runtime reference walk which writes reference values as SVI code.
     */
    class ReferenceBinaryWalk : public ReferenceWalk<binn*> {
    public:
        std::string toString() const {
            return "ReferenceBinaryWalk<>";
        }

    protected:
        binn* walkLocationReference(LocationReference* ref) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (size_t) ref->tag());
            binn_map_set_uint64(obj, BC_AFFINITY, (size_t) ref->affinity());
            binn_map_set_str(obj, BC_NAME, strdup(ref->name().c_str()));
            return obj;
        }

        binn* walkTypeReference(TypeReference* ref) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (size_t) ref->tag());
            binn_map_set_map(obj, BC_TYPE, walkType(ref->value()));
            return obj;
        }

        binn* walkFunctionReference(FunctionReference* ref) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (size_t) ref->tag());
            binn_map_set_uint64(obj, BC_BACKEND, (size_t) ref->fn()->backend());
            binn_map_set_str(obj, BC_NAME, strdup(ref->fn()->name().c_str()));

            auto params = binn_list();
            for ( auto p : ref->fn()->getCallVector() ) {
                auto param = binn_map();
                binn_map_set_map(param, BC_VALUE, walk(p.second));
                binn_list_add_map(params, param);
            }
            binn_map_set_list(obj, BC_PARAMS, params);

            return obj;
        }

        binn* walkStreamReference(StreamReference* ref) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (size_t) ref->tag());
            binn_map_set_str(obj, BC_ID, strdup(ref->stream()->id().c_str()));
            binn_map_set_map(obj, BC_TYPE, walkType(ref->stream()->innerType()));
            return obj;
        }

        binn* walkStringReference(StringReference* ref) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (size_t) ref->tag());
            binn_map_set_str(obj, BC_VALUE, strdup(ref->value().c_str()));
            return obj;
        }

        binn* walkNumberReference(NumberReference* ref) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (size_t) ref->tag());
            binn_map_set_double(obj, BC_VALUE, ref->value());
            return obj;
        }

        binn* walkBooleanReference(BooleanReference* ref) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (size_t) ref->tag());
            binn_map_set_bool(obj, BC_VALUE, ref->value());
            return obj;
        }

        binn* walkType(const Type::Type* type) {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_INTRINSIC, (size_t) type->intrinsic());

            if ( Type::Primitive::isPrimitive(type->intrinsic()) )
                return walkPrimitiveType(obj, (Type::Primitive*) type);

            if ( type->intrinsic() == Type::Intrinsic::AMBIGUOUS )
                return walkAmbiguousType(obj, (Type::Ambiguous*) type);

            if ( type->intrinsic() == Type::Intrinsic::MAP )
                return walkMapType(obj, (Type::Map*) type);

            if ( type->intrinsic() == Type::Intrinsic::ENUMERABLE )
                return walkEnumerableType(obj, (Type::Enumerable*) type);

            if ( type->intrinsic() == Type::Intrinsic::RESOURCE )
                return walkResourceType(obj, (Type::Resource*) type);

            if ( type->intrinsic() == Type::Intrinsic::STREAM )
                return walkStreamType(obj, (Type::Stream*) type);

            if ( type->intrinsic() == Type::Intrinsic::LAMBDA0 || type->intrinsic() == Type::Intrinsic::LAMBDA1 )
                return walkLambdaType(obj, (Type::Lambda*) type);

            throw Errors::SwarmError("Cannot serialize unknown or invalid intrinsic type: " + type->toString());
        }

        binn* walkPrimitiveType(binn* obj, const Type::Primitive* type) {
            return obj;
        }

        binn* walkAmbiguousType(binn* obj, const Type::Ambiguous* type) {
            return obj;
        }

        binn* walkMapType(binn* obj, const Type::Map* type) {
            binn_map_set_map(obj, BC_TYPE, walkType(type->values()));
            return obj;
        }

        binn* walkEnumerableType(binn* obj, const Type::Enumerable* type) {
            binn_map_set_map(obj, BC_TYPE, walkType(type->values()));
            return obj;
        }

        binn* walkResourceType(binn* obj, const Type::Resource* type) {
            binn_map_set_map(obj, BC_TYPE, walkType(type->yields()));
            return obj;
        }

        binn* walkStreamType(binn* obj, const Type::Stream* type) {
            binn_map_set_map(obj, BC_TYPE, walkType(type->inner()));
            return obj;
        }

        binn* walkLambdaType(binn* obj, const Type::Lambda* type) {
            auto params = binn_list();
            for ( auto param : type->params() ) {
                binn_list_add_map(params, walkType(param));
            }

            binn_map_set_map(obj, BC_RETURNS, walkType(type->returns()));
            binn_map_set_list(obj, BC_PARAMS, params);
            return obj;
        }
    };

}

#endif //SWARMVM_REFERENCEBINARYWALK
