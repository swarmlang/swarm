#ifndef SWARMVM_DEBUGMETADATA
#define SWARMVM_DEBUGMETADATA

#include <tuple>
#include <map>
#include "../../shared/nslib.h"

using namespace nslib;

namespace swarmc::Runtime::Debug {

    using Position = std::tuple<std::string, size_t, size_t>;

    class Metadata : public IStringable {
    public:

        void addMapping(size_t pc, std::string file, size_t line, size_t col) {
            _sourceMap[pc] = std::make_tuple(file, line, col);
        }

        bool hasMapping(size_t pc) const {
            return _sourceMap.find(pc) != _sourceMap.end();
        }

        Position getMapping(size_t pc) {
            return _sourceMap[pc];
        }

        std::string toString() const override {
            return "Debug::Metadata<#map: " + std::to_string(_sourceMap.size()) + ">";
        }

    protected:
        std::map<size_t, Position> _sourceMap;
    };

}

#endif //SWARMVM_DEBUGMETADATA
