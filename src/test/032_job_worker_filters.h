#ifndef SWARM_032_JOB_WORKER_FILTERS_H
#define SWARM_032_JOB_WORKER_FILTERS_H

#include <sstream>
#include "../lang/AST.h"
#include "../runtime/queue/ExecutionQueue.h"
#include "../runtime/LocalSymbolValueStore.h"
#include "../Configuration.h"

namespace swarmc {
namespace Test {

    class JobWorkerFiltersTest : public Test {
    public:
        JobWorkerFiltersTest() : Test() {}

        bool run() override {
            Runtime::ExecutionQueue::forceClearQueue();

            std::map<std::string, std::string> filters1 = std::map<std::string, std::string>({
                {"continent", "Australia"},
                {"rank", "69"},
            });
            std::map<std::string, std::string> filters2 = std::map<std::string, std::string>({
                {"continent", "Europe"},
                {"rank", "420"},
            });

            Lang::Position pos(0, 0, 0, 0);
            Lang::NumberLiteralExpressionNode num1(&pos, 6.9);
            Lang::NumberLiteralExpressionNode num2(&pos, 42);

            Lang::AddNode exp(&pos, &num1, &num2);

            Runtime::LocalSymbolValueStore locals;

            Runtime::ExecutionQueue e(&locals);

            auto waiter = e.queue(&exp);
            e.workOnce();
            e.checkWaiter(waiter);
            assert(!waiter->get()->finished());

            Configuration::QUEUE_FILTERS = filters1;
            e.workOnce();
            e.checkWaiter(waiter);
            assert(waiter->get()->finished());

            waiter = e.queue(&exp);
            e.workOnce();
            e.checkWaiter(waiter);
            assert(!waiter->get()->finished());

            Runtime::ExecutionQueue::forceClearQueue();
            return true;
        }
    };

}
}

#endif