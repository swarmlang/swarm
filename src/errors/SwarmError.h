#ifndef SWARMC_SWARMERROR_H
#define SWARMC_SWARMERROR_H

#include <string>
#include <stdexcept>
#include "../shared/nslib.h"

using namespace nslib;

namespace swarmc::Errors {

    class SwarmError : public std::logic_error, public IStringable {
    public:
        explicit SwarmError(const std::string& message) : std::logic_error(message), _message(message) {}

        [[nodiscard]] std::string toString() const override {
            return _message;
        }

    private:
        std::string _message;
    };

}

#endif
