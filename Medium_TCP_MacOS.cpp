//AggiE Challenge Spring 2025

#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>

//Following along with Medium tutorial for MacOS TCP implementation.

//Next: implementation of muiltiple calls with vectors

/*
ntohs -->> network to host short
*/

int  main(){
    // No socket initilization for Mac. Different for Windows. 

    // Creating socket with AF_INET address, SOCK_STREAM protocol
    int listening = socket(AF_INET, SOCK_STREAM, 0);
    if(listening == -1){
        std::cerr << "Can't create a socket! Quiting..."<< std::endl;
        return -1;
    }

    // bind the ip address and port to a socket
    // this is a structure
    sockaddr_in hint;
    hint.sin_family = AF_INET; // v4 protocol
    hint.sin_port = htons(54000);
    hint.sin_addr.s_addr = INADDR_ANY; // could also use inet_pton... (What is inet_pton? converts a string address into binary AF_INET representation)
    bind(listening, reinterpret_cast<sockaddr*>(&hint), sizeof(hint)); //hint is the socket address created above

    // LISTENING socket: specifically tailored to receive messages
    listen(listening, SOMAXCONN);

    // New socket: client, waiting to connect to the server. Size of client, in bytes, is known
    sockaddr_in client;
    socklen_t clientSize = sizeof(client);

    //Accept the socket connection and review error messages, if any
    int clientSocket = accept(listening,reinterpret_cast<sockaddr*>(&client), &clientSize);
    if (clientSocket == -1) //This socket's message couldn't be heard; abort. 
    {
        std::cerr << "Error listening a client socket! Quiting..."<< std::endl;
        return -1;    
    }

    char host[NI_MAXHOST]; // Client's remote name, stored as a character array.
    char service[NI_MAXHOST]; // Character array detailing port connected to (slightly different from ASIO implementation).

    memset(host, 0, NI_MAXHOST); // ZeroMemory(host, NI_MAXHOST);
    memset(service, 0, NI_MAXHOST); //Memset: copies the character for the specified number of iterations in memory

    if(getnameinfo(reinterpret_cast<sockaddr*>(&client),sizeof(client),host, NI_MAXHOST, service, NI_MAXSERV, 0)== 0){
        std::cout << host << " connected on port " << service << std::endl; //connected to local 8000
    } else {
        inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST); //connected to some other port
        std::cout << host << " connected on port " << ntohs(client.sin_port) << std::endl; 
    }
    
    // Close listening socket when message has been received
    close(listening);

    // Buffer is very large to accept incoming message and echo back to client.
    char buf[4096];

    while(true){
        memset(buf, 0, 4096);
        // Waiting for client to send data and sending data to buffer created
        int bytesReceived = recv(clientSocket, buf, 4096, 0);
        if (bytesReceived == -1) //Nothing was received, but an error (byte data should never be negative)
        {
            std::cerr << "Error in recv(). Quiting..." << std::endl;
            break;
        }
        if (bytesReceived == 0){ // if zero bytes received, meaning the connection was closed.
            std::cout << "Client disconnected..." << std::endl;
            break;
        }
        std::cout << std::string(buf, 0, bytesReceived) << std::endl; //Benchmark function
        //Echo message back to client by sending from host to client
        send(clientSocket, buf, bytesReceived + 1, 0);
    }

    // Close procedure (differs from ASIO in sense that close is its own function outside of socket scope)
    close(clientSocket);

    // Because socket wasn't initialized, it doesn't need to be shut down
    return 0;
}
