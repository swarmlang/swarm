#include <chrono>
#include "../../Configuration.h"
#include "../../shared/util/Console.h"
#include "Waiter.h"
#include "ExecutionQueue.h"

using namespace swarmc::Runtime;

std::map<std::string, Waiter*>* Waiter::instances = new std::map<std::string, Waiter*>;

sw::redis::Subscriber Waiter::_subscriber = ExecutionQueue::getRedis()->subscriber();

std::thread* Waiter::_thread = nullptr;

bool Waiter::_createdSubscriber = false;

void Waiter::createSubscriber() {
    if ( !_createdSubscriber ) {
        _subscriber.on_message([](std::string channel, std::string message) {
            Console::get()->debug("Incoming message on channel: " + channel);

            // Make sure the channel is for one of the jobs we are waiting on
            std::string prefix = Configuration::REDIS_PREFIX + "job_status_channel_";
            if ( channel.rfind(prefix, 0) != 0 ) return;

            // Parse out the job ID
            std::string jobId = channel.substr(prefix.length());
            Console::get()->debug("Handling message from job ID: " + jobId);

            // Try to find a Waiter we have for that ID
            auto result = instances->find(jobId);
            if ( result == instances->end() ) {
                Console::get()->debug("No waiter for job ID: " + jobId);
                return;
            }

            // If we have one, mark it finished if the status is a final state
            Waiter* waiter = result->second;
            if (
                message == ExecutionQueue::statusString(JobStatus::SUCCESS)
                || message == ExecutionQueue::statusString(JobStatus::FAILURE)
            ) {
                Console::get()->debug("Notifying waiter of job execution finish.");
                waiter->finish();
                instances->erase(jobId);
            }
        });

        _thread = new std::thread([]() mutable {
            while ( !Configuration::THREAD_EXIT ) {
                Console::get()->debug("Consuming subscriber.");
                _subscriber.consume();
                std::this_thread::sleep_for(std::chrono::microseconds(Configuration::WAITER_SLEEP_uS));
            }
        });

        _createdSubscriber = true;
    }
}

void Waiter::wait() {
    if ( !started() ) {
        start();
        createSubscriber();
        instances->insert(std::pair<std::string, Waiter*>(_id, this));
        _subscriber.subscribe({ExecutionQueue::statusChannel(_id)});
    }
}
