#ifndef SWARM_UTIL_STRING_HELPERS
#define SWARM_UTIL_STRING_HELPERS

#include <vector>
#include <string>

std::string string_join(std::string const& delimiter, std::vector<std::string> const& strs) {
    std::string buf;

    if ( strs.size() > 0 ) {
        buf.reserve(strs.size() * strs[0].size());
    }

    bool first = false;
    for ( auto str : strs ) {
        if ( first ) buf += delimiter;
        buf += str;
        first = true;
    }

    return buf;
}

/* https://stackoverflow.com/a/5878802/4971138 */
std::string string_replace(std::string const &s, std::string const &find, std::string const &replace) {
    std::string buf;
    std::size_t pos = 0;
    std::size_t prevPos;

    // Reserves rough estimate of final size of string.
    buf.reserve(s.size());

    while (true) {
        prevPos = pos;
        pos = s.find(find, pos);
        if (pos == std::string::npos)
            break;
        buf.append(s, prevPos, pos - prevPos);
        buf += replace;
        pos += find.size();
    }

    buf.append(s, prevPos, s.size() - prevPos);
    return buf;
}

#endif //SWARM_UTIL_STRING_HELPERS
