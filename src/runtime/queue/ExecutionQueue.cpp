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
