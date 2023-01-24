#include "nslib.h"

namespace nslib {

    bool Framework::_booted = false;
    std::vector<std::function<void()>> Framework::_shutdownCallbacks;
    std::mutex Console::_globalMutex;
    std::optional<std::thread::id> Console::_mainPID = std::nullopt;
    std::map<std::thread::id, Console*> Console::_threadCopies;
    Console* Console::_global = nullptr;

    void Framework::boot() {
        if ( _booted ) return;

        auto epoch = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count();
        priv::generator.seed(epoch);

        Logging::get()->addTarget(Console::get());

        _booted = true;
    }

    void Logger::output(Verbosity v, const std::string& p) {
        Logging::get()->output(_tag, v, p);
    }

    RefHandle localref(IRefCountable* ref) {
        return RefHandle(ref);
    }

#ifdef NSLIB_GC_TRACK
    GCTracks IRefCountable::_tracks;
    bool IRefCountable::_registeredShutdown = false;
#endif

}
