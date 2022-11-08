#ifndef SWARMVM_STATE
#define SWARMVM_STATE

#include <map>
#include <stack>
#include <utility>
#include "../../shared/IStringable.h"
#include "../../errors/SwarmError.h"
#include "../isa_meta.h"

namespace swarmc::Runtime {

    class InlineFunction;
    class IFunctionCall;

    class ScopeFrame : public IStringable {
    public:
        ScopeFrame(std::string id, ScopeFrame* parent) : _parent(parent) {
            _id = std::move(id);
        }
        ScopeFrame(std::string id, ScopeFrame* parent, IFunctionCall* call) : _parent(parent), _call(call) {
            _id = std::move(id);
        }
        virtual ~ScopeFrame() = default;

        void shadow(ISA::LocationReference*);
        ISA::LocationReference* map(ISA::LocationReference*);
        ScopeFrame* newChild();
        ScopeFrame* newCall(IFunctionCall*);
        ScopeFrame* parent() { return _parent; }
        IFunctionCall* call() {
            if ( _call != nullptr ) return _call;
            if ( _parent != nullptr ) return _parent->call();
            return nullptr;
        }

        std::string toString() const;

        ScopeFrame* copy() const {
            auto copy = new ScopeFrame(_id, _parent == nullptr ? nullptr : _parent->copy());
            copy->_map = _map;
            copy->_call = _call;
            return copy;
        }
    protected:
        ScopeFrame* _parent = nullptr;
        std::map<std::string, ISA::LocationReference*> _map;
        std::string _id;
        IFunctionCall* _call = nullptr;
    };

    class State : public IStringable {
    public:
        State(ISA::Instructions is) : _is(is) {
            initialize();
        }

        virtual ~State() = default;

        ISA::Instruction* current() {
            if ( _rewindToHead && !_is.empty() ) return _is[0];
            if ( _pc >= _is.size() ) return nullptr;
            return _is[_pc];
        }

        bool isEndOfProgram() const {
            return _pc >= _is.size();
        }

        void advance() {
            if ( isEndOfProgram() ) throw Errors::SwarmError("Cannot advance beyond end of program.");

            if ( _rewindToHead ) {
                _pc = 0;
                _rewindToHead = false;
                return;
            }

            _pc += 1;
        }

        void rewind() {
            if ( _pc < 1 ) {
                _rewindToHead = true;
                return;
            }

            _pc -= 1;
        }

        ISA::Instruction* pop() {
            auto i = current();
            advance();
            return i;
        }

        void jumpEnd() {
            _pc = _is.size();
        }

        void jump(ISA::Instructions::size_type i) {
            if ( i >= _is.size() ) throw Errors::SwarmError("Cannot advance beyond end of program.");
            _pc = i;
        }

        void jumpCall(ISA::Instructions::size_type i) {
            auto returnTo = _pc;
//            jump(i+1);  // we begin with the first instruction after the beginfn
            jump(i);
            _callStack.push(returnTo);
        }

        void jumpReturn() {
            if ( _callStack.empty() ) throw Errors::SwarmError("Cannot make return jump: the call stack is empty");
            auto returnTo = _callStack.top();
            jump(returnTo);
            _callStack.pop();
        }

        std::vector<ISA::FunctionParam*> loadInlineFunctionParams(ISA::Instructions::size_type pc) const;

        ISA::Instructions::size_type getInlineFunctionPC(const std::string& name) {
            if ( _fJumps.find(name) == _fJumps.end() ) throw Errors::SwarmError("Unable to find pc for inline function f:" + name);
            return _fJumps[name];
        }

        ISA::Instructions::size_type getInlineFunctionSkipPC(const std::string& name) {
            if ( _fSkips.find(name) == _fJumps.end() ) throw Errors::SwarmError("Unable to find pc to skip inline function f:" + name);
            return _fSkips[name];
        }

        ISA::BeginFunction* getInlineFunctionHeader(ISA::Instructions::size_type pc) const;

        bool hasInlineFunction(const std::string& name) {
            return _fJumps.find(name) != _fJumps.end();
        }

        std::string toString() const override {
            return "Runtime::State<>";
        }

        State* copy() const {
            auto copy = new State(_is);
            copy->_pc = _pc;
            copy->_callStack = _callStack;
            return copy;
        }
    protected:
        ISA::Instructions _is;
        std::map<std::string, ISA::Instructions::size_type> _fJumps;
        std::map<std::string, ISA::Instructions::size_type> _fSkips;
        ISA::Instructions::size_type _pc = 0;
        std::stack<ISA::Instructions::size_type> _callStack;
        bool _rewindToHead = false;

        void initialize() {
            _pc = 0;
            annotate();
        }

        void annotate();
    };

}

#endif //SWARMVM_STATE
