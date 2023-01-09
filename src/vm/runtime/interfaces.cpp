#include "interfaces.h"

namespace swarmc::Runtime {
    void IGlobalServices::applySchedulingFilter(const std::string &key, std::string value) {
        Console::get()->debug("Apply scheduling filter: " + key + " -> " + value);
        _filters[key] = std::move(value);
    }

    void IGlobalServices::applySchedulingFilters(SchedulingFilters filters) {
        Console::get()->debug("Apply bulk scheduling filters.");
        _filters = std::move(filters);
    }

    void IGlobalServices::clearSchedulingFilters() {
        Console::get()->debug("Clear scheduling filters.");
        _filters.clear();
    }

    void IGlobalServices::applyContextFilter(const std::string& key, std::string value) {
        _context[key] = std::move(value);
    }

    void IGlobalServices::clearContextFilters() { _context.clear(); }

    [[nodiscard]] bool IQueueJob::matchesFilters(const SchedulingFilters& current) const {
        auto filters = getFilters();

        return std::all_of(filters.begin(), filters.end(), [current](const std::pair<std::string, std::string>& filter) {
            auto result = current.find(filter.first);
            return result != current.end() && (*result).second == filter.second;
        });
    }
}
