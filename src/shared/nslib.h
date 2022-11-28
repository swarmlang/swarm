#ifndef NSLIB_H
#define NSLIB_H

#include <random>
#include <stack>
#include <string>
#include <sstream>
#include <iostream>
#include <chrono>
#include <functional>
#include <unistd.h>
#include <sys/ioctl.h>

namespace nslib {

    namespace priv {
        static std::random_device rd;
        static std::default_random_engine eng(rd());
        static std::mt19937_64 generator;
        static std::uniform_int_distribution<> distribution(0, 15);
        static std::uniform_int_distribution<> distribution2(8, 11);
        static std::uniform_real_distribution<double> realDistribution(1, 0);

        static size_t DETERMINISTIC_UUID_LAST = 0;
        static bool USE_DETERMINISTIC_UUIDS = false;
    }

    class Framework {
    public:
        Framework(Framework& other) = delete;  // don't allow cloning
        void operator=(const Framework&) = delete;  // don't allow assigning

        static void boot() {
            if ( _booted ) return;

            auto epoch = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count();
            priv::generator.seed(epoch);

            _booted = true;
        }

        static void shutdown() {
            if ( !_booted ) return;
        }

    protected:
        Framework() = default;
        static bool _booted;
    };


    /** An instance which has a string representation. */
    class IStringable {
    public:
        virtual ~IStringable() = default;
        [[nodiscard]] virtual std::string toString() const = 0;
    };


    /** ANSI-supported colors. */
    enum class ANSIColor: size_t {
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
    enum class Verbosity: size_t {
        ERROR = 5,
        WARNING = 4,
        INFO = 3,
        DEBUG = 2,
        VERBOSE = 1,
        TRACE = 0,
    };

    inline std::string s(Verbosity v) {
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
    inline std::string s(IStringable& v) { return v.toString(); }
    inline std::string s(IStringable* v) { return v->toString(); }


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
    }


    /**
     * Utility class for interacting with the Console window.
     * @todo prompts
     */
    class Console : public ILogTarget {
    public:
        Console(Console& other) = delete;  // don't allow cloning
        void operator=(const Console&) = delete;  // don't allow assigning

        /** Get the singleton Console instance. */
        static Console* get() {
            if ( _global == nullptr ) {
                _global = new Console();
            }

            return _global;
        }

        /** Output a string at the given verbosity. Satisfies ILogTarget. */
        void output(Verbosity verb, std::string s) override {
            if ( shouldOutput(verb) ) {
                auto& stream = getStreamFor(verb);
                stream << s;
            }
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
            _cleanup.push([](Console* c) {
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
            _cleanup.push([](Console* c) {
                c->output(c->ANSI_RESET_FOREGROUND_COLOR);
            });
            return this;
        }

        /** Start bolding text. */
        Console* bold() {
            output(ANSI_BOLD);
            _cleanup.push([](Console* c) {
                c->output(c->ANSI_RESET_BOLD);
            });
            return this;
        }

        /** Start underlining text. */
        Console* underline() {
            output(ANSI_UNDERLINE);
            _cleanup.push([](Console* c) {
                c->output(c->ANSI_RESET_UNDERLINE);
            });
            return this;
        }

        /** Invert background/foreground colors. */
        Console* invert() {
            output(ANSI_INVERSE);
            _cleanup.push([](Console* c) {
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
            while ( !_cleanup.empty() ) end();
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
            return only(Verbosity::ERROR)
                ->color(ANSIColor::RED)
                ->print("   error ", true)
                ->output(p + "\n")
                ->end();
        }

        /** Output an error message. */
        Console* success(const std::string& p) {
            return only(Verbosity::INFO)
                    ->color(ANSIColor::GREEN)
                    ->print(" success ", true)
                    ->output(p + "\n")
                    ->end();
        }

        /** Output a warning message. */
        Console* warn(const std::string& p) {
            return only(Verbosity::WARNING)
                    ->color(ANSIColor::YELLOW)
                    ->print(" warning ", true)
                    ->output(p + "\n")
                    ->end();
        }

        /** Output an informational message. */
        Console* info(const std::string& p) {
            return only(Verbosity::INFO)
                    ->color(ANSIColor::BLUE)
                    ->print("    info ", true)
                    ->output(p + "\n")
                    ->end();
        }

        /** Output a debugging message. */
        Console* debug(const std::string& p) {
            return only(Verbosity::DEBUG)
                    ->color(ANSIColor::MAGENTA)->print("   debug ")->end()
                    ->output(p + "\n")->end();
        }

        /** Output a verbose message. */
        Console* verbose(const std::string& p) {
            return only(Verbosity::VERBOSE)
                    ->color(ANSIColor::CYAN)
                    ->print(" verbose ", true)
                    ->output(p + "\n")
                    ->end();
        }

        /** Output a trace message. */
        Console* trace(const std::string& p) {
            return only(Verbosity::TRACE)
                    ->color(ANSIColor::GREY)
                    ->print("   trace ", true)
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
            size_t width = _vpWidth - 5;
            size_t filled = static_cast<int>(static_cast<double>(width) * value);
            size_t percent = static_cast<int>(100 * value);
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
            _cleanup.push([](Console* c) {
                c->_muteStack -= 1;
            });
            return this;
        }

        /** Temporarily limit output to the given verbosity. */
        Console* only(Verbosity v) {
            _verbLimits.push(v);
            _cleanup.push([](Console* c) {
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
        /** The global console instance. */
        static Console* _global;

        /** New consoles cannot be made outside this class. */
        Console() = default;

        /** The current verbosity level. */
        Verbosity _verb = Verbosity::INFO;

        /** The current width of the viewport. */
        size_t _vpWidth = 70;

        /** True if we are capturing output into an internal buffer. */
        bool _isCapturing = false;

        /** Internal buffer of captured output. */
        std::stringstream _capture;

        /** List of reset codes that clear applied modifiers, in order. */
        std::stack<std::function<void(Console*)>> _cleanup;

        std::stack<Verbosity> _verbLimits;

        /** Number of layers the Console is muted. */
        size_t _muteStack = 0;

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
    };

    class IUsesConsole {
    public:
        virtual ~IUsesConsole() = default;

        IUsesConsole() {
            console = Console::get();
        }
    protected:
        Console* console;
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
}

#endif //NSLIB_H