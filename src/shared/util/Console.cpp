#include "Console.h"
#include "../IStringable.h"

#include <iostream>
#include <sstream>
#include <algorithm>

#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>

Console* Console::_global_console = nullptr;

Console* Console::get()  {
    /** TODO - this is not thread-safe. Protect with a mutex in multi-threaded env. */
    if ( _global_console == nullptr ) {
        _global_console = new Console();
    }

    return (Console*) _global_console;
}

Console* Console::background(std::string color) {
    if ( color == "black" ) {
        output(ANSI_BACKGROUND_BLACK);
    } else if ( color == "red" ) {
        output(ANSI_BACKGROUND_RED);
    } else if ( color == "green" ) {
        output(ANSI_BACKGROUND_GREEN);
    } else if ( color == "yellow" ) {
        output(ANSI_BACKGROUND_YELLOW);
    } else if ( color == "blue" ) {
        output(ANSI_BACKGROUND_BLUE);
    } else if ( color == "magenta" ) {
        output(ANSI_BACKGROUND_MAGENTA);
    } else if ( color == "cyan" ) {
        output(ANSI_BACKGROUND_CYAN);
    } else if ( color == "white" ) {
        output(ANSI_BACKGROUND_WHITE);
    } else if ( color == "reset" ) {
        output(ANSI_RESET_BACKGROUND_COLOR);
    }

    return this;
}

Console* Console::color(std::string color) {
    if ( color == "black" ) {
        output(ANSI_FOREGROUND_BLACK);
    } else if ( color == "red" ) {
        output(ANSI_FOREGROUND_RED);
    } else if ( color == "green" ) {
        output(ANSI_FOREGROUND_GREEN);
    } else if ( color == "yellow" ) {
        output(ANSI_FOREGROUND_YELLOW);
    } else if ( color == "blue" ) {
        output(ANSI_FOREGROUND_BLUE);
    } else if ( color == "magenta" ) {
        output(ANSI_FOREGROUND_MAGENTA);
    } else if ( color == "cyan" ) {
        output(ANSI_FOREGROUND_CYAN);
    } else if ( color == "white" ) {
        output(ANSI_FOREGROUND_WHITE);
    } else if ( color == "reset" ) {
        output(ANSI_RESET_FOREGROUND_COLOR);
    }

    return this;
}

Console* Console::bold() {
    output(ANSI_BOLD);
    return this;
}

Console* Console::underline() {
    output(ANSI_UNDERLINE);
    return this;
}

Console* Console::invert() {
    output(ANSI_INVERSE);
    return this;
}

Console* Console::reset() {
    output(ANSI_RESET_ALL);
    return this;
}

Console* Console::print(std::string text, bool reset) {
    output(text);
    if ( reset ) { this->reset(); }
    return this;
}

Console* Console::line(std::string text, bool reset) {
    output(text)->output("\n");
    if ( reset ) { this->reset(); }
    return this;
}

Console* Console::info(std::string text) {
    color("blue")->print("[INFO] ")->reset()->line(text);
    return this;
}

Console* Console::error(std::string text) {
    color("red")->print("[ERROR] ")->reset()->line(text);
    return this;
}

Console* Console::cerr(std::string text) {
    std::cerr << text << "\n";
    return this;
}

Console* Console::warn(std::string text) {
    color("yellow")->print("[WARNING] ")->reset()->line(text);
    return this;
}

Console* Console::success(std::string text) {
    color("green")->print("[SUCCESS] ")->reset()->line(text);
    return this;
}

Console* Console::debug(std::string text) {
    if ( _debug ) {
        color("magenta")->print("[DEBUG] ")->reset()->line(text);
    }

    return this;
}

Console* Console::debug() {
    _debug_capture = true;
    return this;
}

bool Console::isDebug() {
    return _debug;
}

Console* Console::end() {
    _debug_capture = false;
    return this;
}

Console* Console::verbose() {
    _debug = true;
    return this;
}

Console* Console::quiet() {
    _debug = false;
    return this;
}

Console* Console::output(std::string raw) {

    if ( !mute() ) {
//        int n_lines = std::count(raw.begin(), raw.end(), '\n');
//        _new_lines_since_last_clear += n_lines;

//        char rets[1] = {'\n'};
//        int index_line_reset = raw.find_last_of(rets);

//        if ( index_line_reset >= 0 ) {
//            std::string additive = raw.substr(index_line_reset, raw.length());
//            _chars_since_last_newline = additive.length();
//        }
//        else {
//            _chars_since_last_newline += raw.length();
//        }

        std::cout << raw;
    }

    return this;
}

std::string Console::pad(std::string text, unsigned int pad, std::string with) const {
    std::stringstream s;
    s << text;

    int pad_diff = pad - text.length();
    for ( int i = 0; i < pad_diff; i++ ) {
        s << with;
    }

    return s.str();
}

std::string Console::padFront(std::string text, unsigned int pad, std::string with) const {
    std::stringstream s;

    int pad_diff = pad - text.length();
    for ( int i = 0; i < pad_diff; i++ ) {
        s << with;
    }

    s << text;
    return s.str();
}

std::string Console::padCenter(std::string text, unsigned int pad, std::string with) const {
    std::stringstream s;

    if ( text.length() >= pad ) {
        return text;
    }

    bool bit = false;
    int lhs = pad - text.length();
    if ( lhs % 2 == 1 ) { lhs -= 1; bit = true; }
    lhs /= 2;

    s << this->pad("", lhs, with);
    s << text;
    s << this->pad("", lhs, with);
    if ( bit ) {
        s << with;
    }

    return s.str();
}

Console* Console::header(std::string text) {
    text = padCenter(text, _vp_width-2);
    box(text)->print("\n")->reset();
    return this;
}

Console* Console::box(std::string text) {
    print(UNI_BAR_TOP_LEFT);
    std::string hbar = pad("", text.length(), UNI_BAR_HORIZ);

    print(hbar);
    print(UNI_BAR_TOP_RIGHT+"\n");
    print(UNI_BAR_VERT)->print(text)->print(UNI_BAR_VERT+"\n");

    print(UNI_BAR_BOTTOM_LEFT)->print(hbar)->print(UNI_BAR_BOTTOM_RIGHT);
    return this;
}

Console* Console::grid(int rows, int cols) {
    _vp_inGrid = true;
    _vp_grid_cols = cols;
    _vp_grid_rows = rows;
    return this;
}

Console* Console::grid(int cols) {
    _vp_inGrid = true;
    _vp_grid_cols = cols;
    _vp_grid_rows = 0;
    _vp_grid_inf_rows = true;
    return this;
}

Console* Console::row(bool skip_newline) {
    _vp_grid_row_skip_newline = skip_newline;

    // if 50 wide, 4 columns
    // 48 after sides x | x | x | x
    // - n cols
    int total_col = (_vp_width - 2) - (_vp_grid_cols-1);
    int col_width = (int) (1.0*total_col)/(_vp_grid_cols);
    double abs_width = (1.0*total_col)/_vp_grid_cols;

    bool bit = false;

    if ( abs_width > col_width ) {
        bit = true;
    }

    _vp_col_bit = bit;
    _vp_col_width = col_width;

    std::string colbar = pad("", col_width, UNI_BAR_HORIZ);

    if ( _vp_curr_row == 0 ) {
        print(UNI_BAR_TOP_LEFT);
    } else if ( _vp_grid_inf_rows || _vp_curr_row < _vp_grid_rows ) {
        print(UNI_BAR_LEFT_ROW_JOIN);
    } else {
        print(UNI_BAR_BOTTOM_LEFT);
    }

    print(colbar);
    for ( int i = 0; i < _vp_grid_cols-1; i++ ) {
        if ( _vp_curr_row == 0 ) {
            print(UNI_BAR_TOP_COL_JOIN + colbar);
        } else if ( _vp_grid_inf_rows || _vp_curr_row < _vp_grid_rows ) {
            print(UNI_BAR_CENTER_JOIN + colbar);
        } else {
            print(UNI_BAR_BOTTOM_COL_JOIN + colbar);
        }
    }

    if ( bit ) {
        print(UNI_BAR_HORIZ);
    }

    if ( _vp_curr_row == 0 ) {
        print(UNI_BAR_TOP_RIGHT);
    } else if ( _vp_grid_inf_rows || _vp_curr_row < _vp_grid_rows ) {
        print(UNI_BAR_RIGHT_ROW_JOIN);
    } else {
        print(UNI_BAR_BOTTOM_RIGHT);
        line("");
        _vp_grid_rows = 0;
        _vp_grid_cols = 0;
        _vp_curr_col = 0;
        _vp_curr_row = 0;
        _vp_inGrid = false;
        _vp_grid_inf_rows = false;
    }

    print("\n");
    return this;
}

Console* Console::endGrid() {
    if ( !_vp_inGrid ) return this;
    _vp_grid_inf_rows = false;
    _vp_curr_row = _vp_grid_rows + 1;
    return row();
}

Console* Console::col(std::string text) {
    text = padCenter(text, _vp_col_width);
    print(UNI_BAR_VERT)->print(text);
    _vp_curr_col += 1;

    if ( _vp_curr_col >= _vp_grid_cols ) {
        if ( _vp_col_bit ) { print(" "); }
        print(UNI_BAR_VERT);
        if ( !_vp_grid_row_skip_newline ) print("\n");
        _vp_grid_row_skip_newline = false;
        _vp_curr_col = 0;
        _vp_curr_row += 1;
    }

    return this;
}

Console* Console::grow(bool grow_width) {
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

    if ( grow_width ) _vp_width = w.ws_col;
    _vp_height = w.ws_row;

    return this;
}

// TODO
Console* Console::position(int &x, int &y) {
    y = _new_lines_since_last_clear;
    x = _chars_since_last_newline;

    return this;
}

// TODO account for this in the virtual position
Console* Console::jumpTo(int x, int y) {
    printf("\033[%d;%dH", x, y);
    return this;
}

Console* Console::sep() {
    std::string bar = pad("", _vp_width/2, UNI_BAR_HORIZ+" ");
    if ( _vp_col_bit ) { bar = " "+bar; }
    line("")->line(bar)->line("");
    return this;
}

std::string Console::toString(int text) const {
    return std::to_string(text);
}

std::string Console::toString(unsigned int text) const {
    return std::to_string(text);
}

std::string Console::toString(long int text) const {
    return std::to_string(text);
}

std::string Console::toString(double text) const {
    return std::to_string(text);
}

std::string Console::toString(char text) const {
    std::stringstream s;
    s << text;
    return s.str();
}

std::string Console::toString(bool text) const {
    if ( text ) { return "true"; }
    else { return "false"; }
}

std::string Console::toString(IStringable &stringable) const {
    return stringable.toString();
}

std::string Console::toString(IStringable* stringable) const {
    return stringable->toString();
}

bool Console::mute() const {
    return _debug_capture && !_debug;
}

Console* Console::clear() {
    reset()->output(ANSI_CLEAR_ALL);
    if ( !mute() ) {
        _new_lines_since_last_clear = 0;
        std::cout << std::flush;
    }
    return this;
}

Console* Console::progress(double value) {
    int width = _vp_width-5;
    int percent = 100*value;
    if ( value >= 1 ) percent = 100;

    this->output("\r");

    int filled = width*value;
    if ( value >= 1 ) filled = width;
    std::string bar = this->pad("", filled, UNI_BLOCK_FULL);
    this->color("green")->output(bar)->reset();

    std::string space = this->pad("", (width - (filled)), UNI_BLOCK_FULL);
    this->output(space);
    this->color((value >= 1 ? "white" : "green"))->output(this->padFront(this->toString(percent)+"%", 5))->reset();
    std::cout.flush();

    return this;
}

std::string Console::trim(std::string input) {
    std::string::size_type first_real_index = input.find_first_not_of("\n\t ");

    if ( std::string::npos == first_real_index ) {
        return "";
    }

    std::string::size_type last_real_index = input.find_last_not_of("\n\t ");
    return input.substr(first_real_index, (last_real_index - first_real_index + 1));
}

std::string Console::promptAllowingWhitespace(std::string message) {
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
        return this->template prompt<std::string>(message);
    }

    std::string input;
    std::getline(std::cin, input);

    if ( std::cin.fail() ) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        this->error("Cannot cast value. Invalid type. Please try again.");
        return this->template prompt<std::string>(message);
    }

    return input;
}

