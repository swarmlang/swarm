#include <functional>
#include "../../shared/nslib.h"
#include "../walk/binary_const.h"
#include "../Wire.h"
#include "../isa_meta.h"
#include "../VirtualMachine.h"

using namespace nslib::serial;
using namespace swarmc::ISA;

namespace swarmc::Runtime {

    Factory<Reference, VirtualMachine*>* Wire::buildReferences() {
        auto factory = new Factory<Reference, VirtualMachine*>;

        // Location references
        factory->registerReducer(s(ReferenceTag::LOCATION), [](const Reference* baseRef, VirtualMachine*) {
            auto ref = dynamic_cast<const LocationReference*>(baseRef);
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (std::size_t) ref->tag());
            binn_map_set_uint64(obj, BC_AFFINITY, (std::size_t) ref->affinity());
            binn_map_set_str(obj, BC_NAME, strdup(ref->name().c_str()));
            binn_map_set_map(obj, BC_EXTRA, ref->getExtraSerialData());
            return obj;
        });
        factory->registerProducer(s(ReferenceTag::LOCATION), [](binn* obj, VirtualMachine*) {
            auto affinity = (Affinity) binn_map_uint64(obj, BC_AFFINITY);
            std::string name = binn_map_str(obj, BC_NAME);
            auto ref = new LocationReference(affinity, name);
            ref->loadExtraSerialData((binn*) binn_map_map(obj, BC_EXTRA));
            return ref;
        });


        // Type references
        factory->registerReducer(s(ReferenceTag::TYPE), [](const Reference* baseRef, VirtualMachine*) {
            auto ref = dynamic_cast<const TypeReference*>(baseRef);
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (std::size_t) ref->tag());
            binn_map_set_map(obj, BC_TYPE, types()->reduce(ref->value(), nullptr));
            binn_map_set_map(obj, BC_EXTRA, ref->getExtraSerialData());
            return obj;
        });
        factory->registerProducer(s(ReferenceTag::TYPE), [](binn* obj, VirtualMachine*) {
            auto type = types()->produce((binn*) binn_map_map(obj, BC_TYPE), nullptr);
            auto ref = new TypeReference(type);
            ref->loadExtraSerialData((binn*) binn_map_map(obj, BC_EXTRA));
            return ref;
        });


        // ObjectType references
        factory->registerReducer(s(ReferenceTag::OTYPE), [](const Reference* baseRef, VirtualMachine*) {
            auto ref = dynamic_cast<const ObjectTypeReference*>(baseRef);
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (std::size_t) ref->tag());
            binn_map_set_map(obj, BC_TYPE, types()->reduce(ref->value(), nullptr));
            binn_map_set_map(obj, BC_EXTRA, ref->getExtraSerialData());
            return obj;
        });
        factory->registerProducer(s(ReferenceTag::OTYPE), [](binn* obj, VirtualMachine*) {
            auto baseType = types()->produce((binn*) binn_map_map(obj, BC_TYPE), nullptr);
            assert(baseType->intrinsic() == Type::Intrinsic::OBJECT);
            auto type = dynamic_cast<Type::Object*>(baseType);
            auto ref = new ObjectTypeReference(type);
            ref->loadExtraSerialData((binn*) binn_map_map(obj, BC_EXTRA));
            return ref;
        });


        // Object references
        factory->registerReducer(s(ReferenceTag::OBJECT), [factory](const Reference* baseRef, VirtualMachine* vm) {
            auto ref = dynamic_cast<const ObjectReference*>(baseRef);
            auto obj = binn_map();

            auto propertyKeys = binn_list();
            auto propertyValues = binn_list();
            for ( const auto& p : ref->getProperties() ) {
                binn_list_add_str(propertyKeys, strdup(p.first.c_str()));
                binn_list_add_map(propertyValues, factory->reduce(p.second, vm));
            }

            binn_map_set_uint64(obj, BC_TAG, (std::size_t) ref->tag());
            binn_map_set_map(obj, BC_TYPE, types()->reduce(ref->type(), nullptr));
            binn_map_set_map(obj, BC_EXTRA, ref->getExtraSerialData());
            binn_map_set_bool(obj, BC_FINAL, ref->isFinal());
            binn_map_set_list(obj, BC_OTYPE_K, propertyKeys);
            binn_map_set_list(obj, BC_OTYPE_V, propertyValues);

            return obj;
        });
        factory->registerProducer(s(ReferenceTag::OBJECT), [factory](binn* obj, VirtualMachine*) {
            auto type = types()->produce((binn*) binn_map_map(obj, BC_TYPE), nullptr);
            assert(type->intrinsic() == Type::Intrinsic::OBJECT);

            auto otype = dynamic_cast<Type::Object*>(type);
            auto inst = new ObjectReference(otype);

            auto propertyKeys = binn_map_list(obj, BC_OTYPE_K);
            auto propertyValues = binn_map_list(obj, BC_OTYPE_V);
            binn_iter iter;
            binn binnPropertyValue;
            std::vector<std::string> keys;
            binn_list_foreach(propertyKeys, binnPropertyValue) {
                std::string bstr(binn_get_str(&binnPropertyValue));
                keys.push_back(bstr);
            }
            int i = -1;
            binn_list_foreach(propertyValues, binnPropertyValue) {
                i += 1;
                auto value = factory->produce(&binnPropertyValue, nullptr);
                inst->setProperty(keys.at(i), value);
            }

            if ( binn_map_bool(obj, BC_FINAL) ) {
                inst = inst->finalize();
            }
            inst->loadExtraSerialData((binn*) binn_map_map(obj, BC_EXTRA));
            return inst;
        });


        // Enumerable references
        factory->registerReducer(s(ReferenceTag::ENUMERATION), [factory](const Reference* baseRef, VirtualMachine* vm) {
            auto ref = dynamic_cast<const EnumerationReference*>(baseRef);
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (std::size_t) ref->tag());

            auto values = binn_list();
            for ( auto i = 0; i < ref->length(); i++ ) {
                binn_list_add_map(values, factory->reduce(ref->get(i), vm));
            }

            auto innerType = ((Type::Enumerable*)ref->type())->values();
            binn_map_set_map(obj, BC_VECTOR_TYPES, types()->reduce(innerType, vm));
            binn_map_set_list(obj, BC_VECTOR_VALUES, values);
            binn_map_set_map(obj, BC_EXTRA, ref->getExtraSerialData());

            return obj;
        });
        factory->registerProducer(s(ReferenceTag::ENUMERATION), [factory](binn* obj, VirtualMachine* vm) {
            auto typebin = (binn*)binn_map_map(obj, BC_VECTOR_TYPES);
            auto valuesbin = (binn*)binn_map_list(obj, BC_VECTOR_VALUES);

            auto type = types()->produce(typebin, vm);

            auto ref = new EnumerationReference(type);

            binn_iter iter;
            binn value;
            binn_list_foreach(valuesbin, value) {
                ref->append(factory->produce(&value, vm));
            }

            ref->loadExtraSerialData((binn*) binn_map_map(obj, BC_EXTRA));
            return ref;
        });


        // Map references
        factory->registerReducer(s(ReferenceTag::MAP), [factory](const Reference* baseRef, VirtualMachine* vm) {
            auto ref = dynamic_cast<const MapReference*>(baseRef);
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (std::size_t) ref->tag());

            auto keys = ref->keys();
            auto binnkeys = binn_list();
            auto binnvals = binn_list();
            for ( auto i = 0; i < keys->length(); i++ ) {
                auto key = dynamic_cast<const StringReference*>(keys->get(i))->value();
                binn_list_add_str(binnkeys, strdup(key.c_str()));
                binn_list_add_map(binnvals, factory->reduce(ref->get(key), vm));
            }

            binn_map_set_map(obj, BC_TYPE, types()->reduce(ref->type()->values(), vm));
            binn_map_set_list(obj, BC_VECTOR_TYPES, binnkeys);
            binn_map_set_list(obj, BC_VECTOR_VALUES, binnvals);
            binn_map_set_map(obj, BC_EXTRA, ref->getExtraSerialData());

            return obj;
        });
        factory->registerProducer(s(ReferenceTag::MAP), [factory](binn* obj, VirtualMachine* vm) {
            auto typebin = (binn*)binn_map_map(obj, BC_TYPE);
            auto keysbin = (binn*)binn_map_list(obj, BC_VECTOR_TYPES);
            auto valuesbin = (binn*)binn_map_list(obj, BC_VECTOR_VALUES);

            auto ref = new MapReference(types()->produce(typebin, vm));

            binn_iter iter;
            binn value;
            std::vector<std::string> keys;
            binn_list_foreach(keysbin, value) {
                std::string key(binn_get_str(&value));
                keys.push_back(key);

            }
            auto i = 0;
            binn_list_foreach(valuesbin, value) {
                ref->set(keys.at(i), factory->produce(&value, vm));
                i++;
            }

            ref->loadExtraSerialData((binn*) binn_map_map(obj, BC_EXTRA));
            return ref;
        });

        // Function references
        factory->registerReducer(s(ReferenceTag::FUNCTION), [factory](const Reference* baseRef, VirtualMachine* vm) {
            auto ref = dynamic_cast<const FunctionReference*>(baseRef);
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (std::size_t) ref->tag());
            binn_map_set_uint64(obj, BC_BACKEND, (std::size_t) ref->fn()->backend());
            binn_map_set_str(obj, BC_NAME, strdup(ref->fn()->name().c_str()));

            // params is ({ BC_VALUE : Wire reduction }) []
            auto params = binn_list();
            for ( auto p : ref->fn()->getCallVector() ) {
                // param is { BC_VALUE : Wire reduction }
                auto param = binn_map();
                binn_map_set_map(param, BC_VALUE, factory->reduce(p.second, vm));
                binn_list_add_map(params, param);
            }
            binn_map_set_list(obj, BC_PARAMS, params);
            binn_map_set_map(obj, BC_EXTRA, ref->getExtraSerialData());

            return obj;
        });
        factory->registerProducer(s(ReferenceTag::FUNCTION), [factory](binn* obj, VirtualMachine* vm) {
            auto backend = binn_map_uint64(obj, BC_BACKEND);
            auto name = binn_map_str(obj, BC_NAME);
            std::vector<Reference*> params;

            // list is ({ BC_VALUE : Wire reduction })[]
            auto list = binn_map_list(obj, BC_PARAMS);
            binn_iter iter;
            binn value;
            binn_list_foreach(list, value) {
                // value is { BC_VALUE : Wire reduction }
                auto reducedValue = (binn*) binn_map_map(&value, BC_VALUE);
                params.push_back(factory->produce(reducedValue, vm));
            }

            auto ref = vm->loadFunction((Runtime::FunctionBackend) backend, name);
            auto fn = ref->fn();
            for ( auto p : params ) fn = fn->curry(p);

            auto curriedRef = new FunctionReference(fn);
            curriedRef->loadExtraSerialData((binn*) binn_map_map(obj, BC_EXTRA));

            delete ref;
            return curriedRef;
        });


        // Stream references
        factory->registerReducer(s(ReferenceTag::STREAM), [](const Reference* baseRef, VirtualMachine*) {
            auto ref = dynamic_cast<const StreamReference*>(baseRef);
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (std::size_t) ref->tag());
            binn_map_set_str(obj, BC_ID, strdup(ref->stream()->id().c_str()));
            binn_map_set_map(obj, BC_TYPE, types()->reduce(ref->stream()->innerType(), nullptr));
            binn_map_set_map(obj, BC_EXTRA, ref->getExtraSerialData());
            return obj;
        });
        factory->registerProducer(s(ReferenceTag::STREAM), [](binn* obj, VirtualMachine* vm) {
            auto id = binn_map_str(obj, BC_ID);
            auto type = types()->produce((binn*) binn_map_map(obj, BC_TYPE), nullptr);

            auto stream = vm->getStream(id, type);
            auto ref = new StreamReference(stream);
            ref->loadExtraSerialData((binn*) binn_map_map(obj, BC_EXTRA));
            return ref;
        });


        // Context ID references
        factory->registerReducer(s(ReferenceTag::CONTEXT_ID), [](const Reference* baseRef, VirtualMachine*) {
            auto ref = dynamic_cast<const ContextIdReference*>(baseRef);
            auto obj = binn_map();
            binn_map_set_str(obj, BC_ID, strdup(ref->id().c_str()));
            binn_map_set_map(obj, BC_EXTRA, ref->getExtraSerialData());
            return obj;
        });
        factory->registerProducer(s(ReferenceTag::CONTEXT_ID), [](binn* obj, VirtualMachine*) {
            auto ref = new ContextIdReference(binn_map_str(obj, BC_ID));
            ref->loadExtraSerialData((binn*) binn_map_map(obj, BC_EXTRA));
            return ref;
        });

        // JobID references
        factory->registerReducer(s(ReferenceTag::JOB_ID), [](const Reference* baseRef, VirtualMachine*) {
            auto ref = dynamic_cast<const JobIdReference*>(baseRef);
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_ID, ref->id());
            binn_map_set_map(obj, BC_EXTRA, ref->getExtraSerialData());
            return obj;
        });
        factory->registerProducer(s(ReferenceTag::JOB_ID), [](binn* obj, VirtualMachine*) {
            auto ref = new JobIdReference(binn_map_uint64(obj, BC_ID));
            ref->loadExtraSerialData((binn*) binn_map_map(obj, BC_EXTRA));
            return ref;
        });

        // ReturnValueMap references
        factory->registerReducer(s(ReferenceTag::RETURN_VALUE_MAP), [factory](const Reference* baseRef, VirtualMachine* vm) {
            auto ref = dynamic_cast<const ReturnValueMapReference*>(baseRef);
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (std::size_t) ref->tag());
            
            auto keys = ref->keys();
            auto binnmapping = binn_map();
            for ( auto i = 0; i < keys->length(); i++ ) {
                auto key = dynamic_cast<const JobIdReference*>(keys->get(i))->id();
                binn_map_set_map(binnmapping, key, factory->reduce(ref->getReturnValue(key), vm));
            }

            binn_map_set_map(obj, BC_VECTOR_VALUES, binnmapping);
            binn_map_set_map(obj, BC_EXTRA, ref->getExtraSerialData());

            return obj;
        });
        factory->registerProducer(s(ReferenceTag::RETURN_VALUE_MAP), [factory](binn* obj, VirtualMachine* vm) {
            auto valuesbin = (binn*)binn_map_map(obj, BC_VECTOR_VALUES);

            Runtime::ReturnMap map;
            binn_iter iter;
            int key;
            binn value;
            binn_map_foreach(valuesbin,key,value) {
                map.insert({ key, factory->produce(&value, vm) });
            }

            auto ref = new ReturnValueMapReference(map);
            ref->loadExtraSerialData((binn*) binn_map_map(obj, BC_EXTRA));
            return ref;
        });

        // String references
        factory->registerReducer(s(ReferenceTag::STRING), [](const Reference* baseRef, VirtualMachine*) {
            auto ref = dynamic_cast<const StringReference*>(baseRef);
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (std::size_t) ref->tag());
            binn_map_set_str(obj, BC_VALUE, strdup(ref->value().c_str()));
            binn_map_set_map(obj, BC_EXTRA, ref->getExtraSerialData());
            return obj;
        });
        factory->registerProducer(s(ReferenceTag::STRING), [](binn* obj, VirtualMachine*) {
            auto ref = new StringReference(binn_map_str(obj, BC_VALUE));
            ref->loadExtraSerialData((binn*) binn_map_map(obj, BC_EXTRA));
            return ref;
        });


        // Number references
        factory->registerReducer(s(ReferenceTag::NUMBER), [](const Reference* baseRef, VirtualMachine*) {
            auto ref = dynamic_cast<const NumberReference*>(baseRef);
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (std::size_t) ref->tag());
            binn_map_set_double(obj, BC_VALUE, ref->value());
            binn_map_set_map(obj, BC_EXTRA, ref->getExtraSerialData());
            return obj;
        });
        factory->registerProducer(s(ReferenceTag::NUMBER), [](binn* obj, VirtualMachine*) {
            auto ref = new NumberReference(binn_map_double(obj, BC_VALUE));
            ref->loadExtraSerialData((binn*) binn_map_map(obj, BC_EXTRA));
            return ref;
        });


        // Boolean references
        factory->registerReducer(s(ReferenceTag::BOOLEAN), [](const Reference* baseRef, VirtualMachine*) {
            auto ref = dynamic_cast<const BooleanReference*>(baseRef);
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (std::size_t) ref->tag());
            binn_map_set_bool(obj, BC_VALUE, ref->value());
            binn_map_set_map(obj, BC_EXTRA, ref->getExtraSerialData());
            return obj;
        });
        factory->registerProducer(s(ReferenceTag::BOOLEAN), [](binn* obj, VirtualMachine*) {
            auto ref = new BooleanReference(binn_map_bool(obj, BC_VALUE));
            ref->loadExtraSerialData((binn*) binn_map_map(obj, BC_EXTRA));
            return ref;
        });


        // Resource references (serialized through Fabric)
        factory->registerReducer(s(ReferenceTag::RESOURCE), [](const Reference* baseRef, VirtualMachine* vm) {
            auto ref = dynamic_cast<const ResourceReference*>(baseRef);
            auto obj = binn_map();

            // Publish the resource to the Fabric, if necessary
            if ( vm->fabric()->shouldPublish(ref->resource()) ) {
                vm->fabric()->publish(ref->resource());
            }

            binn_map_set_str(obj, BC_ID, strdup(ref->resource()->id().c_str()));
            return obj;
        });
        factory->registerProducer(s(ReferenceTag::RESOURCE), [](binn* obj, VirtualMachine* vm) {
            std::string id = binn_map_str(obj, BC_ID);
            auto resource = vm->fabric()->load(id);
            auto ref = new ResourceReference(resource);
            ref->loadExtraSerialData((binn*) binn_map_map(obj, BC_EXTRA));
            return ref;
        });

        Framework::onShutdown([factory]() {
            delete factory;
        });
        return factory;
    }
}
