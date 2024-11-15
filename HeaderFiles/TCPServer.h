// The server and client code have been adapted from the following sources:
//  https://www.geeksforgeeks.org/socket-programming-cc/
//  https://stackoverflow.com/questions/662328/what-is-a-simple-c-or-c-tcp-server-and-client-example
// https://www.geeksforgeeks.org/tcp-server-client-implementation-in-c/
//  https://github.com/bozkurthan/Simple-TCP-Server-Client-CPP-Example?tab=readme-ov-file

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <thread>
#include <vector>
#include "Database.h"
using namespace std;
Database db;

class TCPServer
{
public:
    TCPServer(int port) : port(port), serverSocket(-1) {} // Constructor
    bool start()
    {
        // The function creates a socket, binds it to an IP address and port, and sets the socket as a listening socket

        // Create a socket
        serverSocket = socket(AF_INET, SOCK_STREAM, 0); // IPv4, TCP
        if (serverSocket < 0)
        {
            std::cerr << "Error creating socket\n";
            return false;
        }
        // Bind the socket to an IP address and port
        serverAddr.sin_family = AF_INET;         // IPv4
        serverAddr.sin_addr.s_addr = INADDR_ANY; // Bind to any available interface
        serverAddr.sin_port = htons(port);       // Convert port to network byte order
        if (::bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
        { // Bind socket to address
            std::cerr << "Error binding socket\n";
            return false;
        }
        // Start listening for incoming connections
        if (listen(serverSocket, 5) < 0)
        { // set the server socket as a listening socket and allow 5 connections
            // if listen returns -1, there was an error
            std::cerr << "Error listening on socket\n";
            return false;
        }
        std::cout << "Server listening on port " << port << std::endl;
        return true;
    }

    // THREADING
    void acceptConnections()
    {
        // Accept incoming connections
        int clientSocket;
        struct sockaddr_in clientAddr;                 // Client address
        socklen_t clientAddrSize = sizeof(clientAddr); // get size of client address for accept function

        while ((clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrSize)))
        { // runs until clientSocket is -1, meaning there was an error

            std::thread(&TCPServer::handleClient, this, clientSocket).detach(); // Detach thread and attach to handleClient function
            // detach() allows the thread to run independently of the main thread
        }
    }

    void handleClient(int clientSocket)
    {
        char buffer[1024]; // Buffer to store message from client

        while (true) // maintains the 1-1 connection until the client closes it
        {
            // Read message from client
            int bytesRead = read(clientSocket, buffer, sizeof(buffer));
            if (bytesRead < 0)
            {
                std::cerr << "Error reading from socket\n";
                break;
            }
            else if (bytesRead == 0)
            {
                // Client closed the connection
                std::cout << "Client disconnected\n";
                break;
            }
            if (buffer[0] == 'c') // if the first character of the buffer is 'c', close the connection
            {
                break;
            }

            // Process the request
            string result = db.handleRequest(buffer); // pass the request to the database to handle

            if (write(clientSocket, result.c_str(), result.size()) < 0) // Write the result back to the client
            {
                std::cerr << "Error writing to socket\n";
                break;
            }
        }

        close(clientSocket); // Close the client socket
        std::cout << "Socket closed\n";
    }

    ~TCPServer() // Destructor
    {
        if (serverSocket >= 0)
        {
            close(serverSocket);
        }
    }

private:
    int port;
    int serverSocket;
    struct sockaddr_in serverAddr; // Server address
};
