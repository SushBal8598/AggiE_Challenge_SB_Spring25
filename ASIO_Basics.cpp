//AggiE Challenge Spring 2025

/* Following tutorial: https://www.youtube.com/watch?v=2hNdkYInj4g
ASIO is completely HEADER-based. Download from website, either Boost or standalone.
*/

//Connect to Internet and extract data with ASIO. 

//Required header to define ASIO as standalone. 
#define ASIO_STANDALONE

//Define the Windows WINSOCK utilizations if working on Windows. 

#include <iostream> 
#include <asio.hpp> //Also used for general IO. 
#include <asio/ts/buffer.hpp> //Memory movement
#include <asio/ts/internet.hpp> //Network communication
#include <vector>

//Create a vector of chracters to handle asynchronous requests. Arbitrarily large capacity. 
std::vector<char> secondBuffer(20 * 1024);

//Create a data reading function to work with ASIO
void GetData(asio::ip::tcp::socket& socket)
{
    //Asynchronously "read some"
    socket.async_read_some(asio::buffer(secondBuffer.data(), secondBuffer.size())), //add lambda function
        [&](std::error_code ec, std::size_t length)
        {
            if (!ec) //if no errors, warnings
            {
                std::cout << "Read " << length << " bytes of data." << std::endl;

                for (int i = 0; i < length; i++)
                {
                    std::cout << secondBuffer[i]; //iterate through character array of data
                }

                //Runs into a similar issue: program will terminate because the async method is called immediately.
            }
        }
}

int main()
{
    //Create 12 errors tracked by ASIO
    asio::error_code errors;

    //Provide ASIO space to perform by creating context object. 
    asio::io_context asio_context;

    //Keep the context occupied to avoid preemptive termination. 
    asio::io_context::work work_idle(asio_context);

    //Running the above in a separate thread
    std::thread context_thread = std::thread([&]() {asio_context.run();});

    /* Connection address, called an endpoint
    TCP style means IP address must be used. 
    Make an IP with make_address and handle exceptions with the error_code object created earlier. */
    asio::ip::tcp::endpoint endpoint(asio::ip::make_address("93.184.216.34", errors), 80); //basic HTTP sites

    //ASIO TCP socket
    asio::ip::tcp::socket my_socket(asio_context); //Takes in ASIO instance as parameter

    //Bind the socket to connection and handle errors appropriately. (Same approach as used earlier)
    my_socket.connect(endpoint, errors);

    //Error handling: Was an error picked up?
    if (!ec)
    {
        std::cout << "Connection successful." << std::endl;
    }
    else //Error handling works here, so grab the message
    {
        std::cout << "Received error: " << errors.message() << std::endl;
    }

    //Do work with open socket 
    if (my_socket.is_open())
    {

        GetData(socket); //Look for data IMMEDIATELY.

        std::this_thread::sleep_for(2000ms); //indefinite wait

        //Send a verbose text request.
        std::string StringRequest = 
            "GET /index.html HTTP/1.1\r\n"
            "Host: example.com\r\n"
            "Connection: close\r\n\r\n"

        //Return the data with writesome(). Sending a buffer object, which is an array of bytes. 
        my_socket.write_some(asio::buffer(StringRequest.data(), StringRequest.size()), errors); //This approach is similar to original TCP implementation buffer

        //Investigate size of return in bytes. Need to wait, otherwise bytes returned will always be 0. 
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(200ms); //Sleep for 0.2 seconds to allow for time to receive. Not practical. 

        //Better alternative: use built-in ASIO wait
        my_socket.wait(my_socket.wait_read);

        //Returns the number of bytes available
        size_t bytes = my_socket.available(); 
        std::cout << "Number of bytes available is: " << bytes << std::endl;

        //Read bytes if nuber is greater than zero
        if (bytes > 0)
        {
            std::vector<char> sendBuffer(bytes);
            my_socket.read_some(asio::buffer(sendBuffer.data(), sendBuffer.size()), errors);

            //Iterate through the message and print the character array:
            for (auto c : sendBuffer)
            {
                std::cout << c;
            }
        }
    }


    system("pause"); //break code
    return 0;
}
