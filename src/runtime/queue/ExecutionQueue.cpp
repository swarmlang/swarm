#include "ExecutionQueue.h"
#include "../InterpretWalk.h"

sw::redis::Redis* swarmc::Runtime::ExecutionQueue::_redis = nullptr;

void swarmc::Runtime::executionQueueSignalHandler(int) {
    Console::get()->info("Exiting...");
    Configuration::THREAD_EXIT = true;
}

bool swarmc::Runtime::ExecutionQueue::workOnce() {
    // Pop job ID from queue
    auto nextJob = getRedis()->lpop(queueKey());

    // If job exists, load AST node (else return false)
    if ( !nextJob ) return false;
    std::string jobId = *nextJob;
    console->debug("Popped job for execution: " + jobId);

    bool flagsGood = true;
    auto filters = getRedis()->get(filterKey(jobId));
    if ( !filters ) return false;
    std::map<std::string, std::string> filterMap = nlohmann::json::parse(*filters);

    console->debug("Job Filters:");
    for (auto f : filterMap) {
        console->debug(f.first + ": " + f.second);
    }

    console->debug("Worker Filters: ");
    for (auto f : Configuration::QUEUE_FILTERS) {
        console->debug(f.first + ": " + f.second);
    }

    for (auto filter : filterMap) {
        auto worker = Configuration::QUEUE_FILTERS.find(filter.first);
        if ( worker == Configuration::QUEUE_FILTERS.end() ) {
            flagsGood = false;
            break;
        }
        if ( worker->second != filter.second ) {
            flagsGood = false;
            break;
        }
    }

    if ( !flagsGood ) {
        getRedis()->rpush(queueKey(), jobId);
        return false;
    }

    // Update status to running
    updateStatus(jobId, JobStatus::RUNNING);

    try {
        // Pull the AST from Redis and deserialize it
        auto payload = getRedis()->get(payloadKey(jobId));
        if ( !payload ) {
            throw Errors::QueueExecutionError("Unable to load payload for job ID: " + jobId);
        }

        Walk::DeSerializeWalk deserialize;
        std::istringstream ijson(*payload);
        ASTNode* node = deserialize.deserialize(&ijson);

        // Pull the locals from Redis and deserialize them
        auto localsPayload = getRedis()->get(localsKey(jobId));
        if ( !localsPayload ) {
            throw Errors::QueueExecutionError("Unable to load local environment for job ID: " + jobId);
        }

        InterpretWalk interpreter;
        interpreter.locals()->deserialize(*localsPayload);

        // Evaluate the program tree
        ASTNode* result = interpreter.walk(node);

        // Serialize the result and push it back into redis
        Walk::SerializeWalk serialize;
        std::string resultJson = serialize.toJSON(result);

        getRedis()->set(resultKey(jobId), resultJson);
        updateStatus(jobId, JobStatus::SUCCESS);
    } catch (const std::exception& e) {
        getRedis()->set(failReasonKey(jobId), e.what());
        updateStatus(jobId, JobStatus::FAILURE);
        console->debug("Failed to execute job " + jobId + "; exception:  " + e.what());
    } catch (...) {
        getRedis()->set(failReasonKey(jobId), "Caught unknown exception");
        updateStatus(jobId, JobStatus::FAILURE);
        console->debug("Failed to execute job " + jobId + "; unknown exception");
    }

    return true;
}
