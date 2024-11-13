
# OS Project - MST, Strategy/Factory, Client-Server, Threads, Active Object, Thread Pool (Leader-Follower) and Valgrind

### Overview

This project implements various **design patterns** and **algorithms** in **C++** including:
- **Minimum Spanning Tree** (MST) algorithms (Prim's and Kruskal's).
- **Strategy** and **Factory** design patterns.
- **Client-Server** architecture.
- **Active Object** pattern.
- **Thread Pool** using **Leader-Follower** pattern.
- **Memory leak detection** using Valgrind.
- **Race Condition, Deadlocks, Thread Safety Analysis**, using Helgrind.

### Project Structure

- **Graph**: Implements graph data structure and MST algorithms.
- **MSTStrategy**: Abstract class for MST algorithms.
- **PrimStrategy**: Implements Prim's algorithm for MST.
- **KruskalStrategy**: Implements Kruskal's algorithm for MST.
- **MSTFactory**: Factory class to create MST strategy objects.
- **RequestService**: Abstract class for handling client requests.
- **TaskQueue**: Implements a task queue for Active Object pattern.
- **ActiveObjectDP**: Implements Active Object pattern.
- **PipeDP**: Implements a pipeline of Active Objects.
- **LeaderFollowerDP**: Implements Leader-Follower thread pool pattern.
- **Server**: Implements the server handling client connections.

## Getting Started

### Prerequisites

- C++17 or later
- Makefile
- Valgrind (for memory leak detection)

### Building the Project

1. Clone the repository:
    ```bash
    git clone https://github.com/AmitGini/OS_Project.git
    cd OS_Project
    ```

2. Build the project using the Makefile:
    ```bash
    make
    ```

### Running the Server

1. Start the server:
    ```bash
    ./graph -p  # For PipeDP
    ./graph -l  # For LeaderFollowerDP
    ```

### Debug Options

1. **Valgrind Memory Check**: Run Valgrind to check for memory leaks.
   ```bash
   make memcheck_pipe
   make memcheck_lf
   ```

2. **Helgrind Thread Error Detector**: Run Helgrind to detect synchronization errors.
   ```bash
   make helgrind_pipe
   make helgrind_lf
   ```

3. **Callgrind Performance Profiler**: Generate callgraphs to analyze performance.
   ```bash
   make callgraph_pipe
   make callgraph_lf
   ```

4. **Coverage Testing**: Generate code coverage reports.
   ```bash
   make coverage
   ```

## Project Details

### Graph

The `Graph` class represents a graph using an adjacency matrix and provides methods to add/remove edges and compute MST.

### MSTStrategy

The `MSTStrategy` class is an abstract base class for MST algorithms. It defines a method `computeMST` that must be implemented by derived classes.

### PrimStrategy

The `PrimStrategy` class implements Prim's algorithm for computing MST.

### KruskalStrategy

The `KruskalStrategy` class implements Kruskal's algorithm for computing MST.

### MSTFactory

The `MSTFactory` class provides a method to create MST strategy objects based on the specified algorithm type.

### RequestService

The `RequestService` class is an abstract base class for handling client requests. It provides methods to create/modify graphs, compute MST, and send messages to clients.

### TaskQueue

The `TaskQueue` class implements a task queue for the Active Object pattern. It provides methods to enqueue and execute tasks.

### ActiveObjectDP

The `ActiveObjectDP` class implements the Active Object pattern. It manages a task queue and a worker thread to process tasks asynchronously.

### PipeDP

The `PipeDP` class implements a pipeline of Active Objects. It sets up a series of stages, each represented by an `ActiveObjectDP`, and connects them to form a pipeline.

### LeaderFollowerDP

The `LeaderFollowerDP` class implements the Leader-Follower thread pool pattern. It manages a pool of threads to handle client requests and execute tasks.

### Server

The `Server` class handles client connections and delegates request processing to the appropriate design pattern (PipeDP or LeaderFollowerDP).

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- [Valgrind](http://valgrind.org/) for more information on memory leak detection.
- [Makefile Tutorial](https://makefiletutorial.com/) for more information about Makefiles.
