//AggiE Challenge Spring 2025

/* With reference from: 
https://www.youtube.com/watch?v=dquxuXeZXgo 
"Multiple Chat Clients, One Thread" 

select(): FD_CLR (remove from set), FD_SET (add to set), FD_ZERO (clear set) -- ref 
("fail descriptors") that works with fd_set (array of nums) representing 
CLIENTS and SOCKETS connecting to the server */

#include <iostream> 
#include <WS2tcpip.h> //https://learn.microsoft.com/en-us/windows/win32/api/ws2tcpip/
#include <string>
#include <strstream>

//Pragma include: special instructions to compiler
#pragma comment (lib, "ws2_32.lib") //Windows Sockets 2

using namespace std;

void main()
{
    //Winslock initialization 
    WSADATA wsData; 
    WORD ver = MAKEWORD(2, 2); //create a 16-bit integer

    int wsOk = WSAStartup(ver, &wasData); // Returns 0 if initialization successfully created
    if (wsOk != 0)
    {
        cerr << " Cannot initialize Windows Socket. " << endl;
        return;
    }

    //Creating sockets...
    SOCKET listening = socket(AF_INET, SOCK_STREAM, 0) //See ref document prepared before
    if (listening == INVALID_SOCKET) //Listening socket allows server to wait for connections
    {
        cerr << "Cannot create socket." << endl;
        return;
    }

    //IP address bound to socket
    sockaddr_in hint; 
    hint.sin_family = AF_INET; //access the parameters of socket objects
    hint.sin_port = htons(54000);
    hint.sin_addr.S_un.S_addr = INADDR_ANY;

    //Bind a listening socket
    bind(listening, (sockaddr*)&hint, sizeof(hint));

    //Officially "initialize" the listening socket (assignment)
    listen(listening, SOMAXCONN);

    //Definiting set of connections
    fd_set master; 

    //Clear the set initially
    FD_ZERO(&master); //note pass-by-reference

    FD_SET(listening, &master); //Add listening socket to master set

    while (true)
    {
        /*Select destroys fail descriptor upon invokation. 
        Make a copy of the set. */

        fd_set copy = master;

        //For single-threaded application, requests run in sequence.
        int socketCount = select(0, &copy, nullptr, nullptr);

        //Enumerate through list of sockets
        for (int i = 0; i < socketCount; i++)
        {
            //Can only ACCEPT an incoming client request, or RECEIVE a message
            SOCKET sock = copy.fd_array[i]; //array traversal

            if (sock == listening)
            {
                //Accept a new connection to list of connected clients. 
                SOCKET client = accept(listening, nullptr, nullptr); //optional client info

                FD_SET(client, &master);

                string welcomeMessage = "Welcome to the server!\n";

                //Broadcast message to client with respective size
                send(client, welcomeMessage.c_str(), welcomeMessage.size() + 1, 0);

                //Need to indicate that sockets in FailDetector are interested in having messages received from.

            }
            else
            {
                //Create a buffer
                char buff[4096];
                ZeroMemory(buff, 4096);

                //Accept a new message. Send this message to other clients, NOT listening socket.
                int bytesInput = recv(sock, buf, 4096, 0);

                if (bytesInput <= 0)
                {
                    // This message is null. Drop the client. 
                    closesocket(sock);
                    FD_CLR(sock, &master);
                }
                else
                {
                    //Send message to other clients, and not listening socket. 

                    for (int i = 0; i < master.fd_count; i++)
                    {
                        //Go through the sockets in master
                        SOCKET out_sock = master.fd_array[i]; //array traversal for list of sockets
                        if (outSock != listening && outSock != sock)
                        {
                            //Create a sender ID
                            ostringstream ss;
                            ss << "SOCKET #" << sock << ":" << buf << "\n";
                            string str_out = ss.str(); 

                            //Send the message with required params
                            send(outSock, str_out.c_str(), str_out.size() + 1, 0);
                        }
                    }
                }
            }
        }
    }
}
