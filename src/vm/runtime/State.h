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

    enum class StateFlag: size_t {
        NONE = 0,
        JUMPED_FROM_RETURN = 2 << 0,
    };

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
            if ( _pc >= _is.size() ) return nullptr;
            return _is[_pc];
        }

        bool isEndOfProgram() const {
            return _pc >= _is.size();
        }

        void advance() {
            if ( isEndOfProgram() ) throw Errors::SwarmError("Cannot advance beyond end of program.");
            _pc += 1;
        }

        void rewind() {
            if ( _pc < 1 ) throw Errors::SwarmError("Cannot rewind beyond beginning of the program.");
            _pc -= 1;
        }

        ISA::Instruction* pop() {
            auto i = current();
            advance();
            return i;
        }

        void jump(ISA::Instructions::size_type i) {
            if ( i >= _is.size() ) throw Errors::SwarmError("Cannot advance beyond end of program.");
            _pc = i;
        }

        void jumpCall(ISA::Instructions::size_type i) {
            auto returnTo = _pc;
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

        ISA::Instructions::size_type getInlineFunctionPC(std::string name) {
            if ( _fJumps.find(name) == _fJumps.end() ) throw Errors::SwarmError("Unable to find pc for inline function f:" + name);
            return _fJumps[name];
        }

        void setFlag(StateFlag flag) {
            _flags = _flags | ((size_t) flag);
        }

        bool hasFlag(StateFlag flag) const {
            return _flags & ((size_t) flag);
        }

        void clearFlag(StateFlag flag) {
            _flags = _flags & ~((size_t) flag);
        }

        std::string toString() const override {
            return "Runtime::State<>";
        }
    protected:
        ISA::Instructions _is;
        std::map<std::string, ISA::Instructions::size_type> _fJumps;
        ISA::Instructions::size_type _pc = 0;
        std::stack<ISA::Instructions::size_type> _callStack;
        size_t _flags = 0;

        void initialize() {
            _pc = 0;
            annotate();
        }

        void annotate();
    };

}

#endif //SWARMVM_STATE
