#ifndef SWARMC_POSITION_H
#define SWARMC_POSITION_H

#include <cstdint>
#include <string>
#include <sstream>
#include <utility>
#include "../shared/nslib.h"

using namespace nslib;
// TODO: create RuntimePosition class w/ trace?

namespace swarmc::Lang {

    /**
     * Class representing a character range in the input file.
     */
    class Position : public IStringable {
    public:
        Position(size_t startLine, size_t endLine, size_t startCol, size_t endCol) :
            _startLine(startLine), _endLine(endLine), _startCol(startCol), _endCol(endCol) {};

        Position(Position* start, Position* end) {
            _startLine = start->_startLine;
            _endLine = end->_endLine;
            _startCol = start->_startCol;
            _endCol = end->_endCol;
        }

        virtual void expand(Position* start, Position* end) {
            _startLine = start->_startLine;
            _endLine = end->_endLine;
            _startCol = start->_startCol;
            _endCol = end->_endCol;
        }

        [[nodiscard]] virtual std::string start() const {
            std::stringstream s;
            s << "[l: " << _startLine << ", c: " << _startCol << "]";
            return s.str();
        }

        [[nodiscard]] virtual std::string end() const {
            std::stringstream s;
            s << "[l: " << _endLine << ", c: " << _endCol << "]";
            return s.str();
        }

        [[nodiscard]] std::string toString() const override {
            std::stringstream s;
            s << start() << "-" << end();
            return s.str();
        }

        [[nodiscard]] size_t startLine() const {
            return _startLine;
        }

        [[nodiscard]] size_t endLine() const {
            return _endLine;
        }

        [[nodiscard]] size_t startCol() const {
            return _startCol;
        }

        [[nodiscard]] size_t endCol() const {
            return _endCol;
        }

        [[nodiscard]] virtual Position* copy() const {
            return new Position(_startLine, _endLine, _startCol, _endCol);
        }

    protected:
        size_t _startLine;
        size_t _endLine;
        size_t _startCol;
        size_t _endCol;
    };


    /** Arbitrary Position instance representing something defined in the Prologue standard library. */
    class ProloguePosition : public Position {
    public:
        explicit ProloguePosition(std::string symbolName): Position(0, 0, 0, 0), _symbolName(std::move(symbolName)) {}

        [[nodiscard]] std::string start() const override {
            return toString();
        }

        [[nodiscard]] std::string end() const override {
            return toString();
        }

        [[nodiscard]] std::string toString() const override {
            return "[Prologue Definition: " + _symbolName + "]";
        }

        [[nodiscard]] ProloguePosition* copy() const override {
            return new ProloguePosition(_symbolName);
        }

    protected:
        std::string _symbolName;
    };

}

#endif
