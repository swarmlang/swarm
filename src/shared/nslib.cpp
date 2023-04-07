#include "nslib.h"

namespace nslib {

    ThreadID Framework::_lastThreadID = 1;
    bool Framework::_booted = false;
    std::thread::id Framework::_mainPID;
    std::vector<std::function<void()>> Framework::_shutdownCallbacks;
    std::mutex Framework::_mutex;
    std::map<ThreadID, IThreadContext*> Framework::_contexts;
    std::map<std::thread::id, ThreadID> Framework::_threadMap;
    std::optional<std::thread::id> Console::_mainPID = std::nullopt;
    std::map<std::thread::id, Console*> Console::_threadCopies;
    Console* Console::_global = nullptr;
    event::EventBus* event::EventBus::_global = nullptr;
    std::mutex IThreadContext::_threadNumberMutex;
    ThreadID IThreadContext::_nextThreadNumber = 0;

    std::function<void()> ThreadContext::wrap(const std::function<int()>& body) {
        return [body, this]() {
            std::unique_lock<std::mutex> fwLock(Framework::_mutex);
            Framework::_threadMap.insert({std::this_thread::get_id(), _id});
            fwLock.unlock();

            auto exitCode = body();
            shutdown();

            std::lock_guard<std::mutex> m(_mutex);
            _exitCode = exitCode;
            _exited = true;
        };
    }

    template<typename T>
    T IContextualStorage<T>::getFromContext() {
        std::unique_lock<std::mutex> csm(_ctxStoreMutex);

        // Try to find an existing instance in the store
        auto iter = _ctxStore.find(std::this_thread::get_id());
        if ( iter != _ctxStore.end() ) {
            return iter->second;
        }

        // Otherwise, produce a new instance
        auto inst = produceNewForContext();
        _ctxStore[std::this_thread::get_id()] = inst;
        csm.unlock();
        Framework::context()
            ->onShutdown([inst, this]() {
                std::lock_guard<std::mutex> csm2(_ctxStoreMutex);
                disposeOfElementForContext(inst);
            });
        return inst;
    }

    void Framework::boot() {
        std::unique_lock<std::mutex> m(_mutex);
        if ( _booted ) return;
        m.unlock();  // If we haven't booted, we make the simplifying assumption that we are the only thread

        _mainPID = std::this_thread::get_id();
        _contexts[0] = new MainContext();
        _threadMap[_mainPID] = 0;

        auto epoch = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count();
        priv::generator.seed(epoch);

        Logging::get()->addTarget(new ConsoleTarget);

        _booted = true;
    }

    void Logger::output(Verbosity v, const std::string& p) {
        Logging::get()->output(_tag, v, p);
    }

    Console* ConsoleService::produceNewForContext() {
        return new Console();
    }

    void ConsoleService::disposeOfElementForContext(Console* c) {
        delete c;
    }

    RefHandle localref(IRefCountable* ref) {
        return RefHandle(ref);
    }

#ifdef NSLIB_GC_TRACK
    GCTracks IRefCountable::_tracks;
    bool IRefCountable::_registeredShutdown = false;
#endif


}
