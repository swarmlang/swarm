#include "Reporting.h"
#include "Configuration.h"
#include "shared/nslib.h"

namespace swarmc {
    void Reporting::parseDebug(const Lang::Position* pos, const std::string& message) {
        Console::get()->only(Verbosity::DEBUG)
            ->bold()->color(ANSIColor::MAGENTA)->print("[Parse Debug] ")->reset()
            ->bold()->print(pos->toString() + " ")->reset()
            ->println(message);
    }

    void Reporting::parseError(const Lang::Position* pos, const std::string& message) {
        Console::get()
            ->bold()->color(ANSIColor::RED)->print("[Parse Error] ")->reset()
            ->bold()->print(pos->start() + " ")->reset()
            ->println(message);

        if ( Configuration::DEBUG ) {
            throw std::runtime_error("parseError");
        }
    }

    void Reporting::nameDebug(const Lang::Position* pos, const std::string& message) {
        Console::get()->only(Verbosity::DEBUG)
            ->bold()->color(ANSIColor::MAGENTA)->print("[Name Debug] ")->reset()
            ->bold()->print(pos->toString() + " ")->reset()
            ->println(message);
    }

    void Reporting::nameError(const Lang::Position* pos, const std::string& message) {
        Console::get()
            ->bold()->color(ANSIColor::RED)->print("[Name Error] ")->reset()
            ->bold()->print(pos->start() + " ")->reset()
            ->println(message);

        if ( Configuration::DEBUG ) {
            throw std::runtime_error("nameError");
        }
    }

    void Reporting::typeDebug(const Lang::Position* pos, const std::string& message) {
        Console::get()->only(Verbosity::DEBUG)
            ->bold()->color(ANSIColor::MAGENTA)->print("[Type Debug] ")->reset()
            ->bold()->print(pos->toString() + " ")->reset()
            ->println(message);
    }

    void Reporting::typeError(const Lang::Position* pos, const std::string& message) {
        Console::get()
            ->bold()->color(ANSIColor::RED)->print("[Type Error] ")->reset()
            ->bold()->print(pos->start() + " ")->reset()
            ->println(message);

        if ( Configuration::DEBUG ) {
            throw std::runtime_error("typeError");
        }
    }

    void Reporting::syntaxError(const Lang::Position* pos, const std::string& message) {
        Console::get()
            ->bold()->color(ANSIColor::RED)->print("[Syntax Error] ")->reset()
            ->bold()->print(pos->start() + " ")->reset()
            ->println(message);

        if ( Configuration::DEBUG ) {
            throw std::runtime_error("syntaxError");
        }
    }
}