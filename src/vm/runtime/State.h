#ifndef SWARMVM_STATE
#define SWARMVM_STATE

#include <map>
#include <stack>
#include <utility>
#include "../../shared/nslib.h"
#include "../../errors/SwarmError.h"
#include "../isa_meta.h"
#include "../debug/Metadata.h"

using namespace nslib;

namespace swarmc::Runtime {

    class InlineFunction;
    class IFunctionCall;

    /**
     * A linked-list style dynamic scope data structure used by the VM.
     */
    class ScopeFrame : public IStringable {
    public:
        ScopeFrame(std::string id, ScopeFrame* parent) : _parent(parent) {
            _id = std::move(id);
        }
        ScopeFrame(std::string id, ScopeFrame* parent, IFunctionCall* call) : _parent(parent), _call(call) {
            _id = std::move(id);
        }
        ~ScopeFrame() override = default;

        /** Make this instance the parent scope of the given location. */
        void shadow(ISA::LocationReference*);

        /** Resolve the nominal location to the dynamically-scoped location. */
        ISA::LocationReference* map(ISA::LocationReference*);

        /** Create a new child of this scope and return it. */
        ScopeFrame* newChild();

        /** Create a new function call scope as a child of this scope and return it. */
        ScopeFrame* newCall(IFunctionCall*);

        /** Get the parent of this scope. If this is the top-level, returns nullptr. */
        ScopeFrame* parent() { return _parent; }

        /** Get the nearest call in the call stack. If top-level, returns nullptr. */
        IFunctionCall* call() {
            if ( _call != nullptr ) return _call;
            if ( _parent != nullptr ) return _parent->call();
            return nullptr;
        }

        [[nodiscard]] std::string toString() const override;

        /** Create a deep copy of this scope. */
        [[nodiscard]] ScopeFrame* copy() const {
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


    /**
     * Data structure which loads a list of SVI instructions and keeps
     * track of the current position in the program.
     *
     * Provides helpers for jumps, calls, and loading inline functions.
     */
    class State : public IStringable {
    public:
        explicit State(ISA::Instructions is) : _is(std::move(is)) {
            initialize();
        }

        ~State() override = default;

        /** Get the current instruction. */
        ISA::Instruction* current() {
            if ( _rewindToHead && !_is.empty() ) return _is[0];
            if ( _pc >= _is.size() ) return nullptr;
            return _is[_pc];
        }

        /** Returns true if there are no more instructions to be executed. */
        [[nodiscard]] bool isEndOfProgram() const {
            return _pc >= _is.size();
        }

        /** Advance the position of the program to the next instruction. */
        void advance() {
            if ( isEndOfProgram() ) throw Errors::SwarmError("Cannot advance beyond end of program.");

            if ( _rewindToHead ) {
                _pc = 0;
                _rewindToHead = false;
                return;
            }

            _pc += 1;
        }

        /** Rewind the position of the program to the previous instruction. */
        void rewind() {
            if ( _pc < 1 ) {
                _rewindToHead = true;
                return;
            }

            _pc -= 1;
        }

        /** Retrieve the current instruction and advance the position to the next one. */
        ISA::Instruction* pop() {
            auto i = current();
            advance();
            return i;
        }

        /** Jump to the end of the program. */
        void jumpEnd() {
            _pc = _is.size();
        }

        /** Jump to a specific position in the program. */
        void jump(ISA::Instructions::size_type i) {
            if ( i >= _is.size() ) throw Errors::SwarmError("Cannot advance beyond end of program.");
            _pc = i;
        }

        /** Jump to a specific position in the program, keeping track of the return position. */
        void jumpCall(ISA::Instructions::size_type i) {
            auto returnTo = _pc;
            jump(i);
            _callStack.push(returnTo);
        }

        /** Jump to the return location for the function currently in scope and pop the call stack. */
        void jumpReturn() {
            if ( _callStack.empty() ) throw Errors::SwarmError("Cannot make return jump: the call stack is empty");
            auto returnTo = _callStack.top();
            jump(returnTo);
            _callStack.pop();
        }

        /** Get the `fnparam` instructions for the inline function beginning at `pc`. */
        [[nodiscard]] std::vector<ISA::FunctionParam*> loadInlineFunctionParams(ISA::Instructions::size_type pc) const;

        /** Get the position of the inline function with the given name. */
        ISA::Instructions::size_type getInlineFunctionPC(const std::string& name) {
            if ( _fJumps.find(name) == _fJumps.end() ) throw Errors::SwarmError("Unable to find pc for inline function f:" + name);
            return _fJumps[name];
        }

        /** Get the position of the first instruction after the inline function with the given name. */
        ISA::Instructions::size_type getInlineFunctionSkipPC(const std::string& name) {
            if ( _fSkips.find(name) == _fJumps.end() ) throw Errors::SwarmError("Unable to find pc to skip inline function f:" + name);
            return _fSkips[name];
        }

        /** Get the `beginfn` instruction for the function at the given position. */
        [[nodiscard]] ISA::BeginFunction* getInlineFunctionHeader(ISA::Instructions::size_type pc) const;

        /** Returns true if the loaded program has an inline function with the given name. */
        bool hasInlineFunction(const std::string& name) {
            return _fJumps.find(name) != _fJumps.end();
        }

        [[nodiscard]] std::string toString() const override {
            return "Runtime::State<>";
        }

        /** Create a deep copy of this state object. */
        [[nodiscard]] State* copy() const {
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
        Debug::Metadata _meta;
        bool _rewindToHead = false;

        void initialize() {
            _pc = 0;
            extractMetadata();
            annotate();
        }

        void extractMetadata();
        void annotate();
    };

}

#endif //SWARMVM_STATE
