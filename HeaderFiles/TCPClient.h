// The server and client code have been adapted from the following sources:
//  https://www.geeksforgeeks.org/socket-programming-cc/
//  https://stackoverflow.com/questions/662328/what-is-a-simple-c-or-c-tcp-server-and-client-example
// https://www.geeksforgeeks.org/tcp-server-client-implementation-in-c/
//  https://github.com/bozkurthan/Simple-TCP-Server-Client-CPP-Example?tab=readme-ov-file
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>

class TCPClient
{
public:
    TCPClient(const std::string &serverAddress, int port) : serverAddress(serverAddress), port(port), clientSocket(-1) {} // Constructor

    bool connectToServer()
    {
        // Create a socket
        clientSocket = socket(AF_INET, SOCK_STREAM, 0); // IPv4, TCP
        if (clientSocket < 0)                           // socket creation fails
        {
            std::cerr << "Error creating socket\n";
            return false;
        }
        // Connect to the server
        serverAddr.sin_family = AF_INET;                                                   // IPv4
        serverAddr.sin_port = htons(port);                                                 // Convert port to network byte order
        inet_pton(AF_INET, serverAddress.c_str(), &serverAddr.sin_addr);                   // Convert IP address to binary form
        if (connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) // connect to server, passing in the socket, server address, and size of server address
        {
            std::cerr << "Error connecting to server\n";
            return false;
        }
        return true;
    }

    bool sendMessage(const std::string &message) // send message to server
    {
        // Send a message to the server
        if (write(clientSocket, message.c_str(), message.size()) < 0) // write to the socket, passing in the socket, message, and size of message
        // if write returns -1, there was an error
        {
            std::cerr << "Error writing to socket\n";
            return false;
        }
        return true;
    }

    std::string receiveMessage()
    {
        std::string message;
        char buffer[1024]; //reads 1024 bytes at a time
        int bytesRead; 

        while ((bytesRead = read(clientSocket, buffer, sizeof(buffer))) > 0) 
        //read function returns the number of bytes read and takes in the socket, &buffer, and size of buffer
        {
            message.append(buffer, bytesRead); // append the buffer to the message
            if (bytesRead < sizeof(buffer)) 
            {
                break; // No more data to read
            }
        }

        if (bytesRead < 0)
        {
            std::cerr << "Error reading from socket\n";
            return "";
        }

        return message;
    }

    ~TCPClient()
    {
        if (clientSocket >= 0)
        {
            close(clientSocket);
        }
    }

private:
    std::string serverAddress; // IP address of the server
    int port;                 // Port number
    int clientSocket; 
    struct sockaddr_in serverAddr;
};
