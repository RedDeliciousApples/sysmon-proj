# Partially AI generated because who has time for this? Not me
# Name of the final executable that will be created
TARGET_EXEC := sysmon

# Folder where compiled object files and executable will go
BUILD_DIR := ./build

# Folder containing source code (.cpp files)
SRC_DIRS := ./src

# Compiler preprocessor flags:
# -Iinclude  -> look in include/ for header files
# -Iexternal -> look in external/ for header files
# -MMD -MP   -> automatically generate dependency (.d) files
CPPFLAGS := -Iinclude -Iexternal -MMD -MP

# Find all .cpp source files inside src/
SRCS := $(shell find $(SRC_DIRS) -name '*.cpp')

# Convert source file names into object file names inside build/
#
# Example:
# src/main.cpp
# becomes:
# build/src/main.cpp.o
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)

# Convert object file names into dependency file names
#
# Example:
# build/src/main.cpp.o
# becomes:
# build/src/main.cpp.d
DEPS := $(OBJS:.o=.d)

# Cross-compilation support:
#   make CROSS_COMPILE=aarch64-linux-gnu-
# uses aarch64-linux-gnu-g++ automatically.
# (Buildroot also passes CXX=... on the command line, which overrides this.)
CROSS_COMPILE ?=
CXX := $(CROSS_COMPILE)g++

# -pthread is required for std::thread (MetricsSampler) to link correctly,
# especially on non-glibc targets like musl/uClibc
CXXFLAGS := -std=c++17 -Wall -Wextra -pthread

# Linker flags.
# STATIC=1 produces a fully static binary, handy for dropping onto boards
# that don't have matching shared libraries installed:
#   make STATIC=1 CROSS_COMPILE=aarch64-linux-gnu-
LDFLAGS := -pthread
ifeq ($(STATIC),1)
LDFLAGS += -static
endif

# Default target
.PHONY: all
all: $(BUILD_DIR)/$(TARGET_EXEC)

# Final linking step to combine into one executable
$(BUILD_DIR)/$(TARGET_EXEC): $(OBJS)
	mkdir -p $(dir $@)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)

# Rule for compiling any .cpp file into a .o object file
#
# $< = input file
# $@ = output file
$(BUILD_DIR)/%.cpp.o: %.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

# Install to a staging directory, e.g.:
#   make install DESTDIR=/path/to/staging PREFIX=/usr
# This is what a Buildroot .mk file will call.
PREFIX ?= /usr
.PHONY: install
install: $(BUILD_DIR)/$(TARGET_EXEC)
	install -D -m 0755 $(BUILD_DIR)/$(TARGET_EXEC) $(DESTDIR)$(PREFIX)/bin/$(TARGET_EXEC)

# "clean" is not a real file target
.PHONY: clean

# Remove all generated build files
clean:
	rm -rf $(BUILD_DIR)

# make run to run it (host build only)
.PHONY: run
run: $(BUILD_DIR)/$(TARGET_EXEC)
	$(BUILD_DIR)/$(TARGET_EXEC)

# Include automatically generated dependency files
#
# This allows Make to rebuild files when included headers change.
# The "-" suppresses errors if the .d files do not exist yet.
-include $(DEPS)
