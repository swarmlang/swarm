# The executable to write
TARGET_EXEC ?= swarmc

# Some helper constants
BUILD_DIR ?= ./build
DEBUG_BUILD_DIR ?= ./build_debug
SRC_DIRS ?= ./src
LEXER := flex
BISON := bison

# Files automatically generated by Bison. Removed by clean.
BISON_FILES := src/bison/grammar.hh src/bison/parser.cc src/bison/parser.output src/bison/stack.hh

SRCS := $(shell find $(SRC_DIRS) -name *.cpp -or -name *.c -or -name *.s)
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)
DEBUG_OBJS := $(SRCS:%=$(DEBUG_BUILD_DIR)/%.o)
DEBUG_DEPS := $(DEBUG_OBJS:.o=.d)

INC_DIRS := $(shell find $(SRC_DIRS) -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

CPPFLAGS ?= $(INC_FLAGS) -MMD -MP -g -std=c++17 -Wall
CPPFLAGS_debug ?= $(INC_FLAGS) -MMD -MP -g -std=c++17 -Wall -DSWARM_DEBUG

$(TARGET_EXEC): $(OBJS) $(BUILD_DIR)/lexer.o
	g++ $(OBJS) $(BUILD_DIR)/lexer.o -o $@ $(LDFLAGS)

.PHONY: all
all: $(TARGET_EXEC) debug

# c++ sources
$(BUILD_DIR)/%.cpp.o: %.cpp
	$(MKDIR_P) $(dir $@)
	g++ $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

# Build the flex-generated lexer source. Relies on parser code, so build that first.
$(BUILD_DIR)/lexer.yy.cc: src/swarm.l $(BUILD_DIR)/parser.o
	$(LEXER) --outfile $(BUILD_DIR)/lexer.yy.cc $<

$(BUILD_DIR)/lexer.o: $(BUILD_DIR)/lexer.yy.cc
	g++ $(CPPFLAGS) -c $(BUILD_DIR)/lexer.yy.cc -o $(BUILD_DIR)/lexer.o

$(BUILD_DIR)/parser.o: src/bison/parser.cc
	g++ $(CPPFLAGS) -c -o $@ $<



# Build a version with debugging enabled. Should be (mostly) non-clashing with the release version
debug: $(DEBUG_OBJS) $(DEBUG_BUILD_DIR)/parser.o $(DEBUG_BUILD_DIR)/lexer.o
	g++ $(DEBUG_OBJS) $(DEBUG_BUILD_DIR)/lexer.o $(DEBUG_BUILD_DIR)/parser.o -o $(TARGET_EXEC)_debug $(LDFLAGS) $(CPPFLAGS_debug)

# c++ sources for debug
$(DEBUG_BUILD_DIR)/%.cpp.o: %.cpp
	$(MKDIR_P) $(dir $@)
	g++ $(CPPFLAGS_debug) $(CPPFLAGS_debug) -c $< -o $@

# Build the flex-generated lexer source. Relies on parser code, so build that first.
$(DEBUG_BUILD_DIR)/lexer.yy.cc: src/swarm.l $(DEBUG_BUILD_DIR)/parser.o
	$(LEXER) --outfile $(DEBUG_BUILD_DIR)/lexer.yy.cc $<

$(DEBUG_BUILD_DIR)/lexer.o: $(DEBUG_BUILD_DIR)/lexer.yy.cc
	g++ $(CPPFLAGS_debug) -c $(DEBUG_BUILD_DIR)/lexer.yy.cc -o $(DEBUG_BUILD_DIR)/lexer.o

# This is PHONY to force the Bison generated source files to re-generate
# between debug and release builds.
.PHONY: src/bison.parser.cc
src/bison/parser.cc: src/bison/swarm.yy
	cd src/bison && $(BISON) -Wall --defines=grammar.hh -v swarm.yy

$(DEBUG_BUILD_DIR)/parser.o: src/bison/parser.cc
	g++ $(CPPFLAGS_debug) -c -o $@ $<

.PHONY: clean
clean:
	$(RM) -r $(BUILD_DIR) $(TARGET_EXEC) $(DEBUG_BUILD_DIR) $(TARGET_EXEC)_debug $(BISON_FILES)

-include $(DEPS)

MKDIR_P ?= mkdir -p
