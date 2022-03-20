#include "Waiter.h"

using namespace swarmc::Runtime;

std::map<std::string, Waiter*>* Waiter::instances = new std::map<std::string, Waiter*>;

void waiterThread() {}
