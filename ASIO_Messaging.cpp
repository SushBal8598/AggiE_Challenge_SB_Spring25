//AggiE Challenge Spring 2025

/* Utilizing a MESSAGES structure with ASIO: 
Message Header with ID (Enum Class), Size of Message; and Message Body with info. 
Message<T>: using templates for polymorphism. */

//Tutorial, part two. Custom data types for sending and receiving objects.

//Common includes, and ASIO
#pragma once

#include <memory>
#include <thread>
#include <mutex>
#include <deque> //double-ended queue for deletions
#include <optional>
#include <vector> 
#include <iostream>
#include <algorithm>
#include <chrono>
#include <cstdint>

//ASIO includes
#define ASIO_STANDALONE
#include <asio.hpp> //Also used for general IO. 
#include <asio/ts/buffer.hpp> //Memory movement
#include <asio/ts/internet.hpp> //Network communication

//Creating the "net" object to send data
namespace common
{
    namespace net
    {
        template <typename T> //initialize the template class
        struct message_header //this is the prefix for all messages sent over server
        {
            T id{};
            uint32_t size = 0; //Bytes ordering changes based on computer architecture. Conversion should be implemented as necessary. 

            //Problem: System is reading garbage and is waiting to create a buffer with large enough size for that garbage
            //Potential solution: Ban connections that don't immediately send welcome message.

            //Write validation: send a "puzzle" to client to determine legitimacy
            //Read the response in Read Validation

            //Can't expect the client to re-send a message like "hello" -- someone else will figure this out and manipulate it
            /* Alternative approach: functionally chnage a piece of data and shuffle between client and server
            This function needs to be kept hidden, like a code. Complexity of function should not be compromised.*/
        }

        template <typename T> //Message template (content of the connection)
        struct message
        {
            message_header<T> header{};
            std::vector<uint8_t> body; //Body is a character array, or filled in with vectors

            //Member function to return size of the message:
            size_t size() const
            {
                return sizeof(message_header<T>) + body.size(); //The total message includes the header + the body
            }

            //Overriding << to be able to print in ostream. Uses friend keyword, as standard
            friend std::ostream& operator << (std::ostream& os, const message<T>& msg) //include the message in output stream
            {
                os << "ID: " << int(msg.header.id) << "; Size: " << msg.header.size;
                return os;
            }

            //How can data be ADDED to the buffer object?
            template<typename DataType> 
            friend message<T>& operator << (message<T>& msg, const DataType& data) //standard overloading
            {
                //Pre-serialization. To be serialized, data type must be a standard layout. 
                static_assert(std::is_standard_layout<DataType>::value, "Data cannot be serialized.");

                //Save size of the body in a variable; check later for insertion. 
                size_t i = msg.body.size(); //This will increase with message size. 

                msg.body.resize(msg.body.size() + sizeof(DataType)); //similar to an append

                //Copy the data
                std::memcpy(msg.body.data() + i, &data, sizeof(DataType));

                //Updated message size is the size of the copy. 
                msg.header.size = msg.size();

                return msg; 
            }

            //The above implementation serializes data for output. Now, overloading extraction operator >>:
            template<typename DataType>
            friend message<T>& operator >> (message<T>& msg, DataType& data)
            {
                //Mirror serializability check
                static_assert(std::is_standard_layout<DataType>::value, "Data cannot be serialized.");

                size_t i = msg.body.size() - sizeof(DataType); //not starting at the "beginning," so need to calculate the "loss"

                msg.body.resize(i); //resize to the size of the extraction

                /* No reallocation necessary when the vector is treated similar to a stack.
                Alternate approach: use an iteractor, but need to store the iterator somewhere when combining multiple chains. */
                return msg;

            } 
        }

        //Pointer connection does exist. Define it here. 
        template <typename T>
        class connection;

        //Create a new "owned message" struct that holds pointer to message object
        template <typename T>
        struct owned_message
        {
            std::shared_ptr<connection<T>> remote = nullptr; //Manual assignment of lifetime ownership to multiple objects
            message<T> message;

            //Overloaded output
            friend std::ostream& operator<<(std::ostream& os, const owned_message<T>& msg)
            {
                os << msg.msg; //Output the contents of message param of message object to the output stream
                return os;
            }
        }
    }
}

//Server connection building with types established above. First, creating enum class to store messages. 
enum class CustomMessages : uint32_t //Each message is 4 bytes in length
{
    MessageOne,
    MessageTwo
};

int main(void)
{
    //Create the custom message type instance
    common::net::message<CustomMessages> my_message;

    //Example: Set the ID to MessageOne
    my_message.header.id = CustomMessages::MessageOne;

    //How does message sending work here?
    int a = 1; 
    bool b = false;
    float c = 3894.202;

    struct {
        float g;
        float z;
    } d[5]; //create an array d of size 5 to accompany struct

    //Example command:
    msg << a << b << c << d;
    msg >> d >> c >> b >> a; //read messages back out

    //On Windows, can use the method async get key states. Example:
    bool old_keys[3] = {false, false, false};

    key[0] = GetAsyncKeyState('1') & 0x8000; //Was the key pressed at the time of execution? Used in winuser.h for Windows programming on Unix


    //Windows only: Flag to monitor quit state
    bool Quit = false;
    while (!Quit)
    {
        //Is the client still connected to the server?
        if (c.IsConnected())
        {
            //Still connected. Is the incoming queue empty?
            if (!c.Incoming().empty())
            {
                //Get the new message
                auto msg = c.Incoming().pop_front().msg; //Get the message data from the first item in the queue

                //Switch statement around IDs of messages (each has custom)
                switch (msg.header.ID)
                {
                    case CustomMessages::ServerPing:
                    {
                        //Send message back to user
                        std::chrono::system_clock::time_point time_right_now = std::chrono::system_clock:now();

                        //When was the message sent? (Time THEN)
                        std::chrono::system_clock::time_point timeThen;

                        msg>>timeThen;

                        //Calculate the "round trip"
                        std::cout << "Ping is: " << std::chrono::duration<double>(time_right_now - timeThen).count() << std::endl;
                    }

                    //In the event of messaging all:
                    case (CustomMessages::MessageAll):
                    {
                        std::cout << "Message send all: " << client->GetID() << std::endl; //State that the specific ID sent message to all
                        common::net::message<CustomMessages> msg;
                        msg.header.id = CustomMessages::ServerMessage; //Assign ID corresponding to task assignment

                        //Write to the body of the message using the overloaded insertion operator for ostream. 

                        msg << client->GetID(); 

                        //Message all connected clients
                        MessageAll(msg, client); //The second parameter is typically nullptr; but will ignore the client pointer instead
                    }
                }
            }
        }
        else
        {
            //Client no longer connected. 
            std::cout << "Shutting down server" << std::endl;
            Quit = true;
        }
    }

}

//Create a connection type
namespace common
{
    namespace net
    {
        //Declare pointer server interface beforehand for use later. 
        template<typename T>
        class server_interface;
        
        template <typename T> 
        class connection : public std::enable_shared_from_this<connecton<T>> //Allows an object with a shared pointer to create instances tying to that ptr. 
        {
            public: 
                //Make sure that a connection doesn't go out of scope by creating "ownership"
                enum class owner
                {
                    server,
                    client //client-server relationship stored as an enum instance
                };

                connection(owner parent, asio::io_context& context, asio::ip::tcp::socket socket, tsqueue<owned_message<T>>& queueIn) 
                : context(context), socket(std::move(socket)), incomingMessages(queueIn)
                {
                    //Take in an owner type, plus the server's context, and the socket, and the queue to handle incoming requests. This is the PARENT.
                    ownerType = parent;

                    //If the owner is a server, owner is responsible for generating the handshake "hash"
                    if (ownerType == owner::server)
                    {
                        //Use chrono to create sudo-random keys
                        handOut = uint64_t(std::chrono::system_clock::now().time_since_epoch().count()); //Time between "this" instance and clock "epoch"

                        //Scramble data
                        handCheck = scrambler(handOut); //apply custom encryption to the output
                    }
                    else
                    {
                        //Client does not need to create a custom key for validation
                        handIn = 0;
                        handOut = 0;
                    }


                } //Default constructor
                
                
                virtual ~connection() {} //polymorphic destructor

                //Utility functions
                bool ConnectToServer(); //Called by clients. 
                bool Disconnect(); //Called by clients and hosts.
                bool IsConnected() const //Called on destruction.
                {
                    return socket.is_open(); //Is the socket open? If not, then the socket is connected
                }; 

                //Windows only: ping the server when a specific key is pressed. (Example)
                void PingServer()
                {
                    common::net::message<CustomMessages> msg; //Create a custom message instance
                    msg.header.ID = CustomMessages::ServerPing; //ID of the message is equal to server ping...sending a time block

                    //Bad practice, but for example
                    std::chrono::system_clock::time_point time_right_now = std::chrono::system_clock:now(); //Now() function call gets the time "now" at execution
                    //Determines time for message to be sent, received, sent back (ping); risky because of uncertainty surrounding system clock implementation on machine.

                    //Add the message of send to message
                    msg << time_right_now;
                    Send(msg);
                }

                void Send(const message<T>& msg)
                {  
                    //ASIO context is waiting for work (either accepts new client connections or reads incoming)
                    //ASIO post: sends job to context
                    asio::post(context, 
                        [this, msg]()
                        {
                            //Can't just write the header because the message has not yet been invoked in queue. 
                            //What happens if a message is already being written? Need to add a bool to check. 
                            bool isWritingMessage = !messages_out.empty();

                            messages_out.push_back(msg); //Write message to queue

                            //Only write header of message when the queue is empty
                            if (isWritingMessage)
                            {
                                WriteHeader(); //Begin writing messages
                            }  
                        }
                    );

                } //Send message, return whether that was completed

                //Getter: retrieve the connection's unique ID
                uint32_t getID() const
                {
                    return ID; 
                }

                //Write validation code
                void WriteValidation()
                {
                    //Create asynchronous method to write to handshake out
                    asio::async_write(socket, asio::buffer(&handOut, sizeof(uint64_t)),
                    [this] (std::erorr_code errors, std::size_t length)
                    {
                        //If everything is correct and the device is client, wait for data to be sent
                        if (!errors)
                        {
                            if (ownerType == owner::client)
                            {
                                ReadHeader(); //read the incoming message header
                            }
                        }
                        else
                        {
                            //Close the socket because of errors
                            socket.close();
                        }
                    }
                    
                    )
                }

                //Read validation function
                void ReadValidation(common::net::server_interface<T>* server = nullptr)
                {
                    //Pointer object is used to inform server, or derived class from server, that client has connected
                    asio::async_read(socket, asio::buffer(&handIn, sizeof(uint64_t)),
                    [this](std::error_code errors, std::size_t length)
                    {
                        if (!errors)
                        {
                            if (ownerType == owner::server) //If the user type is server, then wait for messages
                            {
                                //If server, then data in is the response from client.
                                if (handIn == handCheck) //if the input validation equals the base
                                {
                                    std::cout << "Client was validated " << std::endl;
                                    server->OnClientValidated(this->shared_from_this()); //Client validation, entry into server system

                                    //Prime the system to read data by waiting for header. 
                                    ReadHeader();
                                }
                                else
                                {
                                    //Close the connection because the puzzle was not completed
                                    socket.close();
                                }
                            }
                            else
                            {
                                //Client relationship, so scramble the handshake check
                                handOut = scrambler(handIn);

                                //Write the result
                                WriteValidation();
                            }
                        }
                        else
                        {
                            //Errors occured, so terminate
                            socket.close();
                        }
                    });
                }

                //Connect client to server
                void connect_to_client(uint32_t ID = 0) //default ID parameter is 0 (first established)
                {
                    //If owner is the server, proceed with the connection attempt. 
                    if (ownerType == owner::server)
                    {
                        //If the socket is open, connect
                        if (socket.is_open())
                        {
                            ID = uid; //UID is the unique identifier attached to the object by the computer
                            
                            //Read the message after server has connected to client
                            ReadHeader(); //This always occurs here, after the socket has been connected to. 

                            //New approach: validation
                            ReadValidation(server);
                        }
                    }
                }

                //Create an encryption method for transfer of messages to incoming clients
                uint64_t scrambler(uint64_t input)
                {
                    //Create the output
                    uint64_t output = input ^ 0xDEADBEEFCODECAFE; //Bitwise XOR creates a binary string
                    output = (out & 0xF0F0F0F0F0F0F0) >> 4 | (output & 0F0F0F0F0F0F0F) << 4;
                    return output ^ 0xC0DEFACE12345678; //This approach uses XOR to shuffle bits around, creating new strings 
                }

            protected:
                asio::ip::tcp::socket socket; //client connecton's socket

                //ASIO power, context
                asio::io_context& asio_context;

                //Queue of messages to be sent
                thread_queue<message<T>> outgoingMessages;

                //Incoming message queue
                thread_queue<message<T>>& incomingMessages; //Reference because it is owned by the client

                //Owner is now the server
                owner ownerType = owner::server;

                uint32_t ID = 0; //Connection ID, returned by getter function

                //"Handshake functions" : 
                uint64_t handOut = 0; //Client leaves
                uint64_t handIn = 0; //Client connection 
                uint64_t handCheck = 0; //Client is valid or not

                virtual void OnClientValidated(std::shared_ptr<connection<T>> client)
                {
                    //After a client has been validated, it must be added to system; output the proper response 
                    //Unlike OnConnect, this requires the connection to have been VALIDATED; so not just "anything" can connect

                }

            private:

                //Read message: PuTTY has no way to actually take in messages sent by one user to another. 
                void ReadHeader() {
                    //Read the header of the message. Message is composed of multiple parts. 
                    asio::async_read(socket, asio::buffer(&msgTemporaryIn.header, sizeof(message_header<T>))),
                        [this] (std::error_code errors, std::size_t lenfth)
                        {
                            if (!ec)
                            {
                                //No errors. Read the message if size is not zero
                                if (msgTemporaryIn.header.size > 0)
                                {
                                    //Allocate enough space to the message buffer to store the message as a character array
                                    msgTemporaryIn.body.resize(msgTemporaryIn.header.size);

                                    //Read the message after character array has been created
                                    ReadBody();
                                }
                                else //message size is 0
                                {
                                    //Add message to the queue since it does not contain any information.
                                    AddToQueue();
                                }
                            }
                            else
                            {
                                //Output the reason why the connection was a failure, and close. 
                                socket.close();
                                std::cout << "Error was: " << errors.message();
                            }
                        }
                     //Creates a buffer with required space to intake message. 
                    //Lambda function for busy-work. 

                }

                void ReadBody() //Read the body of the message, the portion after the header.
                {
                    //Async read: keep the context busy while waiting for message to read
                    asio::async_read(socket, asio::buffer(msgTemporaryIn.body.data(), msgTemporaryIn.body.size())),
                        [this] (std::error_code errors, std::size_t length)
                        {
                            if (!errors) //no errors; proceed with attempt to read message
                            {
                                AddToQueue(); //Complete messages will be added to queue to be processed.
                            }
                            else
                            {
                                //Message is not complete, so socket should be closed
                                socket.close();
                            }
                        }

                }

                void WriteHeader() //Write a header for the message instance.
                {
                    //Async Write function to keep busy
                    asio::async_write(socket, asio::buffer(&messages_out.front().header(), sizeof(message_header<T>)), //Construct intermediate buffer large enough, and draw from outgoing queue
                        [this](std::error_code errors, std::size_t length)
                        {
                            if (!errors)
                            {
                                //If the front message's body is non-zero, send
                                if (messages_out.front.body.size() > 0)
                                {
                                    WriteBody(); //Write body of message to client
                                }
                                else
                                {
                                    //The message has been looked at and is not sent. Remove from queue
                                    messages_out.pop_front();

                                    //No more messages in queue?
                                    if (!messages_out.empty())
                                    {
                                        WriteHeader(); //Write the header if the queue is not empty
                                    }
                                }
                            }
                        }
    
                    
                    )
                }

                void WriteBody() //Write a body for the message instance. 
                {
                    asio::async_write(socket, asio::buffer(messages_out.front().body.data(), messages_out.front().body.size()),
                        [this] (std::error_code errors, std::size_t length)
                        {
                            //Same asynchronous approach as used above, with lambda function
                            if (!errors)
                            {
                                messages_out.pop_front(); //Remove message from queue, indicating that it has been sent
                                if (!messages_out.empty())
                                {
                                    WriteHeader();
                                }
                            }
                            else
                            {
                                socket.close(); //Could not write message because error was picked up by code. 
                            }
                        }
                    )

                }

                void AddToQueue() //Add message to queue to be processed. Requested by both read functions.
                {
                    //If the user is server, needs to OWN the message. 
                    if (ownerType == owner::server)
                    {
                        //Push back into incoming messages queue
                        incomingMessages.push_back({this -> shared_from_this(), msgTemporaryIn}); //Shared from this will safely create member object from the class itself
                        //this -> helps extract the address to use in the constructor
                    }
                    else
                    {
                        incomingMessages.push_back({nullptr, msgTemporaryIn}); //not a message owned by server, so push back a nullptr and temp buffer
                    }

                    //Read the header (called after reading message)
                    ReadHeader(); //More busy work to keep occupied. Reads another header. 
                }
        }

        //Client interface 
        template <typename T>
        class client_interface
        {
            public:
                bool Connect(const std::string& host, const uint16_t port)
                {
                    //Connect to the address and port using ASIO.
                    try
                    {
                        //Domain names may point to IP addresses that change over time. Need to ensure that the system is robust in connectivity. Connect to a domain name?

                        //ASIO resolver #2: Hostname is turned into a connectable address. 
                        asio::ip::tcp::resolver resolver(context);

                        //Create new endpoints with pasted connection
                        asio::ip::tcp::resolver::results_type endpoints = resolver.resolve(host, std::to_string(port));

                        //If the URL cannot be resolved, exception will be caught by handler. 

                        //Need to create connection first. 
                        connection = std::make_unique<connection<T>>(
                            connection<T>::owner::client, 
                            context,
                            asio::ip::tcp::socket(context),
                            incoming_messages
                        ); //more dynamic memory allocation. Make a connection with a socket and queue for messages. 

                        //Using ASIO's resolver, convert a website address into a "connectable"
                        asio::ip::tcp::resolver resolver(context);
                        endpoints = resolver.resolve(host, std::to_string(port)); //New endpoints for socket

                        //If the endpoints were successfully generated, connect. 
                        connection->ConnectToServer(endpoints);

                        //New thread for ASIO context to work with
                        context_thread = std::thread([this]() {context.run();});
                        
                    }
                    catch (std::exception &e)
                    {
                        std::cerr << "Exception caught: " << e.what() << std::endl;
                        return false;
                    }

                    return true;
                }

                void Connect2(const asio::ip::tcp::resolver::results_type& endpoints) //Connect with endpoints generated by resolve
                {
                    //Allow clients only to connect to the server
                    if (owner_type == owner::client)
                    {
                        //Busy work for context
                        asio::async_connect(socket, endpoints, 
                        //Lambda function
                        [this] (std::error_code errors, asio::ip::tcp::endpoint endpoint)
                        {
                            if (!errors)
                            {
                                ReadHeader(); //No errors, so clear to read the message header

                                //New approach: move towards validation
                                ReadValidation();
                            }
                        };
                        )
                    }
                }

                void Disconnect()
                {
                    //Reverse connection, disconnect. Like a destructor member function. 
                    if (IsConnected())
                    {
                        //Async way to delete connection with post
                        asio::post(context, [this]() {socket.close();};)
                        connection->Disconnect(); //Called as courtesy. 
                    }

                    //Stop context from running. 
                    context.stop();

                    if (context_thread.joinable())
                    {
                        context_thread.join(); //join to main thread
                    }

                    //Release connection object as courtesy
                    connection.release();
                }

                bool IsConnected()
                {
                    //Is the connection still open?
                    if (connection)
                    {
                        return connection->IsConnected(); //Run the member function of object
                    }
                    else
                    {
                        return false;
                    }
                }

                //Create an accessor function to get information from the incoming queue, which is private. 
                thread_queue<message<T>>& IncomingMessages()
                {
                    return incomingMessages; //Return contents of private var
                }

                //Constructor, destructor methods
                client_interface() : socket(context) {};

                virtual ~client_interface()
                {
                    Disconnect(); //Call "clear" function
                }

            protected:
                asio::io_context context; //Owned by the client. Couple with a thread:
                std::thread thread;

                asio::ip::tcp::socket socket;
                std::unique_ptr<connection<T>> connection; //Unique pointer is deleted when this client interface object goes out of scope

            private:
                thread_queue<message<T>>& incomingMessages; //owned by client

                //Temporary message buffer for incoming messages; referenced in read_async
                message<T> msgTemporaryIn;
        };
    }
}
