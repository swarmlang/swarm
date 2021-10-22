#ifndef SWARMC_CONSOLE_H
#define SWARMC_CONSOLE_H

#include <string>
#include <sstream>
#include <iostream>
#include <climits>
#include <limits>
#include <memory>
#include "Validator.h"
#include "../IStringable.h"

/**
 * Some helpful methods for interacting with the console and strings.
 */
class Console {
private:
    // Viewport utility methods. Used to track viewport size, and grid printing information.
    int _vp_width = 70;
    int _vp_height = -1; // -1 disables height-specific adjustments
    bool _vp_inGrid = false;
    int _vp_grid_rows = 0;
    bool _vp_grid_inf_rows = false;
    int _vp_grid_cols = 0;
    int _vp_curr_col = 0;
    int _vp_curr_row = 0;
    int _vp_col_width = 0;
    bool _vp_col_bit = false;

    int _new_lines_since_last_clear = 0;
    int _chars_since_last_newline = 0;

    bool _vp_grid_row_skip_newline = false;

    // Verbosity level for debugging output.
    bool _debug = false;
    bool _debug_capture = false;

    std::string ANSI_RESET_ALL = "\033[0m";
    std::string ANSI_CLEAR_ALL = "\x1b[2J\x1b[1;1H";

    // ANSI Formatting Characters
    std::string ANSI_BOLD = "\033[1m";
    std::string ANSI_RESET_BOLD = "\033[22m";
    std::string ANSI_UNDERLINE = "\033[4m";
    std::string ANSI_RESET_UNDERLINE = "\033[24m";
    std::string ANSI_INVERSE = "\033[7m";
    std::string ANSI_RESET_INVERSE = "\033[27m";

    // ANSI Foreground Colors
    std::string ANSI_FOREGROUND_BLACK = "\033[30m";
    std::string ANSI_FOREGROUND_RED = "\033[31m";
    std::string ANSI_FOREGROUND_GREEN = "\033[32m";
    std::string ANSI_FOREGROUND_YELLOW = "\033[33m";
    std::string ANSI_FOREGROUND_BLUE = "\033[34m";
    std::string ANSI_FOREGROUND_MAGENTA = "\033[35m";
    std::string ANSI_FOREGROUND_CYAN = "\033[36m";
    std::string ANSI_FOREGROUND_WHITE = "\033[37m";
    std::string ANSI_RESET_FOREGROUND_COLOR = "\033[39m";

    // ANSI Background Colors
    std::string ANSI_BACKGROUND_BLACK = "\033[40m";
    std::string ANSI_BACKGROUND_RED = "\033[41m";
    std::string ANSI_BACKGROUND_GREEN = "\033[42m";
    std::string ANSI_BACKGROUND_YELLOW = "\033[43m";
    std::string ANSI_BACKGROUND_BLUE = "\033[44m";
    std::string ANSI_BACKGROUND_MAGENTA = "\033[45m";
    std::string ANSI_BACKGROUND_CYAN = "\033[46m";
    std::string ANSI_BACKGROUND_WHITE = "\033[47m";
    std::string ANSI_RESET_BACKGROUND_COLOR = "\033[49m";

    // Unicode Bars
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

    std::string UNI_BLOCK_FULL = "█";

    // Mute output.
    bool mute() const;

    Console() {};

    static Console* _global_console;

public:
    Console(Console& other) = delete;  // don't allow cloning

    void operator=(const Console&) = delete;  // don't allow assigning

    static Console* get();

    // Modifiers
    Console* background(std::string color);
    Console* color(std::string color);
    Console* bold();
    Console* underline();
    Console* invert();

    // Targets
    Console* reset();
    Console* print(std::string text, bool reset = false);
    Console* line(std::string text = "", bool reset = true);
    Console* clear();

    // Customs
    Console* info(std::string text);
    Console* error(std::string text);
    Console* cerr(std::string text);
    Console* warn(std::string text);
    Console* success(std::string text);
    Console* debug(std::string text);
    Console* debug();
    Console* end();

    bool isDebug();


    // ===== Viewport functions =====

    // Print a block header with the provided text
    Console* header(std::string text);

    // Put a box around some text
    Console* box(std::string text);

    // Enable grid mode with the specified rows and columns
    Console* grid(int rows, int cols);

    // Enable grid mode with the specified columns, but an undetermined number of rows
    Console* grid(int cols);

    // Expand the viewport width to fit the width of the user's terminal
    Console* grow(bool grow_width = true);

    // Fetch and store the current cursor position
    Console* position(int& x, int& y);

    // Move the cursor to the specified position
    Console* jumpTo(int x, int y);

    // Print a visual separator to the screen
    Console* sep();

    // Start a new row when in grid-mode
    // Note, should also be called AFTER the last row is printed to close off the grid (or endGrid())
    Console* row(bool skip_newline = false);

    // Explicitly end the grid.
    // Used if the grid is in infinite row mode,
    // or if you want to end the grid before all rows have printed.
    Console* endGrid();

    // Print a single cell's value in grid mode
    Console* col(std::string text);

    Console* progress(double value);

    // Helpers
    // Set verbosity to true
    Console* verbose();

    // Set verbosity to false
    Console* quiet();

    // Print a raw string to the console
    Console* output(std::string raw);

    // Primitive conversion helpers
    std::string toString(char text) const;
    std::string toString(int text) const;
    std::string toString(unsigned int text) const;
    std::string toString(double text) const;
    std::string toString(bool text) const;
    std::string toString(long int text) const;
    std::string toString(IStringable &stringable) const;
    std::string toString(IStringable* stringable) const;

    // Pad the back of a string with spaces until it is at least the min length
    std::string pad(std::string text, unsigned int pad = 20, std::string with = " ") const;

    static std::string sPad(std::string text, unsigned int pad = 20, std::string with = " ") {
        std::stringstream s;
        s << text;

        int pad_diff = pad - text.length();
        for ( int i = 0; i < pad_diff; i++ ) {
            s << with;
        }

        return s.str();
    }

    // Pad the front of a string with spaces until it is at least the min length
    std::string padFront(std::string text, unsigned int pad = 20, std::string with = " ") const;

    // Pad the front and back of a string with spaces until it is at least the min length
    // Keeps it roughly centered
    std::string padCenter(std::string text, unsigned int pad = 20, std::string with = " ") const;

    static std::string sPadCenter(std::string text, unsigned int pad = 20, std::string with = " ") {
        std::stringstream s;

        if ( text.length() >= pad ) {
            return text;
        }

        bool bit = false;
        int lhs = pad - text.length();
        if ( lhs % 2 == 1 ) { lhs -= 1; bit = true; }
        lhs /= 2;

        s << Console::sPad("", lhs, with);
        s << text;
        s << Console::sPad("", lhs, with);
        if ( bit ) {
            s << with;
        }

        return s.str();
    }

    // Prompt the user for a value, get their response or the default.
    template <typename InputType>
    InputType prompt(std::string message, InputType default_value);

    // Prompt the user for a value and get their response
    template <typename InputType>
    InputType prompt(std::string message);

    // Prompt the user for a value and get their response.
    // Repeat this until the user has provided input that passes the provided validator
    // Display the validators requirements on error
    template <typename InputType>
    InputType prompt(std::string message, InputType default_value, std::string name, Validator<InputType> &valid);

    // A special implementation of "prompt" that allows the user input to contain whitespace
    std::string promptAllowingWhitespace(std::string message);

    std::string trim(std::string input);

    // True if grid mode is enabled
    bool inGrid() {
        return _vp_inGrid;
    }
};

template <typename InputType>
InputType Console::prompt(std::string message, InputType default_value) {
    std::stringstream val_string;
    val_string << default_value;

    this->line("")->color("green")->print("   ")->print(message)->reset()->print(" [")->color("yellow")->print(val_string.str())->reset()->line("]");
    this->print("   > ");

    std::cin >> std::noskipws;

    std::string check;
    check = std::cin.peek();

    this->line();
    if ( check == "\n" ) {
        std::string dump;
        std::getline(std::cin, dump);
        return default_value;
    }

    InputType input;
    std::cin >> input;

    if ( std::cin.fail() ) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        this->error("Cannot cast value. Invalid type. Please try again.");
        return this->template prompt<InputType>(message, default_value);
    }

    std::string dump;
    std::getline(std::cin, dump);

    return input;
}

template <typename InputType>
InputType Console::prompt(std::string message) {
    this->line("")->color("green")->print("   ")->print(message)->reset()->line("");
    this->print("   > ");

    std::cin >> std::noskipws;

    std::string check;
    check = std::cin.peek();
    this->line();
    if ( check == "\n" ) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        this->error("Please provide a value.");
        return this->template prompt<InputType>(message);
    }

    InputType input;
    std::cin >> input;

    if ( std::cin.fail() ) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        this->error("Cannot cast value. Invalid type. Please try again.");
        return this->template prompt<InputType>(message);
    }

    std::string dump;
    std::getline(std::cin, dump);

    return input;
}

template <typename InputType>
InputType Console::prompt(std::string message, InputType default_value, std::string name, Validator<InputType> &valid) {
    InputType value = this->template prompt<InputType>(message, default_value);

    while ( !valid.check(value) ) {
        this->error("Invalid "+name+". "+valid.requirements(name));
        value = this->template prompt<InputType>(message, default_value);
    }

    return value;
}

/**
 * Helper interface that adds a protected `console` member.
 * Just allows easy access to the global console instance.
 */
class IUsesConsole {
protected:
    Console* console = nullptr;

public:
    IUsesConsole() {
        console = Console::get();
    }

    virtual ~IUsesConsole() {}
};

#endif //SWARMC_CONSOLE_H
