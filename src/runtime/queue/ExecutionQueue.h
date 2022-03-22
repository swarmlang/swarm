#ifndef SWARM_EXECUTIONQUEUE_H
#define SWARM_EXECUTIONQUEUE_H

#include <cstdlib>
#include <csignal>
#include <unistd.h>
#include <sstream>
#include <sw/redis++/redis++.h>
#include "../../Configuration.h"
#include "../../shared/IStringable.h"
#include "../../shared/util/Console.h"
#include "../../shared/uuid.h"
#include "../../shared/RefPool.h"
#include "../../errors/QueueExecutionError.h"
#include "../../errors/SwarmError.h"
#include "../../lang/AST.h"
#include "../../lang/Walk/SerializeWalk.h"
#include "../../lang/Walk/DeSerializeWalk.h"
#include "../LocalSymbolValueStore.h"
#include "Waiter.h"

namespace swarmc {
namespace Runtime {

    class SharedSymbolValueStore;
    class Lock;

    using namespace sw::redis;

    enum class JobStatus {
        PENDING = 'p',
        RUNNING = 'r',
        SUCCESS = 's',
        FAILURE = 'f',
        UNKNOWN = 'u'
    };

    void executionQueueSignalHandler(int);

    class ExecutionQueue : public IStringable, public IUsesConsole {
    public:
        ExecutionQueue(LocalSymbolValueStore* local) : IUsesConsole(), _local(local) {}
        virtual ~ExecutionQueue() {}

        std::string toString() const override {
            return "ExecutionQueue<>";
        }

        static std::string statusString(JobStatus status) {
            if ( status == JobStatus::PENDING ) return "PENDING";
            if ( status == JobStatus::RUNNING ) return "RUNNING";
            if ( status == JobStatus::SUCCESS ) return "SUCCESS";
            if ( status == JobStatus::FAILURE ) return "FAILURE";
            return "UNKNOWN";
        }

        static void forceClearQueue() {
            while ( true ) {
                auto result = getRedis()->lpop(queueKey());
                if ( !result ) break;

                auto jobId = *result;

                getRedis()->del(payloadKey(jobId));
                getRedis()->del(localsKey(jobId));
                getRedis()->del(statusKey(jobId));
                getRedis()->del(resultKey(jobId));
            }
        }

        JobStatus getStatus(const std::string& jobId) {
            auto status = getRedis()->get(statusKey(jobId));

            if ( status && isJobStatus((char) std::stoi(*status)) ) {
                return (JobStatus) std::stoi(*status);
            }

            console->warn("Unable to retrieve status of job ID: " + jobId);
            console->debug([jobId]() {
                throw Errors::QueueExecutionError("Unable to retrieve status of job ID: " + jobId);
            });

            return JobStatus::UNKNOWN;
        }

        void updateStatus(const std::string& jobId, JobStatus status) {
            console->debug("Setting status of job " + jobId + " to " + statusString(status));
            getRedis()->set(statusKey(jobId), std::to_string((char) status));
            getRedis()->publish(statusChannel(jobId), statusString(status));
        }

        Lang::ASTNode* evaluate(Lang::ASTNode* node) {
            // Push the job onto the queue
            auto waiterRef = queue(node);
            auto waiter = waiterRef->get();

            // Do background work until the job finishes
            workUntil(waiterRef);

            // Make sure it evaluated successfully
            auto status = getStatus(waiter->id());
            if ( status == JobStatus::FAILURE ) {
                throw Errors::QueueExecutionError(getFailureReason(waiter->id()));
            } else if ( status == JobStatus::UNKNOWN ) {
                throw Errors::QueueExecutionError("Job status transitioned to UNKNOWN.");
            }

            // Get the result and return it
            auto result = getResult(waiter->id());
            delete waiter;
            return result;
        }

        void bulkEvaluate(Lang::StatementList* nodes) {
            auto waiterRefs = new std::vector<RefInstance<Waiter>*>;

            // Queue all of the nodes to be evaluated
            for ( auto node : *nodes ) {
                waiterRefs->push_back(queue(node));
            }

            // Wait for all of the nodes to finish evaluating
            for ( auto ref : *waiterRefs ) {
                auto waiter = ref->get();

                // Do background work until the job finishes
                workUntil(ref);

                // Make sure it evaluated successfully
                auto status = getStatus(waiter->id());
                if ( status == JobStatus::FAILURE ) {
                    throw Errors::QueueExecutionError(getFailureReason(waiter->id()));
                } else if ( status == JobStatus::UNKNOWN ) {
                    throw Errors::QueueExecutionError("Job status transitioned to UNKNOWN.");
                }

                // Drop the ref
                delete ref;
            }

            delete waiterRefs;
        }

        RefInstance<Waiter>* queue(Lang::ASTNode* node) {
            std::string jobId = util::uuid4();

            // Push node to queue
            console->debug("Pushing node to queue as job " + jobId + ": " + node->toString());
            Lang::Walk::SerializeWalk serialize;

            // Push the program tree into Redis
            auto payload = serialize.toJSON(node);
            getRedis()->set(payloadKey(jobId), payload);

            // Set the default status
            updateStatus(jobId, JobStatus::PENDING);

            // Push the program's locals into Redis
            auto localsPayload = _local->serialize();
            getRedis()->set(localsKey(jobId), localsPayload);

            // Push the jobId onto the queue so it can be executed
            getRedis()->rpush(queueKey(), jobId);

            // Get new waiter and start it
            auto waiterRef = _waiterPool->alloc(new Waiter(jobId));

            auto waiterInst = waiterRef->get();
            waiterInst->get()->wait();

            // Return ref to waiter
            return waiterInst;
        }

        Lang::ASTNode* getResult(const std::string& jobId) {
            auto result = getRedis()->get(resultKey(jobId));
            if ( !result ) {
                console->debug("Unable to get result for job ID: " + jobId);
                return nullptr;
            }

            Lang::Walk::DeSerializeWalk deserialize;
            std::string json = *result;
            std::istringstream ijson(json);
            return deserialize.deserialize(&ijson);
        }

        std::string getFailureReason(const std::string& jobId) {
            auto reason = getRedis()->get(failReasonKey(jobId));
            if ( !reason ) {
                console->debug("Unable to get failure reason for job ID: " + jobId);
                return "unknown error";
            }

            return *reason;
        }

        bool workOnce();

        void workForever() {
            console->info("Starting queue worker...");
            signal(SIGINT, executionQueueSignalHandler);

            while ( !Configuration::THREAD_EXIT ) {
                if ( !workOnce() ) {
                    // No job was executed. Sleep for a bit to prevent CPU hogging
                    console->debug("No jobs found to execute. Sleeping...");
                    usleep(Configuration::QUEUE_SLEEP_uS);
                }
            }
        }

        void workUntil(RefInstance<Waiter>* waiterRef) {
            auto waiter = waiterRef->get();

            console->debug("Starting work cycle while waiting for job ID: " + waiter->id());
            while ( true ) {
                auto status = getStatus(waiter->id());
                if ( status == JobStatus::FAILURE || status == JobStatus::SUCCESS ) {
                    waiter->finish();
                    break;
                }

//            while ( !waiter->finished() ) {
                if ( !workOnce() ) {
                    // No job was executed. Sleep for a bit to prevent CPU hogging
                    console->debug("No jobs found to execute. Sleeping...");
                    usleep(Configuration::QUEUE_SLEEP_uS);
                }
            }

//            delete waiter;
        }
    protected:
        LocalSymbolValueStore* _local;
        RefPool<Waiter>* _waiterPool = new RefPool<Waiter>;
        static Redis* _redis;

        static Redis* getRedis() {
            if ( _redis == nullptr ) {
                Console::get()->debug("Connecting to Redis...");

                ConnectionOptions opts;
                opts.host = Configuration::REDIS_HOST;
                opts.port = Configuration::REDIS_PORT;
                opts.socket_timeout = std::chrono::milliseconds(0);

                _redis = new Redis(opts);

                // TODO push to availability list
            }

            return _redis;
        }

        static bool isJobStatus(char statusChar) {
            return (
                statusChar == 'p'
                || statusChar == 'r'
                || statusChar == 's'
                || statusChar == 'f'
                || statusChar == 'u'
            );
        }

        static std::string statusKey(const std::string& jobId) {
            return Configuration::REDIS_PREFIX + "job_status_" + jobId;
        }

        static std::string statusChannel(const std::string& jobId) {
            return Configuration::REDIS_PREFIX + "job_status_channel_" + jobId;
        }

        static std::string payloadKey(const std::string& jobId) {
            return Configuration::REDIS_PREFIX + "job_payload_" + jobId;
        }

        static std::string resultKey(const std::string& jobId) {
            return Configuration::REDIS_PREFIX + "job_result_" + jobId;
        }

        static std::string localsKey(const std::string& jobId) {
            return Configuration::REDIS_PREFIX + "job_locals_" + jobId;
        }

        static std::string failReasonKey(const std::string& jobId) {
            return Configuration::REDIS_PREFIX + "job_fail_reason_" + jobId;
        }

        static std::string queueKey() {
            return Configuration::REDIS_PREFIX + "job_queue";
        }

        friend class SharedSymbolValueStore;
        friend class Waiter;
        friend class Lock;
    };

}
}

#endif //SWARM_EXECUTIONQUEUE_H
