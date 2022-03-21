#ifndef SWARM_028_REDIS_BASICS_H
#define SWARM_028_REDIS_BASICS_H

#include <cassert>
#include "Test.h"
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
            Runtime::ExecutionQueue eq;

            auto jobId = util::uuid4();
            assert(eq.getStatus(jobId) == Runtime::JobStatus::UNKNOWN);

            eq.updateStatus(jobId, Runtime::JobStatus::FAILURE);
            assert(eq.getStatus(jobId) == Runtime::JobStatus::FAILURE);
        }
    };

}
}

#endif //SWARM_028_REDIS_BASICS_H