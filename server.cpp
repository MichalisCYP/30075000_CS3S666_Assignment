#include "HeaderFiles/TCPServer.h"
#include <iostream>
using namespace std;

int main()
{

    // Start server / Task Manager
    TCPServer server(8888); // start server on port 8888
    if (!server.start())
    {
        std::cerr << "Failed to start server\n"; // print error
        return 1;
    }
    std::cout << "Thread started" << std::endl;
    std::thread serverThread(&TCPServer::acceptConnections, &server); // attach a thread to the accept connections function which runs on an infinite loop

    serverThread.join();
    return 0;
}