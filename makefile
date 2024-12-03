# Makefile for building the Graph-related project

# Compiler settings
CXX = g++
CXXFLAGS = -g
COVFLAGS = -fprofile-arcs -ftest-coverage -g
OBJECTS = Server.o Graph.o KruskalStrategy.o PrimStrategy.o Pipeline.o ActiveObject.o LeaderFollower.o

# Default target
all: graph

# Rule to link the program
graph: $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# run callgrind in the terminal
callgrind: clean client_script.sh graph
	rm -rf callgrind_data
	chmod +x client_script.sh
	valgrind --tool=callgrind --dump-instr=yes --simulate-cache=yes --collect-jumps=yes ./graph & echo $$! > server_pid.txt
	sleep 1 && while ! ss -tln | grep -q ":4040"; do sleep 0.5; done
	./client_script.sh
	kill $$(cat server_pid.txt)
	sleep 1
	@callgrind_file=$$(ls callgrind.out.* | sort -n | tail -n 1) && \
	echo "Using $$callgrind_file for callgrind_report" && \
	callgrind_annotate $$callgrind_file > callgrind_report.txt && \
	mkdir -p callgrind_data && \
	mv $$callgrind_file callgrind_data/ && \
	mv callgrind_report.txt callgrind_data/
	rm -f server_pid.txt

# Run memcheck in the terminal
memcheck: clean client_script.sh graph
	rm -rf memcheck_data
	mkdir -p memcheck_data
	chmod +x client_script.sh
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --log-file=memcheck_data/memcheck_output.txt ./graph & echo $$! > server_pid.txt
	sleep 1 && while ! ss -tln | grep -q ":4040"; do sleep 0.5; done
	./client_script.sh
	kill $$(cat server_pid.txt)
	sleep 1
	rm -f server_pid.txt

# Run helgrind in the terminal
helgrind: clean client_script.sh graph
	rm -rf helgrind_data
	mkdir -p helgrind_data
	chmod +x client_script.sh
	valgrind --tool=helgrind --log-file=helgrind_data/helgrind_report.log ./graph & echo $$! > server_pid.txt
	sleep 1 && while ! ss -tln | grep -q ":4040"; do sleep 0.5; done
	./client_script.sh
	kill $$(cat server_pid.txt)
	sleep 1
	rm -f server_pid.txt
	@echo "Helgrind analysis saved to helgrind_data/helgrind_report.log"

# Target to compile with coverage flags
coverage: CXXFLAGS = $(COVFLAGS)
coverage: clean client_script.sh graph
	rm -rf coverage_data
	rm -rf coverage_report
	chmod +x client_script.sh
	mkfifo server_input
	./graph < server_input & echo $$! > server_pid.txt
	sleep 1 && while ! ss -tln | grep -q ":4040"; do sleep 0.5; done
	./client_script.sh | tee server_input
	echo "stop" > server_input
	sleep 5
	kill $$(cat server_pid.txt) || true
	rm -f server_pid.txt
	rm -f server_input
	sleep 1
	lcov --capture --directory . --output-file coverage.info
	lcov --remove coverage.info '/usr/*' --output-file coverage.info
	genhtml coverage.info --output-directory coverage_report
	mkdir -p coverage_data/gcov
	mv *.gcda coverage_data/
	mv *.gcno coverage_data/
	mv *.gcov coverage_data/gcov/ || true
	rm -f coverage.info


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
	rm -f *.o graph *.gcda *.gcno *.gcov gmon.out callgrind.out.* *.txt *.log *.info server_input

# Declare phony targets
.PHONY: all clean