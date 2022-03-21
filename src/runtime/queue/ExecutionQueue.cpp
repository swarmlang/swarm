#include "ExecutionQueue.h"
#include "../InterpretWalk.h"

sw::redis::Redis* swarmc::Runtime::ExecutionQueue::_redis = nullptr;

bool swarmc::Runtime::ExecutionQueue::workOnce() {
    // Pop job ID from queue
    auto nextJob = getRedis()->lpop(queueKey());

    // If job exists, load AST node (else return false)
    if ( !nextJob ) return false;
    std::string jobId = *nextJob;
    console->debug("Popped job for execution: " + jobId);

    // Update status to running
    updateStatus(jobId, JobStatus::RUNNING);

    // Evaluate AST node - TODO
    try {
        auto payload = getRedis()->get(payloadKey(jobId));
        if ( !payload ) {
            throw Errors::QueueExecutionError("Unable to load payload for job ID: " + jobId);
        }

        Walk::DeSerializeWalk deserialize;
        std::istringstream ijson(*payload);
        ASTNode* node = deserialize.deserialize(&ijson);

        InterpretWalk interpreter;  // FIXME gonna need to set up the local store too
        ASTNode* result = interpreter.walk(node);

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