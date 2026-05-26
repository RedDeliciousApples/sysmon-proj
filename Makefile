# Partially AI generated because who has time for this? Not me
# Name of the final executable that will be created
TARGET_EXEC := final_program

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


CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra

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

# "clean" is not a real file target
.PHONY: clean

# Remove all generated build files
clean:
	rm -r $(BUILD_DIR)

# Include automatically generated dependency files
#
# This allows Make to rebuild files when included headers change.
# The "-" suppresses errors if the .d files do not exist yet.
-include $(DEPS)