#include <functional>
#include "../../shared/nslib.h"
#include "../walk/binary_const.h"
#include "../Wire.h"
#include "../isa_meta.h"
#include "../VirtualMachine.h"

using namespace nslib::serial;
using namespace swarmc::ISA;

namespace swarmc::Runtime {

    Factory<IFunctionCall, VirtualMachine*>* Wire::buildCalls() {
        auto factory = new Factory<IFunctionCall, VirtualMachine*>;

        auto reducer = [](const IFunctionCall* call, VirtualMachine* vm) {
//            auto vectorTypes = binn_list();
            auto vectorValues = binn_list();
            for ( auto pair : call->vector() ) {
//                binn_list_add_object(vectorTypes, types()->reduce(pair.first));
                binn_list_add_object(vectorValues, references()->reduce(pair.second, vm));
            }

            auto binn = binn_map();
            binn_map_set_uint64(binn, BC_BACKEND, (std::size_t) call->backend());
            binn_map_set_str(binn, BC_NAME, strdup(call->name().c_str()));
//            binn_map_set_object(binn, BC_TYPE, types()->reduce(call->returnType()));
            binn_map_set_map(binn, BC_EXTRA, call->getExtraSerialData());
//            binn_map_set_list(binn, BC_VECTOR_TYPES, vectorTypes);
            binn_map_set_list(binn, BC_VECTOR_VALUES, vectorValues);
            return binn;
        };
        factory->registerReducer(s(FunctionBackend::FB_INLINE), reducer);
        factory->registerReducer(s(FunctionBackend::FB_PROVIDER), reducer);
        factory->registerReducer(s(FunctionBackend::FB_INTRINSIC), reducer);

        auto producer = [](binn* obj, VirtualMachine* vm) {
            auto backend = (FunctionBackend) binn_map_uint64(obj, BC_BACKEND);
            std::string name = binn_map_str(obj, BC_NAME);

            // Load the function from the VM
            auto ref = vm->loadFunction(backend, name);
            auto fn = ref->fn();

            // Restore the params via currying
            std::vector<Reference*> params;
            auto list = binn_map_list(obj, BC_VECTOR_VALUES);
            binn_iter iter;
            binn value;
            binn_list_foreach(list, value) {
                fn = fn->curry(references()->produce(&value, vm));
            }

            // Get the call and restore any extra data
            auto call = fn->call();
            call->loadExtraSerialData((binn*) binn_map_map(obj, BC_EXTRA));
            return call;
        };
        factory->registerProducer(s(FunctionBackend::FB_INLINE), producer);
        factory->registerProducer(s(FunctionBackend::FB_PROVIDER), producer);
        factory->registerProducer(s(FunctionBackend::FB_INTRINSIC), producer);

        return factory;
    }

}
