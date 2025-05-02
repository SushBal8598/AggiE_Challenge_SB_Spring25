//AggiE Challenge Spring 2025

//Logical boundary: some connections may be too frequent, so reject them after reading the header. 
//Includes: previous header files coded in previous

//Old namespace: common, net...

//Test server functionality with PuTTY (remote connection to server address)

//Simple server call will continuously run server.Update() as if it were a game.

namespace common
{
    namespace net
    {
        //Create template for server
        template<typename T> 
        class server_int //this is how the messages between clients will interact with the server
        {
            public:
                //Default constructor with port
                server_int(uint16_t port)
                {
                    //Initialize socket acceptor
                    socket_acceptor(context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port));
    
                }

                //Polymorphic destructor
                virtual ~server_interface()
                {
                    //Call stop function to end all instances. 
                    Stop();

                }

                //Helper function: Start the server. Has it been started?
                bool Start()
                {
                    //Items in the on-start method should be containerized in try / except to catch ASIO exceptions (error code)
                    try {
                        //Keep ASIO busy for client connection. Prevents ASIO from giving up too early. 
                        WaitConnection();

                        //Start context in a thread
                        context_thread = std::thread([this]() {context.run();}); //utilization of lambda function in a thread

                    }
                    catch (std::exception& e)
                    {
                        std::cout << "Received an error: " << e.what() << std::endl;
                    }

                    //Flag to indicate that server is running as expected. Too much can cause performance degradation.
                    std::cout << " Successfully started " << std::endl; 
                    return true;
                }

                void Stop() //complementary
                {
                    //Stop ASIO context
                    context.stop();

                    //The thread must be joined to main thread in order for it to stop. 
                    if (context_thread.joinable())
                    {
                        context_thread.join();
                    }

                    //Flag for stoppage
                    std::cout << " Successfully stopped " << std::endl;
                }

                void Update(size_t MaxMessages = -1, bool Wait = false) //Unsigned integer: the -1 actually corresponds to maximium number of messages.
                {
                    //Make sure some of the messages are actually sent. Prevents backlog. 
                    //Add a boolean flag to make the server sleep for the most part.

                    if (Wait) {
                        messages_incoming.wait(); //halt the incoming queue's operation
                    }

                    size_t messageCount = 0; 
                    while (messageCount < MaxMessages && !messages_incoming.empty()) //While the number of messages < max, and the incoming queue isn't empty
                    {
                        //Pop the message off of queue
                        auto msg = messages_incoming.pop_front();

                        ReceiveMessage(msg.remote, msg.msg); //Helper function to handle the message after it has been received. Takes in address, message.

                    }
                }

                //Create a blocking function to suspend the server
                void wait()
                {
                    //Use a mutex and another DEQ. 
                    std::mutex muxQueue;
                    std::deque<T> doubleEndedQueue;

                    //Use a conditional variable for locking
                    std::condition_variable queueBlock;

                    //Mutex for conditional variable
                    std::mutex blockingMux;

                    //Check if the queue is empty
                    while (empty())
                    {
                        std::unique_lock<std::mutex> u1(blockingMux);
                        //Tie CV variable to the mutex
                        queueBlock.wait(u1); //Even in the event of a spurious wakeup (Windows), queue will be empty, and server will go back to sleep
                    }
                }

                //Necessary to keep ASIO busy while waiting for server connection. Prevents immediate termination. 
                void WaitConnection()
                {
                    //Accept the client connection
                    socket_acceptor.async_accept([this] (std::error_code errors, asio::ip::tcp::socket socket)
                    {
                        //Check for errors against the error code
                        if (!errors)
                        {
                            //Remote_endpoint produces the IP of the socket
                            std::cout << "Connected to: " << socket.remote_endpoint(); 

                            //Create a new connection object
                            std::shared_ptr<connection<T>> new_connection = std::make_shared<connection<T>>(connection<T>::owner::server, //owned by server
                            context, std::move(socket), messages_incoming); //Queue is shared because it is only referenced once throughout. 

                            //Deny the client connection
                            if (ClientConnect(new_connection))
                            {
                                //Accept the connection and add to the queue of connections. 
                                DEQ_Connections.push_back(std::move(new_connection)); //Why std::move? Prepares the object's data to be transferred.

                                //Assign an identifier
                                DEQ_Connections.back() -> ClientConnect(this, ID_Client++); //The client's ID is a sequential change to previous ID. 

                                //New: add pointer to the client object

                                //Retrieve the ID from the back of the queue
                                std::cout << "Connection approved for: " << DEQ_Connections.back()->GetID();

                            }
                            else
                            {
                                //Deny the connection
                                std::cout << "Connection was denied." << std::endl; 

                                //Pointer will go out of scope (smart pointer)
                            }
                        }
                        else
                        {
                            std::cout << "Received an error: " << errors.message();
                        }

                        //Wait for client connection to keep busy
                        WaitConnect();
                    }); //Asynchronous accept will require a lambda function to keep busy
                }

                //Send message to client. Takes in client param, message param.
                void SendClient(std::shared_ptr<connection<T>> client, const message<T>& msg) //Const function: won't modify contents of message.
                {
                    //Ensure that there is a valid connection: shared pointer is not out of scope, and client is still connected (bool)
                    if (client && client->ClientConnect())
                    {
                        client->SendClient(); //Send message, since connection is valid
                    }
                    else
                    {
                        //TCP won't allow you to explicitly know when a client has disconnected from server. Assume disconnection:
                        ClientDisconnect(client);

                        //Reset to delete
                        client.reset();
                        //Erase client from queue by moving data
                        DEQ_Connections.erase(std::remove(DEQ_Connections.begin(), DEQ_Connections.end(), client), DEQ_Connections.end()); //Removal requires use of iterators


                    }
                }

                //Message all clients, with the optional functionality to avoid one client pointer. 
                void MessageAll(const message<T>& msg, std::shared_ptr<connection<T>> ignoreClient = nullptr) //Default is no client. 
                {

                    //Second implementation of message_all
                    common::net::message<CustomMessages> msg;
                    msg.header.ID = CustomMessages::MessageAll; //Assign a custom message ID to this message for transmission
                    Send(msg); //Send the message using Send utility function

                    //Note that only the HEADER of the message (containing ID) is sent here.

                    //Flag to indicate that an invalid client exists (can't message)
                    bool InvalidExists = false; //Update when an invalid client is found

                    //Iterate through all clients in queue of active clients
                    for (auto& client : DEQ_Connections) //Auto will assume type of the iterators in the double-ended queue
                    {
                        if (client && client->ClientConnect())
                        {
                            client->Send(msg); //Send a message if the client connection exists
                        }
                        else
                        {
                            //Disconnect the client for cleanup. 
                            ClientDisconnect(client);
                            client.reset();
                            //An invalid client exists. 
                            InvalidExists = true;
                        }
                    }

                    if (InvalidExists)
                    {
                        //Erase the invalid connection
                        DEQ_Connections.erase(std::remove(DEQ_Connections.begin(), DEQ_Connections.end(), nullptr));
                    }
                }

            protected: //Override functions in base class
                //When a client connects to server, send a notification
                virtual bool ClientConnect(std::shared_ptr<connection<T>> client)
                {
                    //Create a message to keep the client busy
                    common::net::message<CustomMessages> msg;
                    msg.header.id = CustomMessages::ServerAccept; //ID sourced from custom accept condition
                    client->Send(msg); //Have the client utilize member send function.

                    return true; //Notification that client has successfully or unsuccessfully connected
                } 

                //Partner function: when client disconnects
                virtual void ClientDisconnect(std::shared_ptr<connection<T>> client)
                {
                    //Remove from the "game world" -- this is something useful to revisit when working on ComputeServer logic. 
                    // (host vs. joinable client dyanmic)

                    //State that the client thas disconnected
                    std::cout << "Removed client: " << client->GetID() << std::endl;

                }

                //Message arrival and response. 
                virtual void ReceiveMessage(std::shared_ptr<connection<T>> client, message<T>& msg)
                {
                    //Define reaction to a ping
                    switch (msg.header.ID)
                    {
                        case (CustomMessages::ServerPing):
                        {
                            std::cout << "Message from: " << client->GetID() << " was a ping" << std::endl;

                            //Send the message from client
                            client->Send(msg); //Returns the message back to client.
                        }

                        //Client message (sent to all)
                        case (CustomMessages::ServerMessage):
                        {
                            uint32_t client_ID; //This is derived from the header data
                            msg >> client_ID;
                            std::cout << "Message sent from " << client_ID << std::endl;
                        }

                        //Server accepted a client and responded to ping; successful bounce. 
                        case (CustomMessages::ServerAccept):
                        {
                            std::cout << "Server has accepted connection. " << std::endl;
                        }
                    }

                } 

                //Create and maintain a double-ended queue of messages
                thread_queue<owned_message<T>> messages_incoming; //custom "owned messages" template developed earlier

                //With ASIO, the order of initialization corresponds with order of declaration -- so, declaration order is important
                asio::io_context context; //Context is created first
                std::thread context_thread; //Thread is created second such that context can be joined to thread

                //ASIO acceptor: Requires ASIO context to be created first. Creates an internal socket. 
                asio::ip::tcp::acceptor socket_acceptor;

                //Client IDs: each client will have a unique ID that differentiates it from another client.
                uint32_t ID_Client = 10000; //Random value for initialization. Safest approach.
                //Alternative approach would be to use IP and port as identifier, but that shouldn't be sent ot other clients connected.
        
                 //Double-ended queue for accepted connections
                 std::deque<std::shared_ptr<connection<T>>> DEQ_Connections;
        };
    }
}
