TARGET_EXEC ?= swarmc

BUILD_DIR ?= ./build
DEBUG_BUILD_DIR ?= ./build_debug
SRC_DIRS ?= ./src

SRCS := $(shell find $(SRC_DIRS) -name *.cpp -or -name *.c -or -name *.s)
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)
DEBUG_OBJS := $(SRCS:%=$(DEBUG_BUILD_DIR)/%.o)
DEBUG_DEPS := $(DEBUG_OBJS:.o=.d)

INC_DIRS := $(shell find $(SRC_DIRS) -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

CPPFLAGS ?= $(INC_FLAGS) -MMD -MP -g -std=c++17 -Wall
CPPFLAGS_debug ?= $(INC_FLAGS) -MMD -MP -g -std=c++17 -Wall -DSWARM_DEBUG

$(TARGET_EXEC): $(OBJS)
	g++ $(OBJS) -o $@ $(LDFLAGS)

# c++ source
$(BUILD_DIR)/%.cpp.o: %.cpp
	$(MKDIR_P) $(dir $@)
	g++ $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@


debug: $(DEBUG_OBJS)
	g++ $(DEBUG_OBJS) -o $(TARGET_EXEC)_debug $(LDFLAGS) $(CPPFLAGS_debug)

# c++ source for debug
$(DEBUG_BUILD_DIR)/%.cpp.o: %.cpp
	$(MKDIR_P) $(dir $@)
	g++ $(CPPFLAGS_debug) $(CPPFLAGS_debug) -c $< -o $@

.PHONY: clean
clean:
	$(RM) -r $(BUILD_DIR) $(TARGET_EXEC) $(DEBUG_BUILD_DIR) $(TARGET_EXEC)_debug

-include $(DEPS)

MKDIR_P ?= mkdir -p
