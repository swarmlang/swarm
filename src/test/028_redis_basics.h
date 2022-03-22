#ifndef SWARM_028_REDIS_BASICS_H
#define SWARM_028_REDIS_BASICS_H

#include <cassert>
#include "Test.h"
#include "../runtime/LocalSymbolValueStore.h"
#include "../runtime/queue/ExecutionQueue.h"
#include "../shared/uuid.h"

namespace swarmc {
namespace Test {

    class RedisBasicsTest : public Test {
    public:
        bool run() override {
            testStatusTracking();
            return true;
        }

        void testStatusTracking() {
            Runtime::LocalSymbolValueStore local;
            Runtime::ExecutionQueue eq(&local);

            auto jobId = util::uuid4();
            eq.updateStatus(jobId, Runtime::JobStatus::FAILURE);
            assert(eq.getStatus(jobId) == Runtime::JobStatus::FAILURE);
        }
    };

}
}

#endif //SWARM_028_REDIS_BASICS_H
