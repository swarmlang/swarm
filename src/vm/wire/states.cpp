#include <functional>
#include "../../shared/nslib.h"
#include "../walk/binary_const.h"
#include "../Wire.h"
#include "../isa_meta.h"
#include "../VirtualMachine.h"
#include "../walk/ISABinaryWalk.h"
#include "../walk/BinaryISAWalk.h"

using namespace nslib::serial;
using namespace swarmc::ISA;

namespace swarmc::Runtime {

    Factory<State, VirtualMachine*>* Wire::buildStates() {
        auto factory = new Factory<State, VirtualMachine*>;

        factory->registerReducer("swarm::Runtime::State", [](const State* state, auto vm) {
            auto fJumps = binn_object();
            for ( const auto& pair : state->_fJumps ) {
                binn_object_set_uint64(fJumps, strdup(pair.first.c_str()), pair.second);
            }

            auto fSkips = binn_object();
            for ( const auto& pair : state->_fSkips ) {
                binn_object_set_uint64(fSkips, strdup(pair.first.c_str()), pair.second);
            }

            // FIXME: change this to use Wire once converted
            ISABinaryWalk isaBinaryWalk(vm);
            auto is = binn_list();
            for ( auto i : state->_is ) {
                binn_list_add_map(is, isaBinaryWalk.walkOne(i));
            }

            // FIXME: Debug::Metadata _meta

            auto obj = binn_map();
            binn_map_set_list(obj, BC_INSTRUCTIONS, is);
            binn_map_set_object(obj, BC_FJUMPS, fJumps);
            binn_map_set_object(obj, BC_FSKIPS, fSkips);
            binn_map_set_uint64(obj, BC_PC, state->_pc);
            binn_map_set_bool(obj, BC_REWIND_TO_HEAD, state->_rewindToHead);
            binn_map_set_map(obj, BC_EXTRA, state->getExtraSerialData());
            return obj;
        });

        factory->registerProducer("swarm::Runtime::State", [](binn* obj, auto) ->State* {
            // FIXME: change this to use Wire once converted
            BinaryISAWalk binaryIsaWalk;
            auto inst = (binn*) binn_map_list(obj, BC_INSTRUCTIONS);
            Instructions is = binaryIsaWalk.walk(inst);

            auto state = State::withoutInitialization(is);
            state->_pc = binn_map_uint64(obj, BC_PC);
            state->_rewindToHead = binn_map_bool(obj, BC_REWIND_TO_HEAD);
            state->loadExtraSerialData((binn*) binn_map_map(obj, BC_EXTRA));

            // FIXME: Debug::Metadata _meta
            auto fJumpsBinn = binn_map_object(obj, BC_FJUMPS);
            binn_iter iter;
            char key[1028];
            binn value;
            binn_object_foreach(fJumpsBinn, key, value) {
                std::string skey(key);
                state->_fJumps[skey] = binn_object_uint64(fJumpsBinn, key);
            }

            auto fSkipsBinn = binn_map_list(obj, BC_FSKIPS);
            binn_object_foreach(fSkipsBinn, key, value) {
                std::string skey(key);
                state->_fSkips[skey] = binn_object_uint64(fSkipsBinn, key);
            }

            return state;
        });

        return factory;
    }

}
