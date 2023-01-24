#include "swarm_thread.h"

namespace swarmc::Runtime::MultiThreaded {

    std::mutex SwarmThread::ThreadsMutex;
    std::mutex SwarmThread::FinishedMutex;

}
