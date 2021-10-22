#ifndef SWARMC_TOKEN_H
#define SWARMC_TOKEN_H

#include <sstream>
#include "../shared/IStringable.h"
#include "Position.h"

namespace swarmc {
namespace Lang {

    /**
     * Base class for tokens lexed from some input source.
     * Should be overridden for types that need more detailed recordkeeping.
     */
    class Token : public IStringable {
    public:
        Token(Position* pos, int kind, std::string display) : _pos(pos), _kind(kind), _display(display) {};
        virtual ~Token() {}

        /** Implements IStringable. */
        virtual std::string toString() const {
            std::stringstream s;

            s << _display << " ";
            s << _pos->start() << "->" << _pos->end();

            return s.str();
        };

    protected:
        Position* _pos;
        int _kind;
        std::string _display;
    };

}
}

#endif
