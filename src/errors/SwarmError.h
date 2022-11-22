#ifndef SWARMC_SWARMERROR_H
#define SWARMC_SWARMERROR_H

#include <string>
#include <stdexcept>
#include "../shared/nslib.h"

using namespace nslib;

namespace swarmc {
namespace Errors {

    class SwarmError : public std::logic_error, public IStringable {
    public:
        SwarmError(std::string message) : std::logic_error(message), _message(message) {}

        virtual std::string toString() const {
            return _message;
        }

    private:
        std::string _message;
    };

}
}

#endif
