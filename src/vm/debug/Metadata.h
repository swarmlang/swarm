#ifndef SWARMVM_DEBUGMETADATA
#define SWARMVM_DEBUGMETADATA

#include <tuple>
#include <map>
#include "../../shared/nslib.h"

using namespace nslib;

namespace swarmc::Runtime::Debug {

    using Position = std::tuple<std::string, std::size_t, std::size_t>;

    class Metadata : public IStringable {
    public:

        void addMapping(std::size_t pc, const std::string& file, size_t line, size_t col) {
            _sourceMap[pc] = std::make_tuple(file, line, col);
        }

        [[nodiscard]] bool hasMapping(size_t pc) const {
            return _sourceMap.find(pc) != _sourceMap.end();
        }

        Position getMapping(size_t pc) {
            return _sourceMap[pc];
        }

        [[nodiscard]] std::string toString() const override {
            return "Debug::Metadata<#map: " + std::to_string(_sourceMap.size()) + ">";
        }

    protected:
        std::map<size_t, Position> _sourceMap;
    };

}

#endif //SWARMVM_DEBUGMETADATA
