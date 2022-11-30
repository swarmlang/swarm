#ifndef SWARMVM_DEBUGGER
#define SWARMVM_DEBUGGER

#include <utility>

#include "../../shared/nslib.h"
#include "Metadata.h"

using namespace nslib;

namespace swarmc::Runtime {
    class VirtualMachine;
}

namespace swarmc::Runtime::Debug {

    enum class DebuggerCommand: size_t {
        PING,
        STEP,
        PEEK,
        LOOKUP,
        EXIT,
    };

    enum class DebuggerDataPrefix: size_t {
        PONG,
        STEP,
        STEP_ERROR,
        PEEK,
        LOOKUP,
        LOOKUP_ERROR,
    };

    class Debugger : public IStringable {
    public:
        [[nodiscard]] Metadata& meta() { return _meta; }

        void setMetadata(Metadata meta) { _meta = std::move(meta); }

        static void launchInteractive(VirtualMachine* vm);

        static void launchInteractive();

        static DebuggerCommand promptInteractiveCommand();

        [[nodiscard]] std::string toString() const override {
            return "Debug::Debugger<>";
        }

    protected:
        Metadata _meta;
    };

}

namespace nslib {
    inline std::string s(swarmc::Runtime::Debug::DebuggerCommand c) {
        if ( c == swarmc::Runtime::Debug::DebuggerCommand::PING ) return "DebuggerCommand(PING)";
        if ( c == swarmc::Runtime::Debug::DebuggerCommand::STEP ) return "DebuggerCommand(STEP)";
        if ( c == swarmc::Runtime::Debug::DebuggerCommand::PEEK ) return "DebuggerCommand(PEEK)";
        if ( c == swarmc::Runtime::Debug::DebuggerCommand::LOOKUP ) return "DebuggerCommand(LOOKUP)";
        if ( c == swarmc::Runtime::Debug::DebuggerCommand::EXIT ) return "DebuggerCommand(EXIT)";
        return "DebuggerCommand(UNKNOWN)";
    }

    inline std::string s(swarmc::Runtime::Debug::DebuggerDataPrefix p) {
        if ( p == swarmc::Runtime::Debug::DebuggerDataPrefix::PONG ) return "data.pong:";
        if ( p == swarmc::Runtime::Debug::DebuggerDataPrefix::STEP ) return "data.step:";
        if ( p == swarmc::Runtime::Debug::DebuggerDataPrefix::STEP_ERROR ) return "data.step.error:";
        if ( p == swarmc::Runtime::Debug::DebuggerDataPrefix::PEEK ) return "data.peek:";
        if ( p == swarmc::Runtime::Debug::DebuggerDataPrefix::LOOKUP ) return "data.lookup:";
        if ( p == swarmc::Runtime::Debug::DebuggerDataPrefix::LOOKUP_ERROR ) return "data.lookup.error:";
        return "data.unknown:";
    }
}

#endif //SWARMVM_DEBUGGER
