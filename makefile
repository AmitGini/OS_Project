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


# Generate callgraph for Pipe Active Object
callgraph_pipe: client_script.sh graph
	chmod +x client_script.sh
# Start the server in the background
	./graph -p & echo $$! > server_pid.txt
# Wait until the server is listening on port 4040
	sleep 1 && while ! ss -tln | grep -q ":4040"; do sleep 0.5; done
# Run the client script
	./client_script.sh || true
# Run valgrind for callgraph analysis
	valgrind --tool=callgrind --dump-instr=yes --simulate-cache=yes --collect-jumps=yes ./graph -p
# Annotate the callgraph
	@callgrind_file=$$(ls callgrind.out.* | head -n 1) && \
	echo "Using $$callgrind_file for callgraph" && \
	callgrind_annotate $$callgrind_file > callgraph_report_PipeAO.txt && \
	mv $$callgrind_file callgraph_data_PipeAO.txt

# Generate callgraph for Leader Follower
callgraph_lf: client_lf_script.sh graph
	chmod +x client_lf_script.sh
# Start the server in the background
	./graph -l & echo $$! > server_pid.txt
# Wait until the server is listening on port 4040
	sleep 1 && while ! ss -tln | grep -q ":4040"; do sleep 0.5; done
# Run the client script	
	./client_lf_script.sh || true		
# Run valgrind for callgraph analysis
	valgrind --tool=callgrind --dump-instr=yes --simulate-cache=yes --collect-jumps=yes ./graph -l
# Annotate the callgraph
	@callgrind_file=$$(ls callgrind.out.* | head -n 1) && \
	echo "Using $$callgrind_file for callgraph" && \
	callgrind_annotate $$callgrind_file > callgraph_report_LF.txt && \
	mv $$callgrind_file callgraph_data_LF.txt

# Generate code coverage
coverage: CXXFLAGS += -fprofile-arcs -ftest-coverage
coverage: LDFLAGS += -fprofile-arcs
coverage: graph
# Start server with Pipe pattern
	./graph -p & echo $$! > server_pid.txt
	sleep 1 && while ! ss -tln | grep -q ":4040"; do sleep 0.5; done
	./client_script.sh || true
	sleep 1 && kill $$(cat server_pid.txt)

# Start server with Leader-Follower pattern
	./graph -l & echo $$! > server_pid.txt
	sleep 1 && while ! ss -tln | grep -q ":4040"; do sleep 0.5; done
	./client_lf_script.sh || true
	sleep 1 && kill $$(cat server_pid.txt)

# Generate coverage reports
	gcov main.cpp Graph.cpp KruskalStrategy.cpp PrimStrategy.cpp Server.cpp \
		RequestService.cpp PipeDP.cpp ActiveObjectDP.cpp LeaderFollowerDP.cpp TaskQueue.cpp

# Create lcov report
	lcov --capture --directory . --output-file coverage.info
	genhtml coverage.info --output-directory coverage

# Run Memcheck for Pipe Active Object
memcheck_pipe: client_script.sh graph
	chmod +x client_script.sh
# Start the server in the background
	./graph -p & echo $$! > server_pid.txt
# Wait until the server is listening on port 4040
	sleep 1 && while ! ss -tln | grep -q ":4040"; do sleep 0.5; done
# Run the client script
	./client_script.sh || true
# Run valgrind Memcheck analysis
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./graph -p 2> memcheck_pipe_output.txt
# Stop the server
	kill $$(cat server_pid.txt) || true
	rm -f server_pid.txt

# Run Memcheck for Leader Follower
memcheck_lf: client_lf_script.sh graph
	chmod +x client_lf_script.sh
# Start the server in the background
	./graph -l & echo $$! > server_pid.txt
# Wait until the server is listening on port 4040
	sleep 1 && while ! ss -tln | grep -q ":4040"; do sleep 0.5; done
# Run the client script
	./client_lf_script.sh || true
# Run valgrind Memcheck analysis
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./graph -l 2> memcheck_lf_output.txt
# Stop the server
	kill $$(cat server_pid.txt) || true
	rm -f server_pid.txt

# Run Helgrind for Pipe Active Object
helgrind_pipe: client_script.sh graph
	chmod +x client_script.sh
# Start the server in the background
	./graph -p & echo $$! > server_pid.txt
# Wait until the server is listening on port 4040
	sleep 1 && while ! ss -tln | grep -q ":4040"; do sleep 0.5; done
# Run the client script
	./client_script.sh || true
# Run Helgrind analysis
	valgrind --tool=helgrind --log-file=helgrind_pipe.log ./graph -p
	@echo "Helgrind analysis for PipeAO saved to helgrind_pipe.log"

# Run Helgrind for Leader Follower
helgrind_lf: client_lf_script.sh graph
	chmod +x client_lf_script.sh
# Start the server in the background
	./graph -l & echo $$! > server_pid.txt
# Wait until the server is listening on port 4040
	sleep 1 && while ! ss -tln | grep -q ":4040"; do sleep 0.5; done
# Run the client script
	./client_lf_script.sh || true
# Run Helgrind analysis
	valgrind --tool=helgrind --log-file=helgrind_lf.log ./graph -l
	@echo "Helgrind analysis for LeaderFollower saved to helgrind_lf.log"

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
	rm -f *.o graph *.gcda *.gcno *.gcov gmon.out callgrind.out.* *.txt *.log

# Declare phony targets
.PHONY: all clean
