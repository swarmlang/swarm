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
            auto fJumpsKeys = binn_list();
            auto fJumpsValues = binn_list();
            int fJumpsLen = 0;
            for ( const auto& pair : state->_fJumps ) {
                fJumpsLen += 1;
                binn_list_add_str(fJumpsKeys, strdup(pair.first.c_str()));
                binn_list_add_uint64(fJumpsValues, pair.second);
            }

            auto fSkipsKeys = binn_list();
            auto fSkipsValues = binn_list();
            int fSkipsLen = 0;
            for ( const auto& pair : state->_fSkips ) {
                fSkipsLen += 1;
                binn_list_add_str(fSkipsKeys, strdup(pair.first.c_str()));
                binn_list_add_uint64(fSkipsValues, pair.second);
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
            binn_map_set_list(obj, BC_FJUMPS_K, fJumpsKeys);
            binn_map_set_list(obj, BC_FJUMPS_V, fJumpsValues);
            binn_map_set_int64(obj, BC_LENGTH, fJumpsLen);
            binn_map_set_list(obj, BC_FSKIPS_K, fSkipsKeys);
            binn_map_set_list(obj, BC_FSKIPS_V, fSkipsValues);
            binn_map_set_int64(obj, BC_LENGTH2, fSkipsLen);
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

            std::vector<std::string> fJumpsKeys;
            auto fJumpsKeysBinn = binn_map_list(obj, BC_FJUMPS_K);
            binn_iter iter;
            binn value;
            binn_list_foreach(fJumpsKeysBinn, value) {
                fJumpsKeys.emplace_back(binn_get_str(&value));
            }

            auto fJumpsValues = binn_map_list(obj, BC_FJUMPS_V);
            auto fJumpsLen = binn_map_int64(obj, BC_LENGTH);
            for ( int i = 0; i < fJumpsLen; i += 1 ) {
                std::string k = fJumpsKeys[i];
                pc_t v = binn_list_uint64(fJumpsValues, i);
                state->_fJumps[k] = v;
            }

            std::vector<std::string> fSkipsKeys;
            auto fSkipsKeysBinn = binn_map_list(obj, BC_FJUMPS_K);
            binn_list_foreach(fSkipsKeysBinn, value) {
                fSkipsKeys.emplace_back(binn_get_str(&value));
            }

            auto fSkipsValues = binn_map_list(obj, BC_FSKIPS_V);
            auto fSkipsLen = binn_map_int64(obj, BC_LENGTH2);
            for ( int i = 0; i < fSkipsLen; i += 1 ) {
                std::string k = fSkipsKeys[i];
                pc_t v = binn_list_uint64(fSkipsValues, i);
                state->_fSkips[k] = v;
            }

            return state;
        });

        return factory;
    }

}
