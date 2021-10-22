#ifndef SWARMC_ISTRINGABLE_H
#define SWARMC_ISTRINGABLE_H

#include <string>

// An interface for classes that can be cast to strings
// This is used so that classes can be printed using the Console class
class IStringable {
public:
    virtual ~IStringable() {}
    virtual std::string toString() const = 0;
};

#endif //SWARMC_ISTRINGABLE_H
