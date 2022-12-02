#include <filesystem>
#include <optional>
#include "../../shared/nslib.h"
#include "../../Configuration.h"
#include "../../errors/SwarmError.h"
#include "Debugger.h"
#include "../VirtualMachine.h"
#include "../ISAParser.h"

using namespace nslib;

/*
 * TODO: print current stack w/ exception handlers
 * TODO: make peek show surrounding instructions
 * TODO: show locks
 * TODO: 2nd publisher from VM to show informational messages
 * TODO: show runtime exceptions
 * TODO: allow setting breakpoints/location watchers
 * TODO: terse mode
 * TODO: load debugger config from file?
 * TODO: intercept job queue execution
 * TODO: mapping debugging metadata back to original source
 */

namespace swarmc::Runtime::Debug {

    void Debugger::launchInteractive(VirtualMachine* vm) {
        auto console = Console::get();
        vm->_debugger->_interactive = true;

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
            } else if ( cmd == s(DebuggerCommand::STATE) ) {
                dataOut.publish(s(DebuggerDataPrefix::STATE) + getStateString(vm));
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
            } else if ( cmd == s(DebuggerCommand::RUN) ) {
                try {
                    vm->execute();
                    dataOut.publish(s(DebuggerDataPrefix::RUN));
                } catch (Errors::SwarmError& e) {
                    dataOut.publish(s(DebuggerDataPrefix::RUN_ERROR) + e.what());
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
            } else if ( data.rfind(s(DebuggerDataPrefix::STATE), 0) == 0 ) {
                console->println(data.substr(s(DebuggerDataPrefix::STATE).size(), data.size()));
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
            dataIn.listenOnce();
        }
    }

    DebuggerCommand Debugger::promptInteractiveCommand() {
        auto console = Console::get();
        while ( true ) {
            auto cmd = console->choice("SVM Interactive Debugger", {"step", "run", "state", "peek", "lookup", "exit"}, "step");
            if ( cmd == "exit" ) return DebuggerCommand::EXIT;
            if ( cmd == "step" ) return DebuggerCommand::STEP;
            if ( cmd == "run" ) return DebuggerCommand::RUN;
            if ( cmd == "lookup" ) return DebuggerCommand::LOOKUP;
            if ( cmd == "state" ) return DebuggerCommand::STATE;
            if ( cmd == "peek" ) return DebuggerCommand::PEEK;

            console->color(ANSIColor::RED)->println("Invalid command '" + cmd + "'.");
        }
    }

    std::string Debugger::getStateString(VirtualMachine* vm) {
        auto console = Console::get();
        console->capture();

        console->println()->color(ANSIColor::MAGENTA)->header("Call Stack")->println();
        size_t level = 0;
        auto scope = vm->_scope;
        while ( scope != nullptr ) {
            auto returnTo = scope->getReturnPC();
            if ( returnTo == std::nullopt || scope->call() == nullptr ) {
                scope = scope->parent();
                continue;
            }

            auto returnI = vm->_state->lookup(*returnTo);
            console->println(str::padFront("Call: ", level++) + s(scope->call()) + "  |  Return: " + s(returnI) + " (pc: " + s(*returnTo) + ")");
            scope = scope->parent();
        }

        console->println()->color(ANSIColor::YELLOW)->header("Exception Handlers")->println();
        scope = vm->_scope;
        while ( scope != nullptr ) {
            auto handlers = scope->getExceptionHandlers();
            scope = scope->parent();
            if ( handlers.empty() ) continue;

            console->color(ANSIColor::CYAN)->println(s(scope));
            stl::stackLoop<ExceptionHandler>(handlers, [&console](const ExceptionHandler& h) {
                console->color(ANSIColor::CYAN)->print("    ID: ", true)->print(std::get<0>(h))
                    ->color(ANSIColor::CYAN)->print("  |  Selector: ", true);

                auto selector = std::get<1>(h);
                if ( exceptionHandlerIsUniversal(h) ) {
                    console->print(" (universal)");
                } else if ( selector.first != std::nullopt ) {
                    console->print(" (code: " + s(*selector.first) + ")");
                } else if ( selector.second != nullptr ) {
                    console->print(" " + s(selector.second));
                } else {
                    console->print(" (invalid!)");
                }

                console->color(ANSIColor::CYAN)->print("  |  Handler: ", true)
                    ->println(s(std::get<2>(h)))
                    ->println();
            });

        }

        return console->endCapture();
    }

}
