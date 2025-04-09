# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -std=c++17 $(shell wx-config --cxxflags)

# Linker flags
LDFLAGS = $(shell wx-config --libs)

# Directories
SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = build

# Source files
SOURCES = $(wildcard $(SRC_DIR)/*.cpp)

# Object files
OBJECTS = $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SOURCES))

# Executable name
TARGET = simulator.exe

# Default target
all: $(BUILD_DIR) $(TARGET)

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Compile source files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -I$(INCLUDE_DIR) -c $< -o $@

# Link object files
$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) $(LDFLAGS) -o $@

# Clean build files
clean:
	rm -rf $(BUILD_DIR) $(TARGET)

# Phony targets
.PHONY: all clean

