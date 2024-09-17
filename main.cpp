#include "Server.hpp"
#include <iostream>

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        std::cerr << "Usage: server <port> <numThreads>\n";
        return 1;
    }

    int port = std::atoi(argv[1]);
    int numThreads = std::atoi(argv[2]);

    try
    {
        Server server(port, numThreads);
        server.start();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
