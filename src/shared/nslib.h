#ifndef NSLIB_H
#define NSLIB_H

//#define NSLIB_GC_TRACK
//#define NSLIB_GC_DEBUG_FREE
#define NSLIB_GC_NO_FREE

#ifndef NSLIB_BINN_H_PATH
#define NSLIB_BINN_H_PATH "../../mod/binn/src/binn.h"
#endif

#ifndef NSLIB_DL_OPTS
#define NSLIB_DL_OPTS (RTLD_NOW)
#endif

#define CONCAT(a, b) CONCAT_INNER(a, b)
#define CONCAT_INNER(a, b) a ## b
#define UNIQUE_NAME(base) CONCAT(base, __COUNTER__)

/* Creates a unique local variable w/ a RefHandle. */
#define GC_LOCAL_REF(ref) auto UNIQUE_NAME(refHandle) = localref(ref);

/* Disables ref counting on a variable */
#define GC_NO_REF(ref) ref.nslibNoRef();

/* Tags a variable to be cleaned up during application shutdown. */
#define GC_ON_SHUTDOWN(ref) nslib::Framework::onShutdown([ref]() { freeref(ref); });

#define NS_DEFER() nslib::Defer UNIQUE_NAME(deferred)

#include <dlfcn.h>
#include <cassert>
#include <random>
#include <stack>
#include <map>
#include <list>
#include <string>
#include <sstream>
#include <iostream>
#include <chrono>
#include <thread>
#include <functional>
#include <filesystem>
#include <fstream>
#include <utility>
#include <stdexcept>
#include <optional>
#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <semaphore.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <execinfo.h>
#include <mutex>
#include <typeinfo>
#include <string_view>
#include NSLIB_BINN_H_PATH

#define NSLIB_SERIAL_TAG 0
#define NSLIB_SERIAL_DATA 1
#define NSLIB_SERIAL_VERSION 2

namespace nslib {

    namespace priv {
        static std::random_device rd;
        static std::default_random_engine eng(rd());
        static std::mt19937_64 generator;
        static std::uniform_int_distribution<> distribution(0, 15);
        static std::uniform_int_distribution<> distribution2(8, 11);
        static std::uniform_real_distribution<double> realDistribution(1, 0);

        static std::size_t DETERMINISTIC_UUID_LAST = 0;
        static bool USE_DETERMINISTIC_UUIDS = false;
    }


    /** An instance which has a string representation. */
    class IStringable {
    public:
        virtual ~IStringable() = default;
        [[nodiscard]] virtual std::string toString() const = 0;
    };


    class NSLibException : public std::logic_error, public IStringable {
    public:
        explicit NSLibException(const std::string& message) : std::logic_error(message) {}

        [[nodiscard]] std::string toString() const override {
            return what();
        }
    };


    class ForeignThreadException : public NSLibException {
    public:
        ForeignThreadException() : NSLibException("Cannot load context for foreign thread. Make sure to create the threads with Framework::newThread(...).") {}
    };


    using ThreadID = std::size_t;

    class IThreadContext {
    public:
        virtual ~IThreadContext() = default;

        [[nodiscard]] virtual bool isReadyForJoin() = 0;

        [[nodiscard]] virtual std::thread* baseThread() const = 0;

        [[nodiscard]] virtual ThreadID getID() const {
            return _threadNumber;
        }

        virtual void shutdown() = 0;

        virtual void onShutdown(const std::function<void()>& f) = 0;

        [[nodiscard]] virtual bool hasExited() = 0;

        [[nodiscard]] virtual int exitCode() = 0;

        static ThreadID getNextThreadNumber() {
            std::unique_lock<std::mutex> lock(_threadNumberMutex);
            return _nextThreadNumber++;
        }

    protected:
        static std::mutex _threadNumberMutex;
        static ThreadID _nextThreadNumber;

        ThreadID _threadNumber;

        IThreadContext() {
            _threadNumber = getNextThreadNumber();
        }
    };

    class MainContext : public IThreadContext {
    public:
        bool isReadyForJoin() override { return false; }

        std::thread* baseThread() const override { return nullptr; }

        void shutdown() override {
            std::lock_guard<std::mutex> m(_mutex);
            for ( const auto& c : _shutdownCallbacks ) c();
        }

        void onShutdown(const std::function<void()>& f) override {
            std::lock_guard<std::mutex> m(_mutex);
            _shutdownCallbacks.push_back(f);
        }

        bool hasExited() override { return false; }

        int exitCode() override { return 0; }

    protected:
        std::mutex _mutex;
        std::vector<std::function<void()>> _shutdownCallbacks;
    };

    class ThreadContext : public IThreadContext {
    public:
        explicit ThreadContext(ThreadID id, const std::function<int()>& body) : _id(id) {
            _thread = new std::thread(wrap(body));
        }

        [[nodiscard]] bool isReadyForJoin() override {
            std::lock_guard<std::mutex> m(_mutex);
            return _thread->joinable() && _exited;
        }

        [[nodiscard]] std::thread* baseThread() const override {
            return _thread;
        }

        void shutdown() override {
            std::lock_guard<std::mutex> m(_mutex);
            for ( const auto& c : _shutdownCallbacks ) c();
        }

        void onShutdown(const std::function<void()>& f) override {
            std::lock_guard<std::mutex> m(_mutex);
            _shutdownCallbacks.push_back(f);
        }

        bool hasExited() override {
            std::lock_guard<std::mutex> m(_mutex);
            return _exited;
        }

        int exitCode() override {
            std::lock_guard<std::mutex> m(_mutex);
            return _exitCode;
        }

    protected:
        ThreadID _id;
        int _exitCode = 0;
        bool _exited = false;
        std::mutex _mutex;
        std::vector<std::function<void()>> _shutdownCallbacks;
        std::thread* _thread = nullptr;

        std::function<void()> wrap(const std::function<int()>& body);
    };


    template <typename T>
    class IContextualStorage {
    public:
        virtual ~IContextualStorage() = default;

        virtual T produceNewForContext() = 0;

        virtual void disposeOfElementForContext(T) = 0;

        virtual T getFromContext();
    protected:
        std::map<std::thread::id, T> _ctxStore;
        std::mutex _ctxStoreMutex;
    };


    template <typename T>
    class IContextualRef {
    public:
        explicit IContextualRef(IContextualStorage<T*>* store) : _store(store) {}

        virtual ~IContextualRef() = default;

        virtual T* get() {
            return _store->getFromContext();
        }

        T* operator->() { return get(); }

    protected:
        IContextualStorage<T*>* _store;
    };


    class Framework {
    public:
        Framework(Framework& other) = delete;  // don't allow cloning
        void operator=(const Framework&) = delete;  // don't allow assigning

        static void boot();

        static IThreadContext* newThread(const std::function<int()>& body) {
            std::unique_lock<std::mutex> m(_mutex);

            auto id = _lastThreadID++;
            auto ctx = new ThreadContext(id, body);
            _contexts[id] = ctx;
            return ctx;
        }

        static void tickThreads() {
            std::unique_lock<std::mutex> m(_mutex);
            auto mapCopy = _contexts;
            m.unlock();

            for ( auto pair : mapCopy ) {
                if ( pair.second->isReadyForJoin() ) {
                    pair.second->baseThread()->join();
                    m.lock();
                    _contexts.erase(pair.first);
                    m.unlock();
                }
            }
        }

        static IThreadContext* context() {
            std::lock_guard<std::mutex> m(_mutex);

            // Map the correct thread ID
            auto idIter = _threadMap.find(std::this_thread::get_id());
            if ( idIter == _threadMap.end() ) {
                throw ForeignThreadException();
            }

            // Look up the context for the mapped thread ID
            auto id = idIter->second;
            return _contexts[id];
        }

        static bool isThread() {
            return !isMainPID();
        }

        static bool hasThreads() {
            return _contexts.size() > 1;
        }

        static std::string getThreadDisplay() {
            auto ctx = context();
            std::ostringstream oss;
            if ( ctx->getID() == 0 ) {
                oss << "main";
            } else {
                oss << "worker_" << ctx->getID();
            }
            return oss.str();
        }

        static bool isMainPID() {
            std::lock_guard<std::mutex> m(_mutex);
            return std::this_thread::get_id() == _mainPID;
        }

        static void shutdown() {
            std::lock_guard<std::mutex> m(_mutex);
            if ( !_booted ) return;
            for ( const auto& c : _shutdownCallbacks ) c();
        }

        static void onShutdown(std::function<void()> f) {
            std::lock_guard<std::mutex> m(_mutex);
            _shutdownCallbacks.push_back(f);
        }

    protected:
        Framework() = default;
        static ThreadID _lastThreadID;
        static bool _booted;
        static std::thread::id _mainPID;
        static std::vector<std::function<void()>> _shutdownCallbacks;
        static std::mutex _mutex;
        static std::map<ThreadID, IThreadContext*> _contexts;
        static std::map<std::thread::id, ThreadID> _threadMap;

        friend class ThreadContext;
    };


    // TODO: better date class
    inline std::string timestamp() {
        std::stringstream s;
        auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        auto local = *localtime(&now);

        s << local.tm_year + 1900 << "-";
        s << local.tm_mon + 1 << "-";
        s << local.tm_mday << " ";
        s << local.tm_hour << ":";
        s << local.tm_min << ":";
        s << local.tm_sec;

        return s.str();
    }


    /** ANSI-supported colors. */
    enum class ANSIColor: std::size_t {
        BLACK,
        GREY,
        RED,
        GREEN,
        YELLOW,
        BLUE,
        MAGENTA,
        CYAN,
        WHITE,
        RESET,
    };

    inline std::string s(ANSIColor c) {
        if ( c == ANSIColor::BLACK ) return "ANSIColor(BLACK)";
        if ( c == ANSIColor::GREY ) return "ANSIColor(GREY)";
        if ( c == ANSIColor::RED ) return "ANSIColor(RED)";
        if ( c == ANSIColor::GREEN ) return "ANSIColor(GREEN)";
        if ( c == ANSIColor::YELLOW ) return "ANSIColor(YELLOW)";
        if ( c == ANSIColor::BLUE ) return "ANSIColor(BLUE)";
        if ( c == ANSIColor::MAGENTA ) return "ANSIColor(MAGENTA)";
        if ( c == ANSIColor::CYAN ) return "ANSIColor(CYAN)";
        if ( c == ANSIColor::WHITE ) return "ANSIColor(WHITE)";
        if ( c == ANSIColor::RESET ) return "ANSIColor(RESET)";
        return "ANSIColor(UNKNOWN)";
    }


    /** Log message verbosity levels. */
    enum class Verbosity: std::size_t {
        SUCCESS = 6,
        ERROR = 5,
        WARNING = 4,
        INFO = 3,
        DEBUG = 2,
        VERBOSE = 1,
        TRACE = 0,
    };

    inline std::string s(Verbosity v) {
        if ( v == Verbosity::SUCCESS ) return "Verbosity(SUCCESS)";
        if ( v == Verbosity::ERROR ) return "Verbosity(ERROR)";
        if ( v == Verbosity::WARNING ) return "Verbosity(WARNING)";
        if ( v == Verbosity::INFO ) return "Verbosity(INFO)";
        if ( v == Verbosity::DEBUG ) return "Verbosity(DEBUG)";
        if ( v == Verbosity::VERBOSE ) return "Verbosity(VERBOSE)";
        if ( v == Verbosity::TRACE ) return "Verbosity(TRACE)";
        return "Verbosity(UNKNOWN)";
    }


    /** An instance which can have log messages pushed to it. */
    class ILogTarget : public IStringable {
    public:
        virtual void output(Verbosity, std::string) = 0;
    };


    class Logger : public IStringable {
    public:
        explicit Logger(std::string tag) : _tag(std::move(tag)) {}

        ~Logger() override = default;

        virtual void output(Verbosity, const std::string& p);

        /** Output an error message. */
        virtual void error(const std::string& p) {
            output(Verbosity::ERROR, p);
        }

        /** Output an error message. */
        virtual void success(const std::string& p) {
            output(Verbosity::SUCCESS, p);
        }

        /** Output a warning message. */
        virtual void warn(const std::string& p) {
            output(Verbosity::WARNING, p);
        }

        /** Output an informational message. */
        virtual void info(const std::string& p) {
            output(Verbosity::INFO, p);
        }

        /** Output a debugging message. */
        virtual void debug(const std::string& p) {
            output(Verbosity::DEBUG, p);
        }

        /** Output a verbose message. */
        virtual void verbose(const std::string& p) {
            output(Verbosity::VERBOSE, p);
        }

        /** Output a trace message. */
        virtual void trace(const std::string& p) {
            output(Verbosity::TRACE, p);
        }

        [[nodiscard]] std::string toString() const override {
            return "nslib::Logger<" + _tag + ">";
        }
    protected:
        std::string _tag;
    };


    class Logging : public IStringable {
    public:
        Logging(Logging& other) = delete;  // don't allow cloning
        void operator=(const Logging&) = delete;  // don't allow assigning

        static Logging* get() {
            if ( _inst == nullptr ) {
                _inst = new Logging();
                Framework::onShutdown([]() {
                    Logging::cleanup();
                });
            }

            return _inst;
        }

        static void useTimestamps(bool v) {
            _useTimestamps = v;
        }

        [[nodiscard]] Verbosity getVerbosity() const {
            return _verb;
        }

        void setVerbosity(Verbosity v) {
            _verb = v;
        }

        void addTarget(ILogTarget* target) {
            _targets.push_back(target);
        }

        Logger* get(const std::string& name) {
            auto res = _loggers.find(name);
            if ( res != _loggers.end() ) return res->second;

            auto inst = new Logger(name);
            _loggers[name] = inst;
            return inst;
        }

        [[nodiscard]] std::string toString() const override {
            return "nslib::Logging<>";
        }

        virtual void output(Verbosity v, const std::string& p) {
            if ( !shouldOutput(v) ) return;
            for ( auto target : _targets ) target->output(v, format(p));
        }

        virtual void output(const std::string& tag, Verbosity v, const std::string& p) {
            if ( !shouldOutput(v) ) return;
            if ( _allowList && std::find(_configuredTags.begin(), _configuredTags.end(), tag) == _configuredTags.end() ) return;
            if ( !_allowList && std::find(_configuredTags.begin(), _configuredTags.end(), tag) != _configuredTags.end() ) return;
            for ( auto target : _targets ) target->output(v, format(tag, p));
        }

        virtual void onlyEnabledLoggers() {
            _allowList = true;
        }

        virtual void configureLoggerTag(const std::string& name) {
            _configuredTags.push_back(name);
        }
    protected:
        static inline Logging* _inst = nullptr;
        static inline bool _useTimestamps = false;

        static void cleanup() {
            if ( _inst == nullptr ) return;
            for ( const auto& e : _inst->_loggers ) delete e.second;
            _inst->_loggers.clear();
            _inst->_targets.clear();
            delete _inst;
            _inst = nullptr;
        }

        Logging() = default;
        Verbosity _verb = Verbosity::INFO;
        std::map<std::string, Logger*> _loggers;
        std::list<ILogTarget*> _targets;

        bool _allowList = false;
        std::list<std::string> _configuredTags;

        static std::string format(const std::string& p) {
            return p;
        }

        static std::string format(const std::string& tag, const std::string& p) {
            return (_useTimestamps ? timestamp() + ": " : "") + "[" + tag + "] " + format(p);
        }

        /** Returns true if the given verbosity should be displayed. */
        [[nodiscard]] bool shouldOutput(Verbosity v) const {
            return v >= _verb;
        }
    };


    class IUsesLogger {
    public:
        explicit IUsesLogger(const std::string& tag) {
            logger = Logging::get()->get(tag);
        }

        virtual ~IUsesLogger() = default;
    protected:
        Logger* logger;
    };


    class LogFileTarget : public ILogTarget {
    public:
        explicit LogFileTarget(const std::string& path) : _path(path) {
            _fh.open(path);
        }

        void output(Verbosity v, std::string s) override {
            _fh << format(v, s) << std::endl;
        }

        [[nodiscard]] std::string toString() const override {
            return "LogFileTarget<path: " + _path + ">";
        }

        static std::string format(Verbosity v, const std::string& s) {
            if ( v == Verbosity::SUCCESS ) return " success " + s + "\n";
            if ( v == Verbosity::ERROR ) return " error   " + s + "\n";
            if ( v == Verbosity::WARNING ) return " warning " + s + "\n";
            if ( v == Verbosity::INFO ) return " info    " + s + "\n";
            if ( v == Verbosity::DEBUG ) return " debug   " + s + "\n";
            if ( v == Verbosity::VERBOSE ) return " verbose " + s + "\n";
            if ( v == Verbosity::TRACE ) return " trace   " + s + "\n";
            return s + "\n";
        }

    protected:
        std::string _path;
        std::ofstream _fh;
    };


    /** A more convenient to_string function. */
    inline std::string s(const std::string& v) { return v; }
    inline std::string s(const char* v) { return v; }
    inline std::string s(int v) { return std::to_string(v); }
    inline std::string s(long v) { return std::to_string(v); }
    inline std::string s(long long v) { return std::to_string(v); }
    inline std::string s(unsigned v) { return std::to_string(v); }
    inline std::string s(unsigned long v) { return std::to_string(v); }
    inline std::string s(unsigned long long v) { return std::to_string(v); }
    inline std::string s(float v) { return std::to_string(v); }
    inline std::string s(double v) { return std::to_string(v); }
    inline std::string s(long double v) { return std::to_string(v); }
    inline std::string s(bool v) { return v ? "true" : "false"; }
    inline std::string s(const std::type_info& v) { return v.name(); }
    inline std::string s(IStringable& v) { return v.toString(); }
    inline std::string s(IStringable* v) {
        if ( v == nullptr ) return "(nullptr)";
        return v->toString();
    }

    template <typename T>
    inline std::string s(std::optional<T> v) {
        if ( v == std::nullopt ) return "(none)";
        return s(*v);
    }

    inline std::string s(std::string_view v) { return std::string(v); }

    template <typename T>
    constexpr auto getTypeName() -> std::string_view {
#if defined(__clang__)
        constexpr auto prefix = std::string_view{"[T = "};
        constexpr auto suffix = "]";
        constexpr auto function = std::string_view{__PRETTY_FUNCTION__};
#elif defined(__GNUC__)
        constexpr auto prefix = std::string_view{"with T = "};
        constexpr auto suffix = "; ";
        constexpr auto function = std::string_view{__PRETTY_FUNCTION__};
#elif defined(_MSC_VER)
        constexpr auto prefix = std::string_view{"getTypeName<"};
        constexpr auto suffix = ">(void)";
        constexpr auto function = std::string_view{__FUNCSIG__};
#else
# error Unsupported compiler
#endif
        const auto start = function.find(prefix) + prefix.size();
        const auto end = function.find(suffix);
        const auto size = end - start;

        return function.substr(start, size);
    }


    namespace str {
        /** Trim whitespace from the front/back of a string. */
        inline std::string trim(const std::string& input) {
            auto firstRealIdx = input.find_first_not_of("\n\t ");
            if ( std::string::npos == firstRealIdx ) return "";
            auto lastRealIdx = input.find_last_not_of("\n\t ");
            return input.substr(firstRealIdx, (lastRealIdx - firstRealIdx + 1));
        }

        /** Pad the end of a string to the given length. */
        inline std::string pad(const std::string& text, unsigned int pad, const std::string& with = " ") {
            std::stringstream s;
            s << text;

            unsigned int pad_diff = pad - text.length();
            for ( unsigned int i = 0; i < pad_diff; i++ ) {
                s << with;
            }

            return s.str();
        }

        /** Pad the front of a string to the given length. */
        inline std::string padFront(const std::string& text, unsigned int pad, const std::string& with = " ") {
            if ( pad < 1 ) return text;
            std::stringstream s;

            unsigned int pad_diff = pad - text.length();
            for ( unsigned int i = 0; i < pad_diff; i++ ) {
                s << with;
            }

            s << text;
            return s.str();
        }

        /** Pad a string on both ends to the given length. */
        inline std::string padCenter(std::string text, unsigned int padLen, const std::string& with = " ") {
            std::stringstream s;

            if ( text.length() >= padLen ) {
                return text;
            }

            bool bit = false;
            unsigned int lhs = padLen - text.length();
            if ( lhs % 2 == 1 ) { lhs -= 1; bit = true; }
            lhs /= 2;

            s << pad("", lhs, with);
            s << text;
            s << pad("", lhs, with);
            if ( bit ) {
                s << with;
            }

            return s.str();
        }

        /** Join a list of strings using a delimiter. */
        inline std::string join(std::string const& delimiter, std::vector<std::string> const& strs) {
            std::string buf;

            if ( !strs.empty() ) {
                buf.reserve(strs.size() * strs[0].size());
            }

            bool first = false;
            for ( const auto& str : strs ) {
                if ( first ) buf += delimiter;
                buf += str;
                first = true;
            }

            return buf;
        }

        /** Find and replace all instances of `find` with `replace` in `s`. */
        inline std::string replace(const std::string& s, const std::string& find, const std::string& replace) {
            std::string buf;
            std::size_t pos = 0;
            std::size_t prevPos;

            // Reserves rough estimate of final size of string.
            buf.reserve(s.size());

            while (true) {
                prevPos = pos;
                pos = s.find(find, pos);
                if (pos == std::string::npos)
                    break;
                buf.append(s, prevPos, pos - prevPos);
                buf += replace;
                pos += find.size();
            }

            buf.append(s, prevPos, s.size() - prevPos);
            return buf;
        }

        inline std::vector<std::string> split(const std::string& delimiter, std::string s) {
            std::vector<std::string> v;

            std::size_t i = 0;
            while ((i = s.find(delimiter)) != std::string::npos) {
                v.push_back(s.substr(0, i));
                s.erase(0, i + delimiter.length());
            }
            v.push_back(s);

            return v;
        }
    }


    inline std::string trace() {
        void* array[15];
        char** strings;

        auto size = backtrace(array, 15);
        strings = backtrace_symbols(array, size);

        if ( strings == nullptr ) {
            return "(unable to obtain trace)";
        }

        std::stringstream s;
        for ( std::size_t i = 0; i < size; i += 1 ) {
            s << strings[i] << "\n";
        }

        free(strings);
        return s.str();
    }


    template<typename T>
    T* unlessNull(T* v, std::function<T*(T*)> f) {
        if ( v != nullptr ) return f(v);
        return f;
    }


    namespace stl {
        namespace priv {
            template <typename ElemT>
            void erase(std::stack<ElemT>& s, std::function<bool(std::size_t, ElemT)> d, std::size_t current) {
                if ( s.empty() ) {
                    return;
                }

                auto elem = s.top();
                if ( d(current, elem) ) {
                    s.pop();
                    return;
                }

                s.pop();
                erase(s, d, current + 1);
                s.push(elem);
            }
        }

        template<typename ElemT>
        void erase(std::stack<ElemT>& s, std::function<bool(std::size_t, ElemT)> d) {
            priv::erase<ElemT>(s, d, 0);
        }

        template<typename ElemT>
        void erase(std::stack<ElemT>& s, std::size_t idx) {
            priv::erase<ElemT>(s, [idx](std::size_t current, ElemT) {
                return current == idx;
            }, 0);
        }

        inline std::string readStreamContents(std::istream& fh) {
            std::stringstream ss;
            std::string s;
            while ( std::getline(fh, s) ) ss << s;
            return ss.str();
        }

        template<typename ElemT>
        inline void stackLoop(std::stack<ElemT> s, std::function<void(ElemT)> fn) {
            while ( !s.empty() ) {
                auto elem = s.top();
                s.pop();
                fn(elem);
            }
        }

        template<typename InputT, typename OutputT>
        inline std::vector<OutputT> map(std::vector<InputT> v, std::function<OutputT(InputT)> f) {
            std::vector<OutputT> out;
            std::transform(v.begin(), v.end(), out, f);
            return out;
        }

        template<class ContainerT, class ElemT>
        bool contains(ContainerT c, const ElemT& v) {
            return std::find(c.begin(), c.end(), v) != c.end();
        }

        template <typename ElemT>
        ElemT popFront(std::vector<ElemT>& vec) {
            assert(!vec.empty());
            auto elem = *vec.front();
            vec.erase(vec.begin());
            return elem;
        }
    }


    /* An impoverished IPC message passing system. */
    namespace ipc {
        namespace priv {
            struct StringSegment {
                sem_t mutex;
                std::size_t size;
                int id;
                bool written;
                bool read;
                bool exit;
            };

            inline void initSharedEntry(StringSegment* ptr) {
                sem_init(&ptr->mutex, 0, 1);
                ptr->size = 0;
                ptr->id = 0;
                ptr->written = false;
                ptr->read = false;
                ptr->exit = false;
            }
        }


        /** Repeatedly checks for a condition. If the condition is false, non-blocking sleep for a period before rechecking. */
        class Sleeper : public IStringable {
        public:
            explicit Sleeper(std::function<bool()> check, std::size_t sleepMs = 10) : _check(std::move(check)), _sleepMs(sleepMs) {}

            void waitFor() {
                while ( !_check() ) std::this_thread::sleep_for(std::chrono::milliseconds(_sleepMs));
            }

            [[nodiscard]] std::string toString() const override {
                return "ipc::Sleeper<ms: " + s(_sleepMs) + ">";
            }
        protected:
            std::function<bool()> _check;
            std::size_t _sleepMs;
        };


        /** Publishes messages to shared memory for an IPC to read. */
        class Publisher : public IStringable {
        public:
            explicit Publisher(std::string path, std::size_t sleepMs = 10) : _path(std::move(path)) {
                _id = shmget(IPC_PRIVATE, sizeof(priv::StringSegment), S_IRUSR|S_IWUSR);
                _segment = (priv::StringSegment*) shmat(_id, nullptr, 0);

                priv::initSharedEntry(_segment);

                _read = new Sleeper([this]() {
                    sem_wait(&_segment->mutex);
                    bool result = _segment->read;
                    sem_post(&_segment->mutex);
                    return result;
                }, sleepMs);

                std::ofstream of;
                of.open(_path);
                of << _id;
                of.close();
            }

            ~Publisher() override {
                if ( !_exited ) exit();
//                _read->waitFor();
                sem_destroy(&_segment->mutex);
                shmdt(_segment);
                shmctl(_id, IPC_RMID, nullptr);
                delete _read;
                unlink(_path.c_str());
            }

            void publish(const std::string& value) {
                // Allocate the string in a new shared memory location
                auto cstr = value.c_str();
                auto id = shmget(IPC_PRIVATE, sizeof(cstr), S_IRUSR|S_IWUSR);
                char* segment = (char*) shmat(id, nullptr, 0);
                strcpy(segment, cstr);
                shmdt(segment);

                // Acquire the mutex
                sem_wait(&_segment->mutex);

                // If we have an existing value, make sure it has been read
                if ( _segment->written ) {
                    sem_post(&_segment->mutex);
                    _read->waitFor();
                    sem_wait(&_segment->mutex);

                    // Deallocate the old string
                    shmctl(_segment->id, IPC_RMID, nullptr);
                }

                // Update the shared data structure with the new string
                _segment->id = id;
                _segment->size = value.size();
                _segment->written = true;
                _segment->read = false;

                // Release the mutex
                sem_post(&_segment->mutex);
            }

            void exit(bool clean = true) {
                if ( _exited ) return;

                // Acquire the mutex
                sem_wait(&_segment->mutex);

                // If we have an existing value, make sure it has been read
                if ( _segment->written ) {
                    if ( clean ) {
                        sem_post(&_segment->mutex);
                        _read->waitFor();
                        sem_wait(&_segment->mutex);
                    }

                    // Deallocate the old string
                    shmctl(_segment->id, IPC_RMID, nullptr);
                }

                // Set the exit flag
                _segment->exit = true;
                _segment->written = true;
                _segment->read = false;

                // Release the mutex
                sem_post(&_segment->mutex);

                _exited = true;
            }

            [[nodiscard]] std::string toString() const override {
                return "ipc::Publisher<path: " + _path + ", id: " + s(_id) + ">";
            }
        protected:
            int _id = 0;
            priv::StringSegment* _segment;
            std::string _path;
            Sleeper* _read;
            bool _exited = false;
        };


        /** Receives messages published to shared memory by an IPC. */
        class Subscriber : public IStringable {
        public:
            Subscriber(std::string path, std::function<void(std::string)> handler, std::size_t sleepMs = 10) : _path(std::move(path)), _handler(std::move(handler)) {
                std::ifstream fh;
                fh.open(_path);
                _id = std::stoi(stl::readStreamContents(fh));
                _segment = (priv::StringSegment*) shmat(_id, nullptr, 0);

                _pending = new Sleeper([this]() {
                    sem_wait(&_segment->mutex);
                    auto result = _segment->written && !_segment->read;

                    // NOTE: don't release the mutex if there's pending data, since we'll immediately read it
                    if ( !result ) sem_post(&_segment->mutex);
                    return result;
                }, sleepMs);
            }

            ~Subscriber() override {
                shmdt(_segment);
            }

            void listenOnce() {
                if ( _exited ) return;

                // This acquires the mutex
                _pending->waitFor();

                // Check for the exit flag
                if ( _segment->exit ) {
                    _exited = true;
                    _segment->read = true;
                    sem_post(&_segment->mutex);
                    return;
                }

                // Attach the allocated string location
                auto segment = (char*) shmat(_segment->id, nullptr, 0);

                // Read the shared string
                std::string value(segment, _segment->size);

                // Detach the allocated string and mark it as read
                shmdt(segment);
                _segment->read = true;

                // Release the mutex
                sem_post(&_segment->mutex);

                // Execute the handler on the read string
                _handler(value);
            }

            void listen() {
                while ( !_exited ) listenOnce();
            }

            [[nodiscard]] std::string toString() const override {
                return "ipc::Subscriber<path: " + _path + ", exited: " + s(_exited) + ">";
            }

        protected:
            int _id;
            priv::StringSegment* _segment;
            std::string _path;
            std::function<void(std::string)> _handler;
            Sleeper* _pending;
            bool _exited = false;
        };
    }

    class Console;

    class ConsoleService : public IContextualStorage<Console*> {
    public:
        ConsoleService(ConsoleService& other) = delete;  // don't allow cloning
        void operator=(const ConsoleService&) = delete;  // don't allow assigning

        static ConsoleService* get() {
            if ( _inst == nullptr ) {
                _inst = new ConsoleService();
//                Framework::onShutdown([]() {
//                    Logging::cleanup();
//                });
            }

            return _inst;
        }

        static Console* getConsole() {
            return get()->getFromContext();
        }

        Console* produceNewForContext() override;

        void disposeOfElementForContext(Console*) override;
    protected:
        static inline ConsoleService* _inst = nullptr;

        ConsoleService() = default;
    };


    /**
     * Utility class for interacting with the Console window.
     */
    class Console : public ILogTarget {
    public:
        Console(Console& other) = delete;  // don't allow cloning
        void operator=(const Console&) = delete;  // don't allow assigning

        /** Get the singleton Console instance. */
        static Console* get() {
            return ConsoleService::getConsole();
        }

        /** Output a string at the given verbosity. Satisfies ILogTarget. */
        void output(Verbosity verb, std::string s) override {
            if ( verb == Verbosity::SUCCESS ) success(s);
            else if ( verb == Verbosity::ERROR ) error(s);
            else if ( verb == Verbosity::WARNING ) warn(s);
            else if ( verb == Verbosity::INFO ) info(s);
            else if ( verb == Verbosity::DEBUG ) debug(s);
            else if ( verb == Verbosity::VERBOSE ) verbose(s);
            else if ( verb == Verbosity::TRACE ) trace(s);
            else only(verb)->output(s + "\n")->end();
        }

        std::string toString() const override {
            return "nslib::Console<>";
        }

        /** Set the maximum verbosity that should be displayed. */
        Console* setVerbosity(Verbosity v) {
            _verb = v;
            return this;
        }

        /** Get the current verbosity. */
        Verbosity verbosity() { return _verb; }

        /** Set the background color. */
        Console* background(ANSIColor color) {
            if ( color == ANSIColor::BLACK ) output(ANSI_BACKGROUND_BLACK);
            if ( color == ANSIColor::GREY ) output(ANSI_BACKGROUND_GREY);
            if ( color == ANSIColor::RED ) output(ANSI_BACKGROUND_RED);
            if ( color == ANSIColor::GREEN ) output(ANSI_BACKGROUND_GREEN);
            if ( color == ANSIColor::YELLOW ) output(ANSI_BACKGROUND_YELLOW);
            if ( color == ANSIColor::BLUE ) output(ANSI_BACKGROUND_BLUE);
            if ( color == ANSIColor::MAGENTA ) output(ANSI_BACKGROUND_MAGENTA);
            if ( color == ANSIColor::CYAN ) output(ANSI_BACKGROUND_CYAN);
            if ( color == ANSIColor::WHITE ) output(ANSI_BACKGROUND_WHITE);
            if ( color == ANSIColor::RESET ) output(ANSI_RESET_BACKGROUND_COLOR);
            _cleanup.emplace([](Console* c) {
                c->output(c->ANSI_RESET_BACKGROUND_COLOR);
            });
            return this;
        }

        /** Set the foreground color. */
        Console* color(ANSIColor color) {
            if ( color == ANSIColor::BLACK ) output(ANSI_FOREGROUND_BLACK);
            if ( color == ANSIColor::GREY ) output(ANSI_FOREGROUND_GREY);
            if ( color == ANSIColor::RED ) output(ANSI_FOREGROUND_RED);
            if ( color == ANSIColor::GREEN ) output(ANSI_FOREGROUND_GREEN);
            if ( color == ANSIColor::YELLOW ) output(ANSI_FOREGROUND_YELLOW);
            if ( color == ANSIColor::BLUE ) output(ANSI_FOREGROUND_BLUE);
            if ( color == ANSIColor::MAGENTA ) output(ANSI_FOREGROUND_MAGENTA);
            if ( color == ANSIColor::CYAN ) output(ANSI_FOREGROUND_CYAN);
            if ( color == ANSIColor::WHITE ) output(ANSI_FOREGROUND_WHITE);
            if ( color == ANSIColor::RESET ) output(ANSI_RESET_FOREGROUND_COLOR);
            _cleanup.emplace([](Console* c) {
                c->output(c->ANSI_RESET_FOREGROUND_COLOR);
            });
            return this;
        }

        /** Start bolding text. */
        Console* bold() {
            output(ANSI_BOLD);
            _cleanup.emplace([](Console* c) {
                c->output(c->ANSI_RESET_BOLD);
            });
            return this;
        }

        /** Start underlining text. */
        Console* underline() {
            output(ANSI_UNDERLINE);
            _cleanup.emplace([](Console* c) {
                c->output(c->ANSI_RESET_UNDERLINE);
            });
            return this;
        }

        /** Invert background/foreground colors. */
        Console* invert() {
            output(ANSI_INVERSE);
            _cleanup.emplace([](Console* c) {
                c->output(c->ANSI_RESET_INVERSE);
            });
            return this;
        }

        /** Reset the last applied modifier. */
        Console* end() {
            if ( !_cleanup.empty() ) {
                _cleanup.top()(this);
                _cleanup.pop();
            }
            return this;
        }

        /** Reset all applied modifiers. */
        Console* reset() {
            while ( !_cleanup.empty() ) {
                _cleanup.top()(this);
                _cleanup.pop();
            }
            output(ANSI_RESET_ALL);
            return this;
        }

        /** Clear the viewport. */
        Console* clear() {
            output(ANSI_CLEAR_ALL);
            return this;
        }

        /** Output an error message. */
        Console* error(const std::string& p) {
            only(Verbosity::ERROR)
                ->color(ANSIColor::RED)
                ->print("   error ", true);

            if ( Framework::isThread() ) {
                only(Verbosity::ERROR)
                    ->print(" [" + Framework::getThreadDisplay() + "] ")
                    ->end();
            }

            return only(Verbosity::ERROR)
                ->output(p + "\n")
                ->end();
        }

        /** Output an error message. */
        Console* success(const std::string& p) {
            only(Verbosity::INFO)
                ->color(ANSIColor::GREEN)
                ->print(" success ", true);

            if ( Framework::isThread() ) {
                only(Verbosity::INFO)
                    ->print(" [" + Framework::getThreadDisplay() + "] ")
                    ->end();
            }

            return only(Verbosity::INFO)
                ->output(p + "\n")
                ->end();
        }

        /** Output a warning message. */
        Console* warn(const std::string& p) {
            only(Verbosity::WARNING)
                ->color(ANSIColor::YELLOW)
                ->print(" warning ", true);

            if ( Framework::isThread() ) {
                only(Verbosity::WARNING)
                    ->print(" [" + Framework::getThreadDisplay() + "] ")
                    ->end();
            }

            return only(Verbosity::WARNING)
                ->output(p + "\n")
                ->end();
        }

        /** Output an informational message. */
        Console* info(const std::string& p) {
            only(Verbosity::INFO)
                ->color(ANSIColor::BLUE)
                ->print("    info ", true);

            if ( Framework::isThread() ) {
                only(Verbosity::INFO)
                    ->print(" [" + Framework::getThreadDisplay() + "] ")
                    ->end();
            }

            return only(Verbosity::INFO)
                ->output(p + "\n")
                ->end();
        }

        /** Output a debugging message. */
        Console* debug(const std::string& p) {
            only(Verbosity::DEBUG)
                ->color(ANSIColor::MAGENTA)
                ->print("   debug ", true);

            if ( Framework::isThread() ) {
                only(Verbosity::DEBUG)
                    ->print(" [" + Framework::getThreadDisplay() + "] ")
                    ->end();
            }

            return only(Verbosity::DEBUG)
                ->output(p + "\n")
                ->end();
        }

        /** Output a verbose message. */
        Console* verbose(const std::string& p) {
            only(Verbosity::VERBOSE)
                ->color(ANSIColor::CYAN)
                ->print(" verbose ", true);

            if ( Framework::isThread() ) {
                only(Verbosity::VERBOSE)
                    ->print(" [" + Framework::getThreadDisplay() + "] ")
                    ->end();
            }

            return only(Verbosity::VERBOSE)
                ->output(p + "\n")
                ->end();
        }

        /** Output a trace message. */
        Console* trace(const std::string& p) {
            only(Verbosity::TRACE)
                ->color(ANSIColor::GREY)
                ->print("   trace ", true);

            if ( Framework::isThread() ) {
                only(Verbosity::TRACE)
                    ->print(" [" + Framework::getThreadDisplay() + "] ")
                    ->end();
            }

            return only(Verbosity::TRACE)
                ->output(p + "\n")
                ->end();
        }

        /** Print a string to the default output, without a newline. */
        Console* print(const std::string& text, bool shouldReset = false) {
            output(text);
            if ( shouldReset ) reset();
            return this;
        }

        /** Print a string to the default output, including a newline. */
        Console* println(const std::string& text = "", bool shouldReset = true) {
            output(text + "\n");
            if ( shouldReset ) reset();
            return this;
        }

        /** Print a pretty header. */
        Console* header(const std::string& text) {
            auto padded = str::padCenter(text, _vpWidth - 2);
            return println(box(padded), true);
        }

        /** Print a horizontal bar. */
        Console* hr() {
            auto hbar = str::pad("", _vpWidth, UNI_DOUBLE_LINE);
            return println("\n" + hbar + "\n", true);
        }

        /** Show a progress bar. `value` should be a decimal percentage from 0 to 1. */
        Console* progress(double value) {
            std::size_t width = _vpWidth - 5;

            std::size_t filled = static_cast<int>(static_cast<double>(width) * value);
            std::size_t percent = static_cast<int>(100 * value);
            if ( value >= 1 ) {
                percent = 100;
                filled = width;
            }

            auto bar = str::pad("", filled, UNI_BLOCK_FULL);
            auto space = str::pad("", width - filled, UNI_BLOCK_FULL);
            auto display = str::padFront(s(percent) + "%", 5);

            return output("\r")
                ->color(ANSIColor::GREEN)->output(bar)->reset()
                ->output(space)
                ->color(value >= 1 ? ANSIColor::WHITE : ANSIColor::GREEN)->output(display)->reset()
                ->flush();
        }

        /** Flush pending output to the streams. */
        Console* flush() {
            if ( !_isCapturing ) {
                std::cout.flush();
                std::cerr.flush();
            }
            return this;
        }

        /** Temporarily mute output. */
        Console* mute() {
            _muteStack += 1;
            _cleanup.emplace([](Console* c) {
                c->_muteStack -= 1;
            });
            return this;
        }

        /** Temporarily limit output to the given verbosity. */
        Console* only(Verbosity v) {
            _verbLimits.push(v);
            _cleanup.emplace([](Console* c) {
                c->_verbLimits.pop();
            });
            return this;
        }

        /** Temporarily limit output to DEBUG verbosity. */
        Console* debug() {
            return only(Verbosity::DEBUG);
        }

        /** Expand the width of the viewport to match the user's window. */
        Console* grow() {
            struct winsize w{};
            ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
            _vpWidth = w.ws_col;
            return this;
        }

        /** Temporarily capture output to an internal buffer. */
        Console* capture() {
            _isCapturing = true;
            return this;
        }

        /** Stop capturing output and return the contents of the internal buffer. */
        std::string endCapture() {
            _isCapturing = false;
            auto s = _capture.str();
            _capture = std::stringstream();
            return s;
        }

        /** Surround the given text in a unicode box. */
        static std::string box(const std::string& text) {
            auto inst = Console::get();
            std::stringstream s;
            std::string hbar = str::pad("", text.length(), inst->UNI_BAR_HORIZ);
            s << inst->UNI_BAR_TOP_LEFT << hbar << inst->UNI_BAR_TOP_RIGHT << '\n';
            s << inst->UNI_BAR_VERT << text << inst->UNI_BAR_VERT << '\n';
            s << inst->UNI_BAR_BOTTOM_LEFT << hbar << inst->UNI_BAR_BOTTOM_RIGHT;
            return s.str();
        }

        std::string prompt(const std::string& message) {
            println()
                ->color(ANSIColor::GREEN)->println("    " + message)
                ->print("    > ");

            std::cin >> std::noskipws;

            std::string check;
            check = std::cin.peek();

            println();
            if ( check == "\n" ) {
                std::string dump;
                std::getline(std::cin, dump);
                return "";
            }

            std::string input;
            std::cin >> input;

            std::string dump;
            std::getline(std::cin, dump);

            return input;
        }

        std::string prompt(const std::string& message, const std::string& def) {
            println()
                    ->color(ANSIColor::GREEN)->print("    " + message)->reset()
                    ->print(" [")->color(ANSIColor::YELLOW)->print(def)->reset()->println("]")
                    ->print("    > ");

            std::cin >> std::noskipws;

            std::string check;
            check = std::cin.peek();

            println();
            if ( check == "\n" ) {
                std::string dump;
                std::getline(std::cin, dump);
                return def;
            }

            std::string input;
            std::cin >> input;

            std::string dump;
            std::getline(std::cin, dump);

            return input;
        }

        std::string choice(const std::string& message, const std::vector<std::string>& options) {
            println()
                    ->color(ANSIColor::GREEN)->println("    " + message + "\n");

            std::size_t idx = 0;
            for ( auto iter = options.begin(); iter != options.end(); ++iter, idx++ ) {
                print("    [")->color(ANSIColor::YELLOW)->print(s(idx), true)->println("] " + (*iter));
            }

            println()->print("    > ");

            std::cin >> std::noskipws;

            std::string check;
            check = std::cin.peek();

            println();
            if ( check == "\n" ) {
                std::string dump;
                std::getline(std::cin, dump);
                color(ANSIColor::RED)->println("Invalid selection.");
                return choice(message, options);
            }

            std::string inputStr;
            std::cin >> inputStr;

            bool inputValid = true;
            std::size_t input = 0;
            try {
                input = std::stoi(inputStr);
            } catch (std::invalid_argument&) {
                inputValid = false;
            }

            if ( !inputValid || input >= options.size() ) {
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                color(ANSIColor::RED)->println("Invalid selection.");
                return choice(message, options);
            }

            std::string dump;
            std::getline(std::cin, dump);

            return options[input];
        }

        std::string choice(const std::string& message, const std::vector<std::string>& options, const std::string& def) {
            println()
                    ->color(ANSIColor::GREEN)->print("    " + message)->reset()
                    ->print(" [")->color(ANSIColor::YELLOW)->print(def)->reset()->println("]\n");

            std::size_t idx = 0;
            for ( auto iter = options.begin(); iter != options.end(); ++iter, idx++ ) {
                print("    [")->color(ANSIColor::YELLOW)->print(s(idx), true)->println("] " + (*iter));
            }

            println()->print("    > ");

            std::cin >> std::noskipws;

            std::string check;
            check = std::cin.peek();

            println();
            if ( check == "\n" ) {
                std::string dump;
                std::getline(std::cin, dump);
                return def;
            }

            std::string inputStr;
            std::cin >> inputStr;

            bool inputValid = true;
            std::size_t input = 0;
            try {
                input = std::stoi(inputStr);
            } catch (std::invalid_argument&) {
                inputValid = false;
            }

            if ( !inputValid || input >= options.size() ) {
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                color(ANSIColor::RED)->println("Invalid selection.");
                return choice(message, options, def);
            }

            std::string dump;
            std::getline(std::cin, dump);

            return options[input];
        }


        /* ================================ */
        /*        ANSI/Unicode Codes        */
        /* ================================ */
        std::string ANSI_RESET_ALL = "\033[0m";
        std::string ANSI_CLEAR_ALL = "\x1b[2J\x1b[1;1H";
        std::string ANSI_BOLD = "\033[1m";
        std::string ANSI_RESET_BOLD = "\033[22m";
        std::string ANSI_UNDERLINE = "\033[4m";
        std::string ANSI_RESET_UNDERLINE = "\033[24m";
        std::string ANSI_INVERSE = "\033[7m";
        std::string ANSI_RESET_INVERSE = "\033[27m";
        std::string ANSI_FOREGROUND_BLACK = "\033[30m";
        std::string ANSI_FOREGROUND_GREY = "\033[90m";
        std::string ANSI_FOREGROUND_RED = "\033[31m";
        std::string ANSI_FOREGROUND_GREEN = "\033[32m";
        std::string ANSI_FOREGROUND_YELLOW = "\033[33m";
        std::string ANSI_FOREGROUND_BLUE = "\033[34m";
        std::string ANSI_FOREGROUND_MAGENTA = "\033[35m";
        std::string ANSI_FOREGROUND_CYAN = "\033[36m";
        std::string ANSI_FOREGROUND_WHITE = "\033[37m";
        std::string ANSI_RESET_FOREGROUND_COLOR = "\033[39m";
        std::string ANSI_BACKGROUND_BLACK = "\033[40m";
        std::string ANSI_BACKGROUND_GREY = "\033[100m";
        std::string ANSI_BACKGROUND_RED = "\033[41m";
        std::string ANSI_BACKGROUND_GREEN = "\033[42m";
        std::string ANSI_BACKGROUND_YELLOW = "\033[43m";
        std::string ANSI_BACKGROUND_BLUE = "\033[44m";
        std::string ANSI_BACKGROUND_MAGENTA = "\033[45m";
        std::string ANSI_BACKGROUND_CYAN = "\033[46m";
        std::string ANSI_BACKGROUND_WHITE = "\033[47m";
        std::string ANSI_RESET_BACKGROUND_COLOR = "\033[49m";
        std::string UNI_BAR_VERT = "│";
        std::string UNI_BAR_HORIZ = "─";
        std::string UNI_BAR_TOP_LEFT = "┌";
        std::string UNI_BAR_TOP_RIGHT = "┐";
        std::string UNI_BAR_BOTTOM_LEFT = "└";
        std::string UNI_BAR_BOTTOM_RIGHT = "┘";
        std::string UNI_BAR_LEFT_ROW_JOIN = "├";
        std::string UNI_BAR_RIGHT_ROW_JOIN = "┤";
        std::string UNI_BAR_TOP_COL_JOIN = "┬";
        std::string UNI_BAR_BOTTOM_COL_JOIN = "┴";
        std::string UNI_BAR_CENTER_JOIN = "┼";
        std::string UNI_DOUBLE_LINE = "═";
        std::string UNI_BLOCK_FULL = "█";

    protected:
        static std::optional<std::thread::id> _mainPID;
        static std::map<std::thread::id, Console*> _threadCopies;

        /** The global console instance. */
        static Console* _global;

        /** New consoles cannot be made outside this class. */
        Console() = default;

        /** The current verbosity level. */
        Verbosity _verb = Verbosity::INFO;

        /** The current width of the viewport. */
        std::size_t _vpWidth = 70;

        /** True if we are capturing output into an internal buffer. */
        bool _isCapturing = false;

        /** Internal buffer of captured output. */
        std::stringstream _capture;

        /** List of reset codes that clear applied modifiers, in order. */
        std::stack<std::function<void(Console*)>> _cleanup;

        std::stack<Verbosity> _verbLimits;

        /** Number of layers the Console is muted. */
        std::size_t _muteStack = 0;

        /** Output a string to the default stream. */
        Console* output(const std::string& v) {
            if ( _muteStack > 0 ) return this;
            if ( !_verbLimits.empty() && _verbLimits.top() < _verb ) return this;
            auto& stream = getStreamFor(Verbosity::INFO);
            stream << v;
            return this;
        }

        /** Returns true if the given verbosity should be displayed. */
        bool shouldOutput(Verbosity v) const {
            if ( _muteStack > 0 ) return false;
            if ( !_verbLimits.empty() ) return v >= _verbLimits.top();
            return v >= _verb;
        }

        /** Get the stream used to show messages of the given verbosity. */
        std::ostream& getStreamFor(Verbosity verb) {
            if ( _isCapturing ) return _capture;
            if ( verb == Verbosity::ERROR || verb == Verbosity::WARNING ) return std::cerr;
            return std::cout;
        }

        friend class ConsoleService;
    };

    class IUsesConsole {
    public:
        virtual ~IUsesConsole() = default;

        IUsesConsole() : console(ConsoleService::get()) {}
    protected:
        IContextualRef<Console> console;
    };


    class ConsoleTarget : public ILogTarget, public IUsesConsole {
    public:
        explicit ConsoleTarget() = default;

        void output(Verbosity v, std::string s) override {
            if ( v == Verbosity::SUCCESS ) console->success(s);
            else if ( v == Verbosity::ERROR ) console->error(s);
            else if ( v == Verbosity::WARNING ) console->warn(s);
            else if ( v == Verbosity::INFO ) console->info(s);
            else if ( v == Verbosity::DEBUG ) console->debug(s);
            else if ( v == Verbosity::VERBOSE ) console->verbose(s);
            else if ( v == Verbosity::TRACE ) console->trace(s);
            else console->println(" ??????? " + s);
        }

        [[nodiscard]] std::string toString() const override {
            return "ConsoleTarget<>";
        }
    };


    /** Get a UUIDv4-compatible string. */
    inline std::string uuid() {
        if ( priv::USE_DETERMINISTIC_UUIDS ) {
            return "d-guid-" + s(priv::DETERMINISTIC_UUID_LAST++);
        }

        int i;
        std::stringstream s;

        s << std::hex;
        for ( i = 0; i < 8; i++ ) s << priv::distribution(priv::generator);

        s << "-";
        for ( i = 0; i < 4; i++ ) s << priv::distribution(priv::generator);

        s << "-4";
        for ( i = 0; i < 3; i++ ) s << priv::distribution(priv::generator);

        s << "-" << priv::distribution2(priv::generator);
        for ( i = 0; i < 3; i++ ) s << priv::distribution(priv::generator);

        s << "-";
        for ( i = 0; i < 12; i++ ) s << priv::distribution(priv::generator);

        return s.str();
    }

    /** Get a random double between 0 and 1. */
    inline double rand() {
        return priv::realDistribution(priv::eng);
    }

    /** Returns true if the given path exists. */
    inline bool file_exists(const std::string& path) {
        struct stat s{};
        return stat(path.c_str(), &s) == 0;
    }


    /** A factory-pattern serialization library. */
    namespace serial {
        /** Each child-class has a unique tag. */
        using tag_t = std::string;

        /** Helper interface for a class which can be serialized */
        class ISerializable {
        public:
            [[nodiscard]] virtual tag_t getSerialKey() const = 0;

            [[nodiscard]] virtual binn* getExtraSerialData() const { return binn_map(); }

            virtual void loadExtraSerialData(binn*) {}
        };

        namespace priv {
            /** Concept uniting the ISerializable interface w/ some other Class */
            template <typename Class, class T>
            concept Serializable = requires(T& obj) {
                { std::is_base_of_v<ISerializable, T> };
                { std::is_base_of_v<Class, T> };
            };
        }

        /** Thrown by Factory when a conflicting producer/reducer is registered. */
        class DuplicateTagException : public NSLibException {
        public:
            explicit DuplicateTagException(const tag_t& tag) : NSLibException("Cannot register duplicate tag with Factory: " + s(tag)) {}
        };

        /** Thrown by Factory when no producer can be found for a tag. */
        template <typename Class>
        class MissingProducerError : public NSLibException {
        public:
            explicit MissingProducerError(const tag_t& tag) : NSLibException("Cannot find a producer yielding type " + s(getTypeName<Class>()) + " for tag " + tag) {}
        };

        /** Thrown by Factory when no reducer can be found for a tag. */
        template <typename Class>
        class MissingReducerError : public NSLibException {
        public:
            explicit MissingReducerError(const tag_t& tag) : NSLibException("Cannot find a reducer accepting type " + s(getTypeName<Class>()) + " for tag " + tag) {}
        };

        /**
         * The boss. Collects functions mapping some tag_t tag between binn* and child
         * classes of Class.
         * @tparam Class
         */
        template <typename Class, typename Passthrough>
        class Factory : public IStringable {
        public:
            Factory() = default;

            using Producer = std::function<Class*(binn*, Passthrough)>;
            using Reducer = std::function<binn*(const Class*, Passthrough)>;

            /** Register a function which instantiates Class instances w/ the given tag. */
            void registerProducer(tag_t tag, Producer producer) {
                if ( hasProducer(tag) ) {
                    throw DuplicateTagException(tag);
                }

                _producers.insert({ tag, producer });
            }

            /** Register a function which creates binn* instances for Classes w/ the given tag. */
            void registerReducer(tag_t tag, Reducer producer) {
                if ( hasReducer(tag) ) {
                    throw DuplicateTagException(tag);
                }

                _reducers.insert({ tag, producer });
            }

            /** True if a producer w/ that tag has been registered. */
            [[nodiscard]] bool hasProducer(tag_t tag) {
                return _producers.find(tag) != _producers.end();
            }

            /** True if a reducer w/ that tag has been registered. */
            [[nodiscard]] bool hasReducer(tag_t tag) {
                return _reducers.find(tag) != _reducers.end();
            }

            /** Load a Class instance from the serialized data. */
            [[nodiscard]] Class* produce(binn* data, Passthrough p) {
                tag_t tag = binn_map_str(data, NSLIB_SERIAL_TAG);
                auto iter = _producers.find(tag);
                if ( iter == _producers.end() ) {
                    throw MissingProducerError<Class>(tag);
                }

                auto obj = (binn*) binn_map_object(data, NSLIB_SERIAL_DATA); // FIXME?
                return ((*iter).second)(obj, p);
            }

            /** Serialize an object. */
            [[nodiscard]] binn* reduce(priv::Serializable<Class> auto obj, Passthrough p) {
                return reduce(obj->getSerialKey(), obj, p);
            }

            /** Serialize an object w/ the given tag. */
            [[nodiscard]] binn* reduce(tag_t tag, const Class* obj, Passthrough p) {
                auto iter = _reducers.find(tag);
                if ( iter == _reducers.end() ) {
                    throw MissingReducerError<Class>(tag);
                }

                auto data = ((*iter).second)(obj, p);
                auto binn = binn_map();
                binn_map_set_str(binn, NSLIB_SERIAL_TAG, strdup(tag.c_str()));
                binn_map_set_object(binn, NSLIB_SERIAL_DATA, data);
                return binn;
            }

            [[nodiscard]] std::string toString() const override {
                return "nslib::serial::Factory<>";
            }
        protected:
            std::map<tag_t, Producer> _producers;
            std::map<tag_t, Reducer> _reducers;
        };
    }

    /** A simple library for loading dynamic modules. */
    namespace dynamic {
        /** The type of a handle to a loaded module. */
        using import_t = void*;

        class ModuleException : public NSLibException {
        public:
            explicit ModuleException(const std::string& path, const std::string& reason) : NSLibException("Unable to load module at path (" + path + "): " + reason) {}
        };

        /**
         * Loads & manages a dynamic module. This module exports a function that,
         * when called, returns a pointer to an instance of type T.
         * @tparam T
         */
        template <typename T>
        class Module : public IStringable {
        public:
            /**
             * @param path The FS path to the compiled module
             * @param factorySymbol The name of the function that produces T* (i.e. of type () => T*)
             */
            explicit Module(std::string path, std::string factorySymbol = "factory") :
                _path(std::move(path)), _factorySymbol(std::move(factorySymbol)) {}

            ~Module() override {
                if ( _handle != nullptr ) {
                    dlclose(_handle);
                    _handle = nullptr;
                }
            }

            /** Instantiate T* from the module by calling the factory function. */
            [[nodiscard]] T* produce() {
                if ( _handle == nullptr ) {
                    open();
                }

                auto factory = dlsym(_handle, _factorySymbol.c_str());
                if ( factory == nullptr ) {
                    throw ModuleException(_path, "Could not open the factory symbol (" + _factorySymbol + ")");
                }

                typedef T* (*factory_t)();
                auto factory_f = reinterpret_cast<factory_t>(reinterpret_cast<import_t>(factory));
                return factory_f();
            }

            [[nodiscard]] std::string toString() const override {
                return "nslib::dynamic::Module<" + _path + ">";
            }

        protected:
            std::string _path;
            std::string _factorySymbol;
            import_t _handle = nullptr;

            /** Load the handle to the dynamic module. */
            void open() {
                if ( !file_exists(_path) ) {
                    throw ModuleException(_path, "The file does not exist or could not be opened");
                }

                _handle = dlopen(_path.c_str(), NSLIB_DL_OPTS);

                if ( _handle == nullptr ) {
                    std::string e = dlerror();
                    throw ModuleException(_path, "The dynamic library handle could not be loaded (" + e + ")");
                }
            }
        };
    }


    /* Some ref-count memory management helpers: */

#ifdef NSLIB_GC_TRACK
    using GCTracks = std::map<std::string, std::pair<std::vector<std::string>, std::vector<std::string>>>;
#endif

#ifdef NSLIB_GC_DEBUG_FREE
    class UseAfterFreeException : public NSLibException {
    public:
        explicit UseAfterFreeException(const std::string& message) : NSLibException(message) {}
    };
#endif

    /** Trait interface which adds a reference count. */
    class IRefCountable {
    public:
        IRefCountable() : _nslibRefCount(0), _nslibRefDisable(false) {}
        virtual ~IRefCountable() {
            for ( const auto& pair : _onFreeCallbacks ) {
                pair.second();
            }
        }

        /**
         * WARNING: DO NOT CALL DIRECTLY
         * Instead, call `useref(...)`.
         * Increment the reference count.
         */
        virtual void nslibIncRef() {
            _nslibRefCount += 1;

#ifdef NSLIB_GC_DEBUG_FREE
            if ( _nslibWouldHaveFreed ) {
                throw UseAfterFreeException("nslib: useref(...) called on object that was already freed by freeref(...) - free'd at: " + _nslibFreedAtTrace);
            }
#endif

#ifdef NSLIB_GC_TRACK
            if ( !_registeredShutdown ) {
                Framework::onShutdown([]() {
                    if ( _tracks.empty() ) {
                        std::cout << "NSLIB_GC_TRACK: All useref()/freeref() calls were balanced.\n";
                        return;
                    }

                    std::cout << "There were unbalanced useref()/freeref() calls:\n";
                    for ( const auto& ref : _tracks ) {
                        std::cout << "\n\n\n=====================================\nIRefCountable<id: " + ref.first + ">:\n";
                        std::cout << "------- useref() calls:\n";

                        for ( const auto& trace : ref.second.first ) {
                            std::cout << trace << "\n";
                        }

                        std::cout << "------- freeref() calls:\n";

                        for ( const auto& trace : ref.second.second ) {
                            std::cout << trace << "\n";
                        }
                    }
                });
                _registeredShutdown = true;
            }
            auto iter = _tracks.find(_nslibRefId);
            if ( iter == _tracks.end() ) {
                _tracks[_nslibRefId] = {{trace()}, {}};
            } else {
                iter->second.first.emplace_back(trace());  // FIXME: push back?
            }
#endif
        }

        /**
         * WARNING: DO NOT CALL DIRECTLY
         * Instead, call `freeref(...)`.
         * Decrement the reference count.
         */
        virtual void nslibDecRef() {
            _nslibRefCount -= 1;

#ifdef NSLIB_GC_TRACK
            auto iter = _tracks.find(_nslibRefId);
            if ( iter == _tracks.end() ) {
                _tracks[_nslibRefId] = {{}, {trace()}};
            } else {
                iter->second.second.emplace_back(trace());  // FIXME: push back?

                if ( nslibShouldFree() && iter->second.first.size() == iter->second.second.size() ) {
                    _tracks.erase(iter);
                }
            }
#endif
        }

#ifdef NSLIB_GC_TRACK
        [[nodiscard]] static GCTracks nslibGCTracks() { return _tracks; }
#endif

        virtual void nslibNoRef() { _nslibRefDisable = true; }

        /** If true, the instance can be deleted. */
        [[nodiscard]] virtual bool nslibShouldFree() const { return !_nslibRefDisable && _nslibRefCount < 1; }

#ifdef NSLIB_GC_DEBUG_FREE
        virtual void nslibMarkWouldHaveFreed() {
            _nslibWouldHaveFreed = true;
            _nslibFreedAtTrace = trace();
        }

        [[nodiscard]] virtual bool nslibWouldHaveFreed() const { return _nslibWouldHaveFreed; }
#endif

        virtual std::size_t nslibOnFree(std::function<void()> callback) {
            auto id = _nextOnFreeCallbackId++;
            _onFreeCallbacks[id] = std::move(callback);
            return id;
        }

        virtual void nslibUnregisterOnFree(std::size_t id) {
            auto iter = _onFreeCallbacks.find(id);
            if ( iter != _onFreeCallbacks.end() ) {
                _onFreeCallbacks.erase(iter);
            }
        }
    protected:
        std::size_t _nslibRefCount;
        bool _nslibRefDisable;
        std::map<std::size_t, std::function<void()>> _onFreeCallbacks;
        std::size_t _nextOnFreeCallbackId = 1;

#ifdef NSLIB_GC_DEBUG_FREE
        bool _nslibWouldHaveFreed = false;
        std::string _nslibFreedAtTrace;
#endif

#ifdef NSLIB_GC_TRACK
        std::string _nslibRefId = uuid();
        static GCTracks _tracks;
        static bool _registeredShutdown;
#endif
    };


    namespace priv {
        /** Concept uniting the IRefCountable interface w/ some other type T */
        template <class T>
        concept RefCountable = requires(T& obj) {
            { std::is_base_of_v<IRefCountable, T> };
        };
    }

    /** Open a new reference to some instance. */
    auto useref(priv::RefCountable auto r) {
        if ( r == nullptr ) return r;
        r->nslibIncRef();
        return r;
    }

    /** Release a reference to some instance. */
    void freeref(priv::RefCountable auto r) {
        if ( r == nullptr ) return;
        r->nslibDecRef();
        if ( r->nslibShouldFree() ) {
#ifdef NSLIB_GC_DEBUG_FREE
            r->nslibMarkWouldHaveFreed();
#else
#ifndef NSLIB_GC_NO_FREE
            delete r;
#endif
#endif
        }
    }

    /** Release an old reference and use a new one, if they are different. */
    auto swapref(priv::RefCountable auto oldRef, priv::RefCountable auto newRef) {
        if ( oldRef != newRef ) {
            useref(newRef);
            freeref(oldRef);
        }
        return newRef;
    }

    /**
     * A helper for automatically opening/releasing a reference in a scope.
     * @example
     * ```cpp
     * void myfunc(SomeClass* c) {
     *  auto cRef = localref(c);
     *  // ...
     *  // cRef will be released at the end of this call
     * }
     * ```
     */
    class RefHandle {
    public:
        explicit RefHandle(IRefCountable* ref) : _ref(useref(ref)) {}
        virtual ~RefHandle() { freeref(_ref); }
    protected:
        IRefCountable* _ref;
    };

    /** Creates a RefHandle on the stack. */
    RefHandle localref(IRefCountable* ref);

    template <typename T>
    class InlineRefHandle : public RefHandle {
    public:
        InlineRefHandle(T* inst, IRefCountable* ref) : RefHandle(ref), _inst(inst) {}

        T* get() const { return _inst; }

        T* operator->() { return get(); }
    protected:
        T* _inst;
    };

    template <typename T>
    InlineRefHandle<T> inlineref(priv::RefCountable auto inst) {
        return InlineRefHandle(inst, inst);
    }

    template <typename T>
    [[nodiscard]] inline std::string s(const InlineRefHandle<T>& v) {
        return s(v.get());
    }


    /** Holds an IRefCountable, but does not prevent it from being garbage collected. */
    template <typename T>
    class WeakRef : public IStringable {
    public:
        WeakRef(T* inst, IRefCountable* ref) : _inst(inst), _ref(ref) {
            _callbackId = ref->nslibOnFree([this]() {
                _inst = nullptr;
                _ref = nullptr;
            });
        }

        ~WeakRef() override {
            if ( _ref != nullptr ) {
                _ref->nslibUnregisterOnFree(_callbackId);
            }
        }

        [[nodiscard]] T* get() const { return _inst; }
        T* operator->() { return get(); }

        [[nodiscard]] std::string toString() const override {
            return "WeakRef<" + s(_ref) + ">";
        }
    protected:
        T* _inst;
        IRefCountable* _ref;
        std::size_t _callbackId = 0;
    };

    template <typename T>
    WeakRef<T> weakref(priv::RefCountable auto inst) {
        return WeakRef(inst, inst);
    }


    /**
     * A helper for deferring logic until function return a la GoLang.
     * Prefer the NS_DEFER() macro instead of using this class directly:
     *
     * @example
     * ```cpp
     * void myFunc() {
     *     NS_DEFER()(()[] {
     *         std::cout << "Some cleanup work.\n";
     *     });
     *
     *     std::cout << "Hello!\n";
     * }
     * ```
     *
     * This outputs:
     *
     * ```txt
     * Hello!
     * Some cleanup work.
     * ```
     */
    class Defer : public IStringable {
    public:
        Defer(const std::function<void()>& callback) : _callback(callback) {}

        ~Defer() override { _callback(); }

        std::string toString() const override {
            return "nslib::Defer<>";
        }

    protected:
        std::function<void()> _callback;
    };



    /* A simple event bus system inspired by https://github.com/dquist/EventBus */
    namespace event {
        using unsubscribe_t = std::function<void()>;

        /** Base class for events. */
        class Event : public IStringable {};

        /** Listens for events of type T. */
        template <class T>
        class Listener : public IStringable, public IRefCountable {
        public:
            Listener() : IRefCountable() {
                static_assert(std::is_base_of<Event, T>::value, "nslib::event::EventHandler<T>: T must extend Event");
            }

            virtual void handle(T&) = 0;

            void dispatch(Event& e) {
                handle(dynamic_cast<T&>(e));
            }
        };

        namespace priv {
            using listener_map_t = std::unordered_map<std::string, void* const>;

            template <class T>
            class LambdaListener : public Listener<T> {
            public:
                explicit LambdaListener(std::function<void(T&)> handler) : Listener<T>(), _handler(handler) {}

                void handle(T& e) override {
                    _handler(e);
                }

                [[nodiscard]] std::string toString() const override {
                    return "nslib::event::priv::LambdaListener<T: " + s(typeid(T)) + ">";
                }

            protected:
                std::function<void(T&)> _handler;
            };
        }

        /** A simple event bus. */
        class EventBus : public IStringable, public IRefCountable {
        public:
            static EventBus* get() {
                if ( _global == nullptr ) {
                    _global = useref(getNew("global"));
                    Framework::onShutdown([]() {
                        freeref(_global);
                    });
                }
                return _global;
            }

            static EventBus* getNew(const std::string& name) {
                return new EventBus(name);
            }

            explicit EventBus(std::string  name) : _name(std::move(name)) {}

            /**
             * Register a listener for events of type T.
             * Returns a function which can be called to remove the listener.
             */
            template <class T>
            unsubscribe_t listen(Listener<T>* listener) {
                auto name = s(typeid(T));
                priv::listener_map_t listeners = {};
                auto found = _map.find(name);
                if ( found != _map.end() ) {
                    listeners = found->second;
                }

                auto listenerId = uuid();
                listeners.insert({listenerId, useref(listener)});

                _map[name] = listeners;

                // The unsubscriber:
                return [this, listenerId, listener]() {
                    auto found = _map.find(s(typeid(T)));
                    if ( found == _map.end() ) {
                        return;
                    }

                    auto mappedListener = found->second.find(listenerId);
                    if ( mappedListener == found->second.end() ) {
                        return;
                    }

                    found->second.erase(mappedListener);
                    freeref(listener);
                };
            }

            template <class T>
            unsubscribe_t listen(std::function<void(T&)> listener) {
                return listen(new priv::LambdaListener<T>(listener));
            }

            /** Raise an event. */
            void dispatch(Event& e) {
                auto listeners = _map.find(s(typeid(e)));
                if ( listeners == _map.end() ) {
                    // No listeners registered for this event type
                    return;
                }

                for ( const auto& listener : listeners->second ) {
                    static_cast<Listener<Event>*>(listener.second)->dispatch(e);
                }
            }

            [[nodiscard]] std::string toString() const override {
                return "EventBus<name: " + _name + ">";
            }

        protected:
            std::string _name;
            std::unordered_map<std::string, priv::listener_map_t> _map;

            static EventBus* _global;
        };
    }
}

#endif //NSLIB_H
