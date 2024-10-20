# Makefile for building the Graph-related project

# OBJECTS:  LeaderFollowerDP.o
# pcoverage:	gcov ActiveObjectDP.cpp -o .
# lfcoverage:	gcov LeaderFollowerDP.cpp -o .
# LeaderFollowerDP.o: LeaderFollowerDP.cpp RequestService.hpp LeaderFollowerDP.hpp
#	 $(CXX) $(CXXFLAGS) -c $< -o $@
# main.o LeaderFollowerDP.hpp


# Compiler settings
CXX = g++
CXXFLAGS = -g -std=c++17
COVFLAGS = -fprofile-arcs -ftest-coverage
OBJECTS = main.o Graph.o KruskalStrategy.o PrimStrategy.o Server.o RequestService.o ActiveObjectDP.o PipelineDP.o FunctionQueue.o

# Default target
all: graph

# Rule to link the program
graph: $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^

pcoverage: CXXFLAGS += $(COVFLAGS)
pcoverage: graph
	mkdir -p coverage
	-./graph -p
	gcov main.cpp -o .
	gcov Graph.cpp -o .
	gcov KruskalStrategy.cpp -o .
	gcov PrimStrategy.cpp -o .
	gcov Server.cpp -o .
	gcov RequestService.cpp -o .
	gcov PipelineDP.cpp -o .
	gcov ActiveObjectDP.cpp -o .
	gcov FunctionQueue.cpp -o .
	lcov --capture --directory . --output-file coverage/coverage.info
	genhtml coverage/coverage.info --output-directory coverage/html

lfcoverage: CXXFLAGS += $(COVFLAGS)
lfcoverage: graph
	mkdir -p coverage
	-./graph -l
	gcov main.cpp -o .
	gcov Graph.cpp -o .
	gcov KruskalStrategy.cpp -o .
	gcov PrimStrategy.cpp -o .
	gcov Server.cpp -o .
	gcov RequestService.cpp -o .
	gcov PipelineDP.cpp -o .
	gcov ActiveObjectDP.cpp -o .
	gcov FunctionQueue.cpp -o .
	lcov --capture --directory . --output-file coverage/coverage.info
	genhtml coverage/coverage.info --output-directory coverage/html
	
valgrind: graph
	valgrind --leak-check=yes --track-origins=yes --gen-suppressions=all ./graph -p > valgrind_output_Pipeline.txt 2>&1
	valgrind --leak-check=yes --track-origins=yes --gen-suppressions=all ./graph -l >> valgrind_output_LF.txt 2>&1

# Rule to compile the source files
main.o: main.cpp Graph.hpp Server.hpp MSTFactory.hpp MSTStrategy.hpp RequestService.hpp PipelineDP.hpp
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

PipelineDP.o: PipelineDP.cpp RequestService.hpp PipelineDP.hpp ActiveObjectDP.hpp FunctionQueue.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

ActiveObjectDP.o: ActiveObjectDP.cpp ActiveObjectDP.hpp FunctionQueue.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

FunctionQueue.o: FunctionQueue.cpp FunctionQueue.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up
clean:
	rm -f *.o graph *.gcda *.gcno *.gcov gmon.out callgrind.out.* valgrind_output.txt callgraph_report.txt
	rm -rf coverage profile_data callgraph_data out 

# Declare phony targets
.PHONY: all clean
