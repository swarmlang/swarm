#ifndef SWARMC_REPORTING_H
#define SWARMC_REPORTING_H

#include <string>
#include "shared/util/Console.h"
#include "lang/Position.h"
#include "errors/SwarmError.h"
#include "../Configuration.h"

namespace swarmc {

    class Reporting {
    public:
        static void nameError(const Lang::Position* pos, std::string message) {
            Console::get()
                ->bold()->color("red")->print("[Name Error] ")->reset()
                ->bold()->print(pos->start() + " ")->reset()
                ->line(message);

            if ( Configuration::DEBUG ) {
                throw std::runtime_error("nameError");
            }
        }

        static void parseError(const Lang::Position* pos, std::string message) {
            Console::get()
                ->bold()->color("red")->print("[Parse Error] ")->reset()
                ->bold()->print(pos->start() + " ")->reset()
                ->line(message);

            if ( Configuration::DEBUG ) {
                throw std::runtime_error("parseError");
            }
        }

        static void typeError(const Lang::Position* pos, std::string message) {
            Console::get()
                ->bold()->color("red")->print("[Type Error] ")->reset()
                ->bold()->print(pos->start() + " ")->reset()
                ->line(message);

            if ( Configuration::DEBUG ) {
                throw std::runtime_error("typeError");
            }
        }

        static void syntaxError(const Lang::Position* pos, std::string message) {
            Console::get()
                ->bold()->color("red")->print("[Syntax Error] ")->reset()
                ->bold()->print(pos->start() + " ")->reset()
                ->line(message);

            if ( Configuration::DEBUG ) {
                throw std::runtime_error("syntaxError");
            }
        }
    };

}

#endif
