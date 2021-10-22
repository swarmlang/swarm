#ifndef SWARMC_TOKEN_H
#define SWARMC_TOKEN_H

#include "../shared/IStringable.h"
#include "Position.h"

namespace swarmc {
namespace Lang {

    class Token : public IStringable {
    public:
        Token(Position* pos, int kind) : _pos(pos), _kind(kind) {};
        virtual ~Token() {}

        virtual std::string const toString() {
            return "";
        };

    protected:
        Position* _pos;
        int _kind;
    };

}
}

#endif
