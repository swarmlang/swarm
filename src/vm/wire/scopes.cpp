#include <functional>
#include "../../shared/nslib.h"
#include "../walk/binary_const.h"
#include "../Wire.h"
#include "../isa_meta.h"
#include "../VirtualMachine.h"

using namespace nslib::serial;
using namespace swarmc::ISA;

namespace swarmc::Runtime {

    Factory<ScopeFrame, VirtualMachine*>* Wire::buildScopes() {
        auto factory = new Factory<ScopeFrame, VirtualMachine*>;

        factory->registerReducer("swarm::Runtime::ScopeFrame", [factory](const ScopeFrame* scope, VirtualMachine* vm) {
            auto names = binn_list();
            auto locations = binn_list();
            int len = 0;
            for ( const auto& pair : scope->nameMap() ) {
                len += 1;
                binn_list_add_str(names, strdup(pair.first.c_str()));
                binn_list_add_map(locations, references()->reduce(pair.second, vm));
            }


            auto parent = scope->parent();
            auto call = scope->call();
            auto ret = scope->getReturnCall();
            auto returnTo = scope->getReturnPC();

            auto binn = binn_map();
            binn_map_set_list(binn, BC_NAMES, names);
            binn_map_set_list(binn, BC_LOCATIONS, locations);
            binn_map_set_int64(binn, BC_LENGTH, len);
            binn_map_set_str(binn, BC_ID, strdup(scope->id().c_str()));
            binn_map_set_bool(binn, BC_IS_EX_FRAME, scope->isExceptionFrame());
            binn_map_set_bool(binn, BC_CAPTURE_RETURN, scope->shouldCaptureReturn());

            binn_map_set_bool(binn, BC_HAS_PARENT, parent != nullptr);
            if ( parent != nullptr ) binn_map_set_map(binn, BC_PARENT, factory->reduce(parent, vm));

            binn_map_set_bool(binn, BC_HAS_CALL, call != nullptr);
            if ( call != nullptr ) binn_map_set_map(binn, BC_CALL, calls()->reduce(call, vm));

            binn_map_set_bool(binn, BC_HAS_RETURN, ret != nullptr);
            if ( ret != nullptr ) binn_map_set_map(binn, BC_RETURN, calls()->reduce(call, vm));

            binn_map_set_bool(binn, BC_HAS_RETURN_PC, returnTo != std::nullopt);
            if ( returnTo != std::nullopt ) binn_map_set_uint64(binn, BC_RETURN_PC, (std::size_t) *returnTo);

            binn_map_set_map(binn, BC_EXTRA, scope->getExtraSerialData());

            return binn;
        });

        factory->registerProducer("swarm::Runtime::ScopeFrame", [factory](binn* obj, VirtualMachine* vm) ->ScopeFrame* {
            std::string id = binn_map_str(obj, BC_ID);
            ScopeFrame* parent = nullptr;
            if ( binn_map_bool(obj, BC_HAS_PARENT) ) {
                parent = factory->produce((binn*) binn_map_map(obj, BC_PARENT), vm);
            }

            IFunctionCall* call = nullptr;
            if ( binn_map_bool(obj, BC_HAS_CALL) ) {
                call = calls()->produce((binn*) binn_map_map(obj, BC_CALL), vm);
            }

            auto scope = new ScopeFrame(vm->global(), id, parent, call);
            scope->_isExceptionFrame = binn_map_bool(obj, BC_IS_EX_FRAME);
            scope->_shouldCaptureReturn = binn_map_bool(obj, BC_CAPTURE_RETURN);

            if ( binn_map_bool(obj, BC_HAS_RETURN_PC) ) {
                scope->_returnTo = std::make_optional(binn_map_uint64(obj, BC_RETURN_PC));
            }

            IFunctionCall* returnCall = nullptr;
            if ( binn_map_bool(obj, BC_HAS_RETURN) ) {
                returnCall = calls()->produce((binn*) binn_map_map(obj, BC_RETURN), vm);
            }
            scope->_return = returnCall;

            auto names = binn_map_list(obj, BC_NAMES);
            auto locations = binn_map_list(obj, BC_LOCATIONS);
            auto len = binn_map_int64(obj, BC_LENGTH);
            for ( int i = 0; i < len; i += 1 ) {
                std::string name = binn_list_str(names, i);
                auto location = (binn*) binn_list_map(locations, i);
                auto ref = references()->produce(location, vm);
                assert(ref->tag() == ReferenceTag::LOCATION);
                scope->_map[name] = dynamic_cast<LocationReference*>(ref);
            }

            scope->loadExtraSerialData((binn*) binn_map_map(obj, BC_EXTRA));
            return scope;
        });

        return factory;
    }

}
