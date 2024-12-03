# Makefile for building the Graph-related project

# Compiler settings
CXX = g++
CXXFLAGS = -g -std=c++17
COVFLAGS = -fprofile-arcs -ftest-coverage
OBJECTS = Server.o Graph.o KruskalStrategy.o PrimStrategy.o Pipeline.o ActiveObject.o LeaderFollower.o

# Default target
all: graph

# Rule to link the program
graph: $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Generate callgraph
callgraph: client_script.sh graph
	chmod +x client_script.sh
# Start the server in the background
	./graph & echo $$! > server_pid.txt
# Wait until the server is listening on port 4040
	sleep 1 && while ! ss -tln | grep -q ":4040"; do sleep 0.5; done
# Run the client script
	./client_script.sh || true
# Run valgrind for callgraph analysis
	valgrind --tool=callgrind --dump-instr=yes --simulate-cache=yes --collect-jumps=yes ./graph
# Annotate the callgraph
	@callgrind_file=$$(ls callgrind.out.* | head -n 1) && \
	echo "Using $$callgrind_file for callgraph" && \
	callgrind_annotate $$callgrind_file > callgraph_report.txt && \
	mv $$callgrind_file callgraph_data.txt


# Run Memcheck for Pipe Active Object
memcheck: client_script.sh graph
	chmod +x client_script.sh
# Start the server in the background
	./graph & echo $$! > server_pid.txt
# Wait until the server is listening on port 4040
	sleep 1 && while ! ss -tln | grep -q ":4040"; do sleep 0.5; done
# Run the client script
	./client_script.sh || true
# Run valgrind Memcheck analysis
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./graph 2> memcheck_output.txt
# Stop the server
	kill $$(cat server_pid.txt) || true
	rm -f server_pid.txt


# Run Helgrind for Pipe Active Object
helgrind: client_script.sh graph
	chmod +x client_script.sh
# Start the server in the background
	./graph & echo $$! > server_pid.txt
# Wait until the server is listening on port 4040
	sleep 1 && while ! ss -tln | grep -q ":4040"; do sleep 0.5; done
# Run the client script
	./client_script.sh || true
# Run Helgrind analysis
	valgrind --tool=helgrind --log-file=helgrind_report.log ./graph
	@echo "Helgrind analysis saved to helgrind_report.log"


# Target to compile with coverage flags
coverage: CXXFLAGS += $(COVFLAGS)  # Add coverage flags
coverage: graph # compile graph with coverage flags
# Create coverage directory
	mkdir -p coverage
	-./graph
# Generate coverage data in coverage directory
	mkdir -p coverage/gcov
	gcov -o . Graph.cpp > coverage/gcov/Graph.cpp.gcov
	gcov -o . KruskalStrategy.cpp > coverage/gcov/KruskalStrategy.cpp.gcov
	gcov -o . PrimStrategy.cpp > coverage/gcov/PrimStrategy.cpp.gcov
	gcov -o . Server.cpp > coverage/gcov/Server.cpp.gcov
	gcov -o . Pipeline.cpp > coverage/gcov/Pipeline.cpp.gcov
	gcov -o . ActiveObject.cpp > coverage/gcov/ActiveObject.cpp.gcov
	gcov -o . LeaderFollower.cpp > coverage/gcov/LeaderFollower.cpp.gcov
	mv *.gcov coverage/gcov/
	mv *.gcda coverage/
	mv *.gcno coverage/
# Generate HTML report
	lcov --capture --directory coverage --output-file coverage/coverage.info
	lcov --remove coverage/coverage.info '/usr/*' --output-file coverage/coverage.info
	genhtml coverage/coverage.info --output-directory coverage/html

# Rule to compile the source files
Server.o: Server.cpp Server.hpp Graph.hpp  MSTFactory.hpp MSTStrategy.hpp Pipeline.hpp ActiveObject.hpp LeaderFollower.hpp 
	$(CXX) $(CXXFLAGS) -c $< -o $@

Graph.o: Graph.cpp Graph.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

KruskalStrategy.o: KruskalStrategy.cpp Graph.hpp MSTStrategy.hpp KruskalStrategy.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

PrimStrategy.o: PrimStrategy.cpp Graph.hpp MSTStrategy.hpp PrimStrategy.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

ActiveObject.o: ActiveObject.cpp Graph.hpp ActiveObject.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

Pipeline.o: Pipeline.cpp Graph.hpp Pipeline.hpp ActiveObject.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

LeaderFollower.o: LeaderFollower.cpp Graph.hpp LeaderFollower.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@


# Clean up
clean:
	rm -f *.o graph *.gcda *.gcno *.gcov gmon.out callgrind.out.* *.txt *.log *.info
	rm -rf profile_data callgraph_data html out

# Declare phony targets
.PHONY: all clean