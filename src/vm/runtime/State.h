#ifndef SWARMVM_STATE
#define SWARMVM_STATE

#include <map>
#include "../../shared/IStringable.h"
#include "../../errors/SwarmError.h"
#include "../isa_meta.h"

namespace swarmc::Runtime {

    class ScopeFrame : public IStringable {
    public:
        ScopeFrame(std::string id, ScopeFrame* parent) : _parent(parent) {
            _id = id;
        }
        virtual ~ScopeFrame() = default;

        void shadow(ISA::LocationReference*);
        ISA::LocationReference* map(ISA::LocationReference*);
        ScopeFrame* newChild();

        std::string toString() const;
    protected:
        ScopeFrame* _parent = nullptr;
        std::map<std::string, ISA::LocationReference*> _map;
        std::string _id;
    };

    class State : public IStringable {
    public:
        State(ISA::Instructions is) : _is(is) {
            initialize();
        }

        virtual ~State() = default;

        ISA::Instruction* current() {
            if ( _pc >= _is.size() ) return nullptr;
            return _is[_pc];
        }

        void advance() {
            if ( _pc >= _is.size() ) throw Errors::SwarmError("Cannot advance beyond end of program.");
            _pc += 1;
        }

        ISA::Instruction* pop() {
            auto i = current();
            advance();
            return i;
        }

        std::string toString() const override {
            return "Runtime::State<>";
        }
    protected:
        ISA::Instructions _is;
        std::map<std::string, ISA::Instructions::size_type> _fmap;
        ISA::Instructions::size_type _pc = 0;

        void initialize() {
            _pc = 0;
            annotate();
        }

        void annotate() {
            for ( auto it = _is.begin(); it != _is.end(); ++it ) {
                auto i = *it;
                if ( i->tag() == ISA::Tag::BEGINFN ) {
                    auto idx = std::distance(_is.begin(), it);
                    auto fn = (ISA::BeginFunction*) i;
                    auto name = fn->first()->name();

                    if ( _fmap.find(name) == _fmap.end() ) {
                        _fmap[name] = idx;
                    } else {
                        throw Errors::SwarmError("Duplicate function region identifier: " + name + " (inline function names must be unique)");
                    }
                }
            }
        }
    };

}

#endif //SWARMVM_STATE
