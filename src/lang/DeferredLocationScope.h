#ifndef SWARMC_DEFERREDLOCATIONSCOPE_H
#define SWARMC_DEFERREDLOCATIONSCOPE_H

#include <cassert>
#include "../shared/nslib.h"
#include "../vm/isa_meta.h"

namespace swarmc::Lang {

    // locations where JobId and ContextId are stored respectively
    using JobData = std::pair<ISA::LocationReference*, ISA::LocationReference*>;

    class DeferredLocationScope {
    public:
        DeferredLocationScope(DeferredLocationScope* parent) : _parent(parent), _locations({}) {}
        
        ~DeferredLocationScope() = default; 

        [[nodiscard]] DeferredLocationScope* enter() {
            auto w = new DeferredLocationScope(this);
            // the sole purpose of this data structure is that it copies the parent scope
            w->_locations = _locations;
            return w;
        }

        [[nodiscard]] DeferredLocationScope* leave() const {
            assert(_parent != nullptr);
            auto parent = _parent;
            delete this;
            return parent;
        }

        void add(ISA::LocationReference* location, ISA::LocationReference* jobid, ISA::LocationReference* context) {
            assert( !contains(location) );
            _locations.insert({location, JobData(jobid, context)});
        }

        void remove(ISA::LocationReference* location) {
            for ( auto p : _locations ) {
                if ( p.first->is(location)) {
                    _locations.erase(location);
                    break;
                }
            }
        }

        [[nodiscard]] bool contains(ISA::LocationReference* location) {
            for ( auto p : _locations ) {
                if ( p.first->is(location) ) return true;
            }
            return false;
        }

        [[nodiscard]] JobData drain(ISA::LocationReference* loc) {
            for ( auto p : _locations ) {
                if ( p.first->is(loc) ) {
                    remove(p.first);
                    return p.second;
                }
            }
            return { nullptr, nullptr };
        }

    protected:
        DeferredLocationScope* _parent;
        std::unordered_map<ISA::LocationReference*, JobData> _locations;
    };

}

#endif