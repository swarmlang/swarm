#include <functional>
#include "../../shared/nslib.h"
#include "../walk/binary_const.h"
#include "../Wire.h"

using namespace nslib::serial;

namespace swarmc::Runtime {

    Factory<Type::Type, void*>* Wire::buildTypes() {
        auto factory = new Factory<Type::Type, void*>;
        auto common = [](const Type::Type* type, auto) -> binn* {
            auto binn = binn_map();
            binn_map_set_uint64(binn, BC_INTRINSIC, (std::size_t) type->intrinsic());
            binn_map_set_map(binn, BC_EXTRA, type->getExtraSerialData());
            return binn;
        };
        // used for preventing recursive object types from exploding the stack
        static auto CurrentObjectR = std::stack<std::size_t>();

        // Primitive types
        factory->registerReducer("Type::Primitive", common);
        factory->registerProducer("Type::Primitive", [](binn* obj, auto) {
            auto t = Type::Primitive::of(
                (Type::Intrinsic) binn_map_uint64(obj, BC_INTRINSIC)
            );
            t->loadExtraSerialData((binn*) binn_map_map(obj, BC_EXTRA));
            return t;
        });


        // Opaque types
        factory->registerReducer(s(Type::Intrinsic::OPAQUE), [common](const Type::Type* t, auto) {
            auto o = dynamic_cast<const Type::Opaque*>(t);
            auto binn = common(o, nullptr);
            binn_map_set_str(binn, BC_NAME, strdup(o->name().c_str()));
            return binn;
        });
        factory->registerProducer(s(Type::Intrinsic::OPAQUE), [](binn* obj, auto) {
            std::string name = binn_map_str(obj, BC_NAME);
            auto o = Type::Opaque::of(name);
            o->loadExtraSerialData((binn*) binn_map_map(obj, BC_EXTRA));
            return o;
        });


        // Ambiguous types
        factory->registerReducer(s(Type::Intrinsic::AMBIGUOUS), common);
        factory->registerProducer(s(Type::Intrinsic::AMBIGUOUS), [](binn* obj, auto) {
            auto t = Type::Ambiguous::of();
            t->loadExtraSerialData((binn*) binn_map_map(obj, BC_EXTRA));
            return t;
        });


        // Map types
        factory->registerReducer(s(Type::Intrinsic::MAP), [factory, common](const Type::Type* t, auto) {
            auto m = dynamic_cast<const Type::Map*>(t);
            auto binn = common(m, nullptr);
            binn_map_set_map(binn, BC_TYPE, factory->reduce(m->values(), nullptr));
            return binn;
        });
        factory->registerProducer(s(Type::Intrinsic::MAP), [factory](binn* obj, auto) {
            auto inner = factory->produce((binn*) binn_map_map(obj, BC_TYPE), nullptr);
            auto t = new Type::Map(inner);
            t->loadExtraSerialData((binn*) binn_map_map(obj, BC_EXTRA));
            return t;
        });


        // Enumerable types
        factory->registerReducer(s(Type::Intrinsic::ENUMERABLE), [factory, common](const Type::Type* t, auto) {
            auto e = dynamic_cast<const Type::Enumerable*>(t);
            auto binn = common(e, nullptr);
            binn_map_set_map(binn, BC_TYPE, factory->reduce(e->values(), nullptr));
            return binn;
        });
        factory->registerProducer(s(Type::Intrinsic::ENUMERABLE), [factory](binn* obj, auto) {
            auto inner = factory->produce((binn*) binn_map_map(obj, BC_TYPE), nullptr);
            auto t = new Type::Enumerable(inner);
            t->loadExtraSerialData((binn*) binn_map_map(obj, BC_EXTRA));
            return t;
        });


        // Resource types
        factory->registerReducer(s(Type::Intrinsic::RESOURCE), [factory, common](const Type::Type* t, auto) {
            auto e = dynamic_cast<const Type::Resource*>(t);
            auto binn = common(e, nullptr);
            binn_map_set_map(binn, BC_TYPE, factory->reduce(e->yields(), nullptr));
            return binn;
        });
        factory->registerProducer(s(Type::Intrinsic::RESOURCE), [factory](binn* obj, auto) {
            auto inner = factory->produce((binn*) binn_map_map(obj, BC_TYPE), nullptr);
            auto t = Type::Resource::of(inner);
            t->loadExtraSerialData((binn*) binn_map_map(obj, BC_EXTRA));
            return t;
        });


        // Stream types
        factory->registerReducer(s(Type::Intrinsic::STREAM), [factory, common](const Type::Type* t, auto) {
            auto e = dynamic_cast<const Type::Stream*>(t);
            auto binn = common(e, nullptr);
            binn_map_set_map(binn, BC_TYPE, factory->reduce(e->inner(), nullptr));
            return binn;
        });
        factory->registerProducer(s(Type::Intrinsic::STREAM), [factory](binn* obj, auto) {
            auto inner = factory->produce((binn*) binn_map_map(obj, BC_TYPE), nullptr);
            auto t = Type::Stream::of(inner);
            t->loadExtraSerialData((binn*) binn_map_map(obj, BC_EXTRA));
            return t;
        });


        // Object types
        // FIXME: also serialize OBJECT_PROTO??
        // FIXME: remove OTYPE intrinsic??
        factory->registerReducer(s(Type::Intrinsic::OBJECT), [factory, common](const Type::Type* t, auto) {
            auto o = dynamic_cast<const Type::Object*>(t);
            // replace recursive references with p:THIS
            if ( !CurrentObjectR.empty() ) {
                if (CurrentObjectR.top() == o->getId()) {
                    return common(Type::Primitive::of(Type::Intrinsic::THIS), nullptr);
                }
            }
            CurrentObjectR.push(o->getId());
            auto binn = common(o, nullptr);

            auto parent = o->getParent();
            auto propertyKeys = binn_list();
            auto propertyValues = binn_list();
            for ( const auto& pair : o->getProperties() ) {
                binn_list_add_str(propertyKeys, strdup(pair.first.c_str()));
                binn_list_add_map(propertyValues, factory->reduce(pair.second, nullptr));
            }

            binn_map_set_bool(binn, BC_FINAL, o->isFinal());
            binn_map_set_list(binn, BC_OTYPE_K, propertyKeys);
            binn_map_set_list(binn, BC_OTYPE_V, propertyValues);
            binn_map_set_bool(binn, BC_HAS_PARENT, parent != nullptr);

            if ( parent != nullptr ) {
                binn_map_set_map(binn, BC_PARENT, factory->reduce(parent, nullptr));
            }
            CurrentObjectR.pop();

            return binn;
        });
        factory->registerProducer(s(Type::Intrinsic::OBJECT), [factory](binn* obj, auto) -> Type::Type* {
            auto intrinsic = (Type::Intrinsic) binn_map_uint64(obj, BC_INTRINSIC);
            // `THIS` will appear in recursive types during serialization
            // Since we are reconstructing the type, all recursive references
            // should stay as `THIS` until o->finalize() is called
            if ( intrinsic == Type::Intrinsic::THIS ) {
                return Type::Primitive::of(Type::Intrinsic::THIS);
            }
            auto isFinal = binn_map_bool(obj, BC_FINAL);
            auto binnPropertyKeys = binn_map_list(obj, BC_OTYPE_K);
            auto binnPropertyValues = binn_map_list(obj, BC_OTYPE_V);

            Type::Object* parent = nullptr;
            if ( binn_map_bool(obj, BC_HAS_PARENT) ) {
                parent = dynamic_cast<Type::Object*>(factory->produce((binn*) binn_map_map(obj, BC_PARENT), nullptr));
            }

            auto type = new Type::Object(parent);
            std::vector<std::string> propertyKeys;
            std::vector<Type::Type*> propertyValues;

            binn_iter iter;
            binn binnPropertyValue;
            binn_list_foreach(binnPropertyValues, binnPropertyValue) {
                propertyValues.push_back(factory->produce(&binnPropertyValue, nullptr));
            }

            binn binnPropertyKey;
            binn_list_foreach(binnPropertyKeys, binnPropertyKey) {
                std::string key = binn_get_str(&binnPropertyKey);
                propertyKeys.push_back(key);
            }

            assert(propertyValues.size() == propertyKeys.size());
            for ( std::size_t i = 0; i < propertyValues.size(); i += 1 ) {
                type->defineProperty(propertyKeys[i], propertyValues[i]);
            }

            if ( isFinal ) {
                type = type->finalize();
            }

            return type;
        });


        // Lambda types
        auto lambdaReducer = [factory, common](const Type::Type* t, auto) {
            auto l = dynamic_cast<const Type::Lambda*>(t);
            auto binn = common(l, nullptr);

            auto params = binn_list();
            for ( auto param : l->params() ) {
                binn_list_add_map(params, factory->reduce(param, nullptr));
            }

            binn_map_set_map(binn, BC_RETURNS, factory->reduce(l->returns(), nullptr));
            binn_map_set_list(binn, BC_PARAMS, params);
            return binn;
        };
        auto lambdaProducer = [factory](binn* obj, auto) {
            Type::Type* l = factory->produce((binn*) binn_map_map(obj, BC_RETURNS), nullptr);

            binn* params = (binn*) binn_map_list(obj, BC_PARAMS);
            binn_iter iter;
            binn param;

            binn_list_foreach(params, param) {
                l = new Type::Lambda1(factory->produce(&param, nullptr), l);
            }

            // there were no params
            if ( l->intrinsic() != Type::Intrinsic::LAMBDA1 ) {
                l = new Type::Lambda0(l);
            }

            l->loadExtraSerialData((binn*) binn_map_map(obj, BC_EXTRA));
            return l;
        };
        factory->registerReducer(s(Type::Intrinsic::LAMBDA0), lambdaReducer);
        factory->registerProducer(s(Type::Intrinsic::LAMBDA0), lambdaProducer);
        factory->registerReducer(s(Type::Intrinsic::LAMBDA1), lambdaReducer);
        factory->registerProducer(s(Type::Intrinsic::LAMBDA1), lambdaProducer);

        Framework::onShutdown([factory]() {
            delete factory;
        });
        return factory;
    }

}
