#include <functional>
#include "../runtime/interfaces.h"
#include "../Wire.h"
#include "../walk/binary_const.h"
#include "../runtime/single_threaded.h"
#include "../runtime/multi_threaded.h"
#include "../runtime/redis_driver.h"


namespace swarmc::Runtime {

    Factory<IStorageInterface, VirtualMachine*>* Wire::buildStores() {
        auto factory = new Factory<IStorageInterface, VirtualMachine*>;

        //singlthreaded
        factory->registerReducer("swarm::SingleThreaded::StorageInterface", [](const IStorageInterface* store, auto vm) {
            return store->serialize(vm);
        });
        factory->registerProducer("swarm::SingleThreaded::StorageInterface", [](binn* obj, auto vm) -> IStorageInterface* {
            auto store = new SingleThreaded::StorageInterface(ISA::Affinity::LOCAL);
            
            auto refs = binn_map_object(obj, BC_STORE_REFS);
            binn_iter iter;
            char key[1028];
            binn value;
            binn_object_foreach(refs, key, value) {
                std::string skey(key);
                store->store(new ISA::LocationReference(ISA::Affinity::LOCAL, skey.substr(3)), Wire::references()->produce(&value, vm));
            }

            return store;
        });

        // FIXME: implement producers for multithreaded and redis stores.

        //multithreaded
        factory->registerReducer("swarm::MultiThreaded::SharedStorageInterface", [](const IStorageInterface* store, auto vm) {
            return store->serialize(vm);
        });
        factory->registerProducer("swarm::MultiThreaded::SharedStorageInterface", [](binn* obj, auto vm) -> IStorageInterface* {
            return new MultiThreaded::SharedStorageInterface();
        });

        //redis
        factory->registerReducer("swarm::RedisDriver::RedisStorageInterface", [](const IStorageInterface* store, auto vm) {
            return store->serialize(vm);
        });
        factory->registerProducer("swarm::RedisDriver::RedisStorageInterface", [](binn* obj, auto vm) -> IStorageInterface* {
            return new RedisDriver::RedisStorageInterface(vm);
        });

        return factory;
    }

}