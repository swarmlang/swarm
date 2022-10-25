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

    std::string ScopeFrame::toString() const {
        auto mine = "ScopeFrame<id: " + _id + ", #symbols: " + std::to_string(_map.size()) + ">";
        if ( _parent == nullptr ) {
            return mine;
        }

        auto parent = _parent->toString();
        std::string indentedparent = "";
        for ( auto c : parent ) {
            indentedparent += c;
            if ( c == '\n' ) indentedparent += "  ";
        }
        return indentedparent + "\n" + mine;
    }
}
