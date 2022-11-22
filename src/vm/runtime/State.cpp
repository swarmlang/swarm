#include <cassert>
#include <stack>
#include "../../shared/nslib.h"
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
        return new ScopeFrame(nslib::uuid(), this);
    }

    ScopeFrame* ScopeFrame::newCall(IFunctionCall* call) {
        return new ScopeFrame(nslib::uuid(), this, call);
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

    void State::extractMetadata() {
        size_t pc = 0;
        _is.erase(std::remove_if(_is.begin(), _is.end(), [&pc, this](ISA::Instruction* i) {
            pc += 1;
            if ( i->tag() == ISA::Tag::POSITION ) {
                auto pos = (ISA::PositionAnnotation*) i;
                _meta.addMapping(pc, pos->first()->value(), static_cast<size_t>(pos->second()->value()), static_cast<size_t>(pos->third()->value()));
                return true;
            }

            return false;
        }), _is.end());
    }

    void State::annotate()  {
        std::stack<std::string> nesting;
        for ( auto it = _is.begin(); it != _is.end(); ++it ) {
            auto i = *it;
            if ( i->tag() == ISA::Tag::BEGINFN ) {
                auto idx = std::distance(_is.begin(), it);
                auto fn = (ISA::BeginFunction*) i;
                nesting.push(fn->first()->name());

                if ( _fJumps.find(nesting.top()) == _fJumps.end() ) {
                    _fJumps[nesting.top()] = idx;
                } else {
                    throw Errors::SwarmError("Duplicate function region identifier: " + nesting.top() + " (inline function names must be unique)");
                }
            } else if ( i->tag() == ISA::Tag::RETURN0 || i->tag() == ISA::Tag::RETURN1 ) {
                auto idx = std::distance(_is.begin(), it);

                if ( nesting.empty() ) {
                    throw Errors::SwarmError("Return detected outside function scope (pc: " + std::to_string(idx) + ")");
                } else {
                    _fSkips[nesting.top()] = idx + 1;
                    nesting.pop();
                }
            }
        }
    }

    std::vector<ISA::FunctionParam*> State::loadInlineFunctionParams(ISA::Instructions::size_type pc) const {
        assert(pc < _is.size() && _is[pc]->tag() == ISA::Tag::BEGINFN);

        std::vector<ISA::FunctionParam*> ps;
        for ( ISA::Instructions::size_type i = pc+1; i < _is.size(); i += 1 ) {
            auto inst = _is[i];
            // fnparam instructions must be the first instructions after the beginfn
            if ( inst->tag() != ISA::Tag::FNPARAM ) break;
            ps.push_back((ISA::FunctionParam*) inst);
        }

        return ps;
    }

    ISA::BeginFunction* State::getInlineFunctionHeader(ISA::Instructions::size_type pc) const {
        assert(pc < _is.size() && _is[pc]->tag() == ISA::Tag::BEGINFN);
        return (ISA::BeginFunction*) _is[pc];
    }
}
