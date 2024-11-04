# Makefile for building the Graph-related project

# Compiler settings
CXX = g++
CXXFLAGS = -g -std=c++17
COVFLAGS = --coverage -fprofile-arcs -ftest-coverage
OBJECTS = main.o Graph.o KruskalStrategy.o PrimStrategy.o Server.o RequestService.o PipeDP.o ActiveObjectDP.o LeaderFollowerDP.o TaskQueue.o

# Default target
all: graph

# Rule to link the program
graph: $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^

coverage: CXXFLAGS += $(COVFLAGS)  # Add coverage flags
coverage: graph # compile graph with coverage flags
# Create the coverage directory
	mkdir -p coverage
# Run the program
	-./graph -p -l
	-./graph -k
	-./graph asfasf 12312
	-./graph
	-./graph -p
	-./graph -l
# Generate coverage data
	gcov main.cpp -o .
	gcov Graph.cpp -o .
	gcov KruskalStrategy.cpp -o .
	gcov PrimStrategy.cpp -o .
	gcov Server.cpp -o .
	gcov RequestService.cpp -o .
	gcov PipeDP.cpp -o .
	gcov ActiveObjectDP.cpp -o .
	gcov TaskQueue.cpp -o .
	gcov LeaderFollowerDP.cpp -o .
# Generate coverage report
	lcov --capture --directory . --output-file coverage/coverage.info
	genhtml coverage/coverage.info --output-directory coverage/html

valgrind: graph
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --gen-suppressions=all ./graph -p >> valgrind_output_PipeAo.txt 2>&1
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --gen-suppressions=all ./graph -l >> valgrind_output_LF.txt 2>&1

helgrind: graph
	valgrind --verbose --log-file=helgrind_PipeActiveObject.log ./graph -p
	valgrind --verbose --log-file=helgrind_LeaderFollower.log ./graph -l

	

# Rule to compile the source files
main.o: main.cpp Graph.hpp Server.hpp MSTFactory.hpp MSTStrategy.hpp RequestService.hpp PipeDP.hpp LeaderFollowerDP.hpp
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

ActiveObjectDP.o: ActiveObjectDP.cpp PipeDP.hpp ActiveObjectDP.hpp TaskQueue.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

PipeDP.o: PipeDP.cpp RequestService.hpp PipeDP.hpp ActiveObjectDP.hpp TaskQueue.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

LeaderFollowerDP.o: LeaderFollowerDP.cpp RequestService.hpp LeaderFollowerDP.hpp TaskQueue.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

TaskQueue.o: TaskQueue.cpp TaskQueue.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up
clean:
	rm -f *.o graph *.gcda *.gcno *.gcov gmon.out callgrind.out.* valgrind_output_LF.txt valgrind_output_PipeAo.txt helgrind_LeaderFollower.log helgrind_PipeActiveObject.log
	rm -rf coverage profile_data callgraph_data out 
	-fuser -k 4040/tcp
	-sudo sysctl -w net.ipv4.tcp_tw_reuse=1

# Declare phony targets
.PHONY: all clean
