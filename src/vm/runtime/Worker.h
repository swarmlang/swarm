#ifndef SWARMVM_WORKER
#define SWARMVM_WORKER

#include "interfaces.h"
#include "../../Configuration.h"

namespace swarmc::Runtime {

    class Worker {
    public:
        explicit Worker(IGlobalServices* global, IQueue* queue) : _global(global), _queue(queue) {}
    
        void wait() {
            while ( !Configuration::THREAD_EXIT ) {
                _queue->tick();
                std::this_thread::sleep_for(std::chrono::microseconds(Configuration::WAITER_SLEEP_uS));
            }
        }
    protected:
        IGlobalServices* _global;
        IQueue* _queue;
    };
}

#endif