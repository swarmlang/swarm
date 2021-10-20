#ifndef SWARMC_VALIDATOR_H
#define SWARMC_VALIDATOR_H

#include <string>

template <typename T>
class Validator {
private:
    bool use_in_set = false;
    bool use_not_in_set = false;
    bool use_equality = false;
    bool use_less_than = false;
    bool use_greater_than = false;
    bool use_less_than_inclusivity = false;
    bool use_greater_than_inclusivity = false;

    // If true, in_set and not_in_set will be deleted on destruction. Defaults false
    bool do_cleanup_on_end = false;

    // The value must be a member of this set.
    T* in_set = nullptr;
    int in_set_size = 0;

    // The value must not be a member of this set.
    T* not_in_set = nullptr;
    int not_in_set_size = 0;

    // The value must be exactly this
    T equality;

    // The value must be less than this
    T _less_than;

    // The value must be greater than this
    T _greater_than;

public:
    // Construct. If cleanup true, set arrays will be deleted on destroy
    Validator(bool cleanup = false) : do_cleanup_on_end(cleanup) {};

    // If applicable delete the set arrays
    ~Validator();

    // Specify a set of values that are valid
    Validator<T>* in(T* set, int size);

    // Specify a set of values that are not valid
    Validator<T>* not_in(T* set, int size);

    // Specify a single valid value
    Validator<T>* equals(T val);

    // Specify a minimum value
    Validator<T>* greater_than(T val, bool inclusive = false);

    // Specify a maximum value
    Validator<T>* less_than(T val, bool inclusive = false);

    // Check if a value passes the validation criteria
    bool check(T value);

    // Get a human-readable list of requirements for this validator
    std::string requirements(std::string name);
};

#include "Validator.icpp"
#endif //SWARMC_VALIDATOR_H
