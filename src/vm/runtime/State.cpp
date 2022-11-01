#include <cassert>
#include "../../shared/uuid.h"
#include "../../errors/SwarmError.h"
#include "State.h"

namespace swarmc::Runtime {

    void ScopeFrame::shadow(ISA::LocationReference* ref) {
        auto name = ISA::LocationReference::affinityString(ref->affinity()) + ":" + ref->name() + "@" + _id;
        if ( _map.find(name) != _map.end() ) {
            throw Errors::SwarmError("Attempted to shadow reference in a scope where it was already shadowed: " + ref->toString());
        }

        _map[name] = new ISA::LocationReference(ref->affinity(), name);
    }

    ISA::LocationReference* ScopeFrame::map(ISA::LocationReference* ref) {
        auto name = ISA::LocationReference::affinityString(ref->affinity()) + ":" + ref->name() + "@" + _id;
        if ( _map.find(name) != _map.end() ) {
            return _map[name];
        }

        if ( _parent != nullptr ) {
            return _parent->map(ref);
        }

        return ref;
    }

    ScopeFrame* ScopeFrame::newChild() {
        return new ScopeFrame(util::uuid4(), this);
    }

    ScopeFrame* ScopeFrame::newCall(IFunctionCall* call) {
        return new ScopeFrame(util::uuid4(), this, call);
    }

    std::string ScopeFrame::toString() const {
        auto mine = "ScopeFrame<id: " + _id + ", #symbols: " + std::to_string(_map.size()) + ">";
        if ( _parent == nullptr ) {
            return mine;
        }

        auto parent = _parent->toString();
        std::string indentedparent;
        for ( auto c : parent ) {
            indentedparent += c;
            if ( c == '\n' ) indentedparent += "  ";
        }
        return indentedparent + "\n" + mine;
    }

    void State::annotate()  {
        for ( auto it = _is.begin(); it != _is.end(); ++it ) {
            auto i = *it;
            if ( i->tag() == ISA::Tag::BEGINFN ) {
                auto idx = std::distance(_is.begin(), it);
                auto fn = (ISA::BeginFunction*) i;
                auto name = fn->first()->name();

                if ( _fJumps.find(name) == _fJumps.end() ) {
                    _fJumps[name] = idx;
                } else {
                    throw Errors::SwarmError("Duplicate function region identifier: " + name + " (inline function names must be unique)");
                }
            }
        }
    }

    std::vector<ISA::FunctionParam*> State::loadInlineFunctionParams(ISA::Instructions::size_type pc) const {
        assert(pc < _is.size() && _is[pc]->tag() == ISA::Tag::BEGINFN);

        std::vector<ISA::FunctionParam*> ps;
        for ( ISA::Instructions::size_type i = pc; i < _is.size(); i += 1 ) {
            auto inst = _is[i];
            // fnparam instructions must be the first instructions after the beginfn
            if ( inst->tag() != ISA::Tag::FNPARAM ) break;
            ps.push_back((ISA::FunctionParam*) inst);
        }

        return ps;
    }
}
