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


        // Function references
        factory->registerReducer(s(ReferenceTag::FUNCTION), [factory](const Reference* baseRef, VirtualMachine* vm) {
            auto ref = dynamic_cast<const FunctionReference*>(baseRef);
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (std::size_t) ref->tag());
            binn_map_set_uint64(obj, BC_BACKEND, (std::size_t) ref->fn()->backend());
            binn_map_set_str(obj, BC_NAME, strdup(ref->fn()->name().c_str()));

            auto params = binn_list();
            for ( auto p : ref->fn()->getCallVector() ) {
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

            auto list = binn_map_list(obj, BC_PARAMS);
            binn_iter iter;
            binn value;
            binn_list_foreach(list, value) {
                params.push_back(factory->produce(&value, vm));
            }

            auto ref = vm->loadFunction((Runtime::FunctionBackend) backend, name);
            auto fn = ref->fn();
            for ( auto p : params ) fn = fn->curry(p);
            delete ref;

            ref = new FunctionReference(fn);
            ref->loadExtraSerialData((binn*) binn_map_map(obj, BC_EXTRA));
            return ref;
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

        return factory;
    }
}
