# Partially AI generated because who has time for this? Not me
TARGET_EXEC := sysmon
BUILD_DIR := ./build
SRC_DIRS := ./src

CPPFLAGS := -Iinclude -Iexternal -MMD -MP

CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -Wpedantic -Wshadow
CXXFLAGS += -pthread
LDFLAGS += -pthread

# Production application sources and objects.
APP_SRCS := $(shell find $(SRC_DIRS) -name '*.cpp')
APP_OBJS := $(APP_SRCS:%=$(BUILD_DIR)/%.o)
APP_DEPS := $(APP_OBJS:.o=.d)

# Sampler test: explicitly list only what it needs.
TEST_EXEC := $(BUILD_DIR)/sampler_test
TEST_SRCS := \
	tests/sampler_test.cpp \
	src/collectors/metrics_sampler.cpp \
	src/collectors/cpu.cpp \
	src/collectors/mem.cpp \
	src/collectors/uptime.cpp \
	src/collectors/loadavg.cpp \
	src/collectors/storage.cpp \
	src/utils/math_utils.cpp

TEST_OBJS := $(TEST_SRCS:%=$(BUILD_DIR)/%.o)
TEST_DEPS := $(TEST_OBJS:.o=.d)

.PHONY: clean run test

# Production executable.
$(BUILD_DIR)/$(TARGET_EXEC): $(APP_OBJS)
	mkdir -p $(dir $@)
	$(CXX) $(APP_OBJS) -o $@ $(LDFLAGS)

# Test executable.
$(TEST_EXEC): $(TEST_OBJS)
	mkdir -p $(dir $@)
	$(CXX) $(TEST_OBJS) -o $@ $(LDFLAGS)

# Compile any source into its matching build-directory object.
$(BUILD_DIR)/%.cpp.o: %.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

run: $(BUILD_DIR)/$(TARGET_EXEC)
	$(BUILD_DIR)/$(TARGET_EXEC)

test: $(TEST_EXEC)
	$(TEST_EXEC)

clean:
	rm -rf $(BUILD_DIR)

-include $(APP_DEPS) $(TEST_DEPS)