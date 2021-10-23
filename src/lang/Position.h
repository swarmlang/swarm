#ifndef SWARMC_POSITION_H
#define SWARMC_POSITION_H

#include <cstdint>
#include <string>
#include <sstream>
#include "../shared/IStringable.h"

namespace swarmc {
namespace Lang {

    /**
     * Class representing a character range in the input file.
     */
    class Position : public IStringable {
    public:
        Position(size_t startLine, size_t endLine, size_t startCol, size_t endCol) :
            _startLine(startLine), _endLine(endLine), _startCol(startCol), _endCol(endCol) {};

        Position(Position* start, Position* end) {
            expand(start, end);
        }

        virtual void expand(Position* start, Position* end) {
            _startLine = start->_startLine;
            _endLine = end->_endLine;
            _startCol = start->_startCol;
            _endCol = end->_endCol;
        }

        virtual std::string start() const {
            std::stringstream s;
            s << "[l: " << _startLine << ", c: " << _startCol << "]";
            return s.str();
        }

        virtual std::string end() const {
            std::stringstream s;
            s << "[l: " << _endLine << ", c: " << _endCol << "]";
            return s.str();
        }

        std::string toString() const {
            std::stringstream s;
            s << start() << "-" << end();
            return s.str();
        }

    protected:
        size_t _startLine;
        size_t _endLine;
        size_t _startCol;
        size_t _endCol;
    };

}
}

#endif
