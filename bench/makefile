CXX = clang++
CXXFLAGS = -std=c++17 -Wall -fsanitize=address
LDFLAGS = 

# Source files
SRCS = BpTree.cpp mainBench1.cpp testBp.cpp

# Object files
OBJS = $(SRCS:.cpp=.o)

# Executable names
BENCH_EXEC = mainBench1
TEST_EXEC = testBp

# Default target
all: $(BENCH_EXEC)

# Compile the benchmark executable
$(BENCH_EXEC): BpTree.o mainBench1.o
	$(CXX) $(CXXFLAGS) -o $@ $^

# Compile the test executable
$(TEST_EXEC): BpTree.o testBp.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^

# Compile source files to object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@


# Clean up build artifacts
clean:
	rm -f $(OBJS) $(BENCH_EXEC) $(TEST_EXEC)

# Run tests
test: $(TEST_EXEC)
	./$(TEST_EXEC)

.PHONY: all clean test
