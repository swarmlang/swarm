#ifndef SWARMC_TOKEN_H
#define SWARMC_TOKEN_H

#include <sstream>
#include <utility>
#include "../shared/nslib.h"
#include "Position.h"

using namespace nslib;

namespace swarmc::Lang {

    /**
     * Base class for tokens lexed from some input source.
     * Should be overridden for types that need more detailed recordkeeping.
     */
    class Token : public IStringable {
    public:
        Token(Position* pos, int kind, std::string display) : _pos(pos), _kind(kind), _display(std::move(display)) {};
        virtual ~Token() {
            delete _pos;
        }

        Position* position() const {
            return _pos;
        }

        /** Implements IStringable. */
        std::string toString() const override {
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



    /**
     * Token class representing an identifier.
     */
    class IDToken : public Token {
    public:
        IDToken(Position* pos, int kind, std::string display, std::string identifier) : Token(pos, kind, std::move(display)), _identifier(std::move(identifier)) {};

        /** Get the string identifier of this token. */
        std::string identifier() const {
            return _identifier;
        }
    protected:
        std::string _identifier;
    };

    /**
     * Token class representing an string literal.
     */
    class StringLiteralToken : public Token {
    public:
        StringLiteralToken(Position* pos, int kind, std::string display, std::string value) : Token(pos, kind, std::move(display)), _value(std::move(value)) {};

        /** Get the string literal content of this token. */
        std::string value() const {
            return _value;
        }
    protected:
        std::string _value;
    };

    /**
     * Token class representing an number literal.
     */
    class NumberLiteralToken : public Token {
    public:
        NumberLiteralToken(Position* pos, int kind, std::string display, double value) : Token(pos, kind, std::move(display)), _value(value) {};

        /** Get the double representation of number literal content of this token. */
        double value() const {
            return _value;
        }
    protected:
        double _value;
    };

}

#endif
