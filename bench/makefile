CXX = g++
CXXFLAGS = -std=c++17 -Wall -fsanitize=address -g -O0
LDFLAGS = 

# Source files
SRCS = BpTree.cpp mainBench1.cpp testBp.cpp

# Object files
OBJS = $(SRCS:.cpp=.o)

# Executable names
BENCH_EXEC = mainBench1
TEST_EXEC = testBp

# Directories
BIN_DIR := bin

# Default target
all: $(BIN_DIR)/$(BENCH_EXEC) $(BIN_DIR)/$(TEST_EXEC)

# Create bin directory if it doesn't exist
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Compile the benchmark executable
$(BIN_DIR)/$(BENCH_EXEC): BpTree.o mainBench1.o | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^
	rm -f mainBench1.o

# Compile the test executable
$(BIN_DIR)/$(TEST_EXEC): BpTree.o testBp.o | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^
	rm -f $^

# Compile source files to object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up build artifacts
clean:
	rm -f $(OBJS) $(BIN_DIR)/$(BENCH_EXEC) $(BIN_DIR)/$(TEST_EXEC)

# Run tests
test: $(BIN_DIR)/$(TEST_EXEC)
	./$(BIN_DIR)/$(TEST_EXEC)

.PHONY: all clean test
