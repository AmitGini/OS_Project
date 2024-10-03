# Makefile for building the Graph-related project

# Compiler settings
CXX = g++
CXXFLAGS = -g -std=c++17
COVFLAGS = -fprofile-arcs -ftest-coverage
OBJECTS = main.o Graph.o KruskalStrategy.o PrimStrategy.o Server.o RequestService.o ActiveObjectDP.o PipelineDP.o LeaderFollowerDP.o

# Default target
all: graph

# Rule to link the program
graph: $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^

coverage: CXXFLAGS += $(COVFLAGS)
coverage: graph
	mkdir -p coverage
	-./$(MAIN_TARGET) -p
	-./$(MAIN_TARGET) -l
	gcov main.cpp -o .
	gcov Graph.cpp -o .
	lcov --capture --directory . --output-file coverage/coverage.info
	genhtml coverage/coverage.info --output-directory coverage/html

valgrind: $(MAIN_TARGET)
	valgrind --leak-check=yes --track-origins=yes --gen-suppressions=all ./$(MAIN_TARGET) -p > valgrind_output_Pipeline.txt 2>&1
	valgrind --leak-check=yes --track-origins=yes --gen-suppressions=all ./$(MAIN_TARGET) -l >> valgrind_output_LF.txt 2>&1


# Rule to compile the source files
main.o: main.cpp Graph.hpp Server.hpp MSTFactory.hpp MSTStrategy.hpp RequestService.hpp LeaderFollowerDP.hpp PipelineDP.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

Graph.o: Graph.cpp Graph.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

KruskalStrategy.o: KruskalStrategy.cpp MSTStrategy.hpp KruskalStrategy.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

PrimStrategy.o: PrimStrategy.cpp MSTStrategy.hpp PrimStrategy.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

Server.o: Server.cpp Server.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

RequestService.o: RequestService.cpp RequestService.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

ActiveObjectDP.o: ActiveObjectDP.cpp ActiveObjectDP.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

PipelineDP.o: PipelineDP.cpp RequestService.hpp PipelineDP.hpp ActiveObjectDP.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

LeaderFollowerDP.o: LeaderFollowerDP.cpp RequestService.hpp LeaderFollowerDP.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up
clean:
	rm -f $(OBJECTS) graph 

# Declare phony targets
.PHONY: all clean
