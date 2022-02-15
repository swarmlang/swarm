#ifndef SWARMC_REPORTING_H
#define SWARMC_REPORTING_H

#include <string>
#include "shared/util/Console.h"
#include "lang/Position.h"
#include "errors/SwarmError.h"

namespace swarmc {

    class Reporting {
    public:
        static void nameError(const Lang::Position* pos, std::string message) {
            Console::get()
                ->bold()->color("red")->print("[Name Error] ")->reset()
                ->bold()->print(pos->start() + " ")->reset()
                ->line(message);
        }

        static void parseError(const Lang::Position* pos, std::string message) {
            Console::get()
                ->bold()->color("red")->print("[Parse Error] ")->reset()
                ->bold()->print(pos->start() + " ")->reset()
                ->line(message);
        }

        static void typeError(const Lang::Position* pos, std::string message) {
            Console::get()
                ->bold()->color("red")->print("[Type Error] ")->reset()
                ->bold()->print(pos->start() + " ")->reset()
                ->line(message);
        }

        static void syntaxError(const Lang::Position* pos, std::string message) {
            Console::get()
                ->bold()->color("red")->print("[Syntax Error] ")->reset()
                ->bold()->print(pos->start() + " ")->reset()
                ->line(message);
        }
    };

}

#endif
