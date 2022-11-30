#include <filesystem>
#include "../../shared/nslib.h"
#include "../../Configuration.h"
#include "../../errors/SwarmError.h"
#include "Debugger.h"
#include "../VirtualMachine.h"
#include "../ISAParser.h"

using namespace nslib;

namespace swarmc::Runtime::Debug {

    void Debugger::launchInteractive(VirtualMachine* vm) {
        auto console = Console::get();

        // Create the IPC for outputting VM state info to
        ipc::Publisher dataOut(Configuration::DEBUG_SERVER_DATA_PATH);

        // Wait for the command publisher to come online
        ipc::Sleeper cmdReady([]() {
            return std::filesystem::exists(std::filesystem::path(Configuration::DEBUG_SERVER_CMD_PATH));
        });

        console->info("Waiting for debugger to connect...");
        cmdReady.waitFor();
        console->success("Debugger connected. Opening command session...");

        // Listen for commands from the debugger
        ipc::Subscriber cmdIn(Configuration::DEBUG_SERVER_CMD_PATH, [&dataOut, &console, vm](const std::string& cmd) {
            if ( cmd == s(DebuggerCommand::PING) ) {
                console->success("Remote debugger session opened.");
                dataOut.publish(s(DebuggerDataPrefix::PONG));
            } else if ( cmd == s(DebuggerCommand::EXIT) ) {
                console->info("Session terminated from debugger.");
                vm->exit();
                dataOut.exit(false);
                return;
            } else if ( cmd == s(DebuggerCommand::PEEK) ) {
                dataOut.publish(s(DebuggerDataPrefix::PEEK) + s(vm->current()));
            } else if ( cmd.rfind(s(DebuggerCommand::LOOKUP), 0) == 0 ) {
                auto name = cmd.substr(s(DebuggerCommand::LOOKUP).size() + 1, cmd.size());
                try {
                    auto loc = ISA::Parser::parseLocation(name);
                    auto msg = name + " -> " + s(vm->resolve(loc));
                    delete loc;
                    dataOut.publish(s(DebuggerDataPrefix::LOOKUP) + msg);
                } catch (Errors::SwarmError& e) {
                    dataOut.publish(s(DebuggerDataPrefix::LOOKUP_ERROR) + e.what());
                }
            } else if ( cmd == s(DebuggerCommand::STEP) ) {
                auto str = s(vm->current());
                try {
                    vm->step();
                    dataOut.publish(s(DebuggerDataPrefix::STEP) + str);
                } catch (Errors::SwarmError& e) {
                    dataOut.publish(s(DebuggerDataPrefix::STEP_ERROR) + str + "\n\n" + e.what());
                }
            } else {
                console->warn("Invalid command: " + cmd);
            }
        });
        cmdIn.listen();
    }

    void Debugger::launchInteractive() {
        auto console = Console::get();

        // Create the IPC for outputting commands to
        ipc::Publisher cmdOut(Configuration::DEBUG_SERVER_CMD_PATH);

        // Wait for the VM state publisher to come online
        ipc::Sleeper dataReady([]() {
            return std::filesystem::exists(std::filesystem::path(Configuration::DEBUG_SERVER_DATA_PATH));
        });

        console->info("Waiting for interactive session to start...");
        dataReady.waitFor();
        console->success("Interactive session connected. Opening command session...");

        // Create the subscriber to listen for VM state messages
        ipc::Subscriber dataIn(Configuration::DEBUG_SERVER_DATA_PATH, [&console](const std::string& data) {
            if ( data.rfind(s(DebuggerDataPrefix::PONG), 0) == 0 ) {
                console->success("Remote command session opened.");
            } else if ( data.rfind(s(DebuggerDataPrefix::STEP), 0) == 0 ) {
                console->color(ANSIColor::CYAN)->println(data.substr(s(DebuggerDataPrefix::STEP).size(), data.size()));
            } else if ( data.rfind(s(DebuggerDataPrefix::STEP_ERROR), 0) == 0 ) {
                console->color(ANSIColor::RED)->println("Unhandled internal VM exception during execution:\n");
                console->println(data.substr(s(DebuggerDataPrefix::STEP_ERROR).size(), data.size()));
            } else if ( data.rfind(s(DebuggerDataPrefix::PEEK), 0) == 0 ) {
                console->color(ANSIColor::MAGENTA)->println(data.substr(s(DebuggerDataPrefix::PEEK).size(), data.size()));
            } else if ( data.rfind(s(DebuggerDataPrefix::LOOKUP), 0) == 0 ) {
                console->color(ANSIColor::MAGENTA)->println(data.substr(s(DebuggerDataPrefix::LOOKUP).size(), data.size()));
            } else if ( data.rfind(s(DebuggerDataPrefix::LOOKUP_ERROR), 0) == 0 ) {
                console->color(ANSIColor::RED)->println("Unable to look up the value for that location:\n");
                console->println(data.substr(s(DebuggerDataPrefix::LOOKUP_ERROR).size(), data.size()));
            } else {
                console->warn("Invalid data: " + data);
            }
        });

        cmdOut.publish(s(DebuggerCommand::PING));
        dataIn.listenOnce();

        while ( true ) {
            auto cmd = promptInteractiveCommand();

            if ( cmd == DebuggerCommand::LOOKUP ) {
                auto name = console->prompt("Enter a location (e.g. $l:var1, $s:var2, f:FUN1)");
                if ( name.empty() ) {
                    console->info("Aborted lookup.");
                    continue;
                }

                cmdOut.publish(s(cmd) + ":" + name);
                dataIn.listenOnce();
                continue;
            }

            cmdOut.publish(s(cmd));
            if ( cmd == DebuggerCommand::EXIT ) {
                cmdOut.exit();
                return;
            }
            if ( cmd == DebuggerCommand::STEP ) dataIn.listenOnce();
            if ( cmd == DebuggerCommand::PEEK ) dataIn.listenOnce();
        }
    }

    DebuggerCommand Debugger::promptInteractiveCommand() {
        auto console = Console::get();
        while ( true ) {
            auto cmd = console->choice("SVM Interactive Debugger", {"step", "peek", "lookup", "exit"}, "step");
            if ( cmd == "exit" ) return DebuggerCommand::EXIT;
            if ( cmd == "step" ) return DebuggerCommand::STEP;
            if ( cmd == "lookup" ) return DebuggerCommand::LOOKUP;
            if ( cmd == "peek" ) return DebuggerCommand::PEEK;

            console->color(ANSIColor::RED)->println("Invalid command '" + cmd + "'.");
        }
    }

}
