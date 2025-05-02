//AggiE Challenge Spring 2025

/* Conditional variables: One thread prepares data, 
while the other thread waits for data to be produced before consuming.

Standard protocol for client-server relations; revolves around sending MESSAGE.

Common approach: utilize shared memory to set a flag that the reader thread can access and modify. 

Critical section revolves around this CONDITIONAL. 
*/

//Producer and consumer functions

#include <mutex>
#include <numeric>
#include <thread>
#include <vector>
#include <iostream>

std::mutex g_mutex;
unsigned int counter;
bool data_ready = false;

void producer()
{
    for (int i = 0; i < 100; i++)
    {
        std::unique_lock<std::mutex> u1(g_mutex);
    }

    counter = rand(); //produce a random message
    data_ready = false; //conditional

    u1.unlock();

    std::this_thread::sleep_for(std::chrono::milliseconds(20)); //after the flag has been set, sleep for a desired amt of time
    
    u1.lock();
}

//Consumer function

void consumer()
{
    int data; 
    for (int i = 0; i < 100; i++)
    {
        std::unique_lock<std::mutex> u1(g_mutex);

        while (data_ready == false)
        {
            u1.unlock();

            //lock data after check so producer can access -- bad because it occupies CPU
            u1.lock();
        }
    }
    data_ready = false;
}

//Better approach 

void consumer_better()
{
    int data; 
    for (int i = 0; i < 100; i++)
    {
        std::unique_lock<std::mutex> u1(g_mutex);

        while (data_ready == false)
        {
            u1.unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(20));

            //lock data after check so producer can access -- bad because it occupies CPU
            u1.lock();
        }
    }
    data = data;
    data_ready = false;
}

/* Implement a conditional variable instead: Remains in shared memory, in critical section
Thread produces data, sends flag, but also sends message through conditional variable.
*/

//1) create variables
std::mutex g_mutex; 
std::condition_variable g_cv;
bool g_ready = false;
int g_data = 0;

//2) Producer, consumer with CV
void producer_new()
{
    while (true)
    {
        std::unique_lock<std::mutex> u1(g_mutex);

        //Data is in shared memory
        g_data = rand(); //random assignment for data -- in network, this would be byte of data
        g_ready = true;
        u1.unlock(); 
        g_cv.notify_one(); //notify single thread of completion
        u1.lock(); //reciprocal locking
        g_cv.wait(u1, []() {return g_ready = false;});
    }
}

void consumer_new()
{
    int data = 0; 
    while (true)
    {
        //"Pragmatism": lock, unlock called depending on state of lock
        std::unique_lock<std::mutex> u1(g_mutex);

        //Somewhat of an "async" method: call unlock if blocked, and lock if unblocked.
        g_cv.wait(u1, []() {return g_ready;}); //wait until the predicate becomes true

        data = g_data; //retrieve data from sender
        std::cout << "The data is: " << data << std::endl;

        g_ready = false;

        u1.unlock();

        g_cv.notify_one();

        u1.lock();

        //do something with the data
    }
}

/*Conditional review

1) Sender operates on mutex applied to shared mutex
2) Modify data while the lock is held
3) Send to one thread, or send to all (broadcasting) -- can be unlocked state.

4) Receiver uses unique_lock on mutex of focus
5) Async wait, and mutex is locked or unlocked appropriately.

TO-DO: 
1) Single thread notification
2) Producer, consumer anti-infinite-loop
- Wait for a CV flag. 

*/

/* Critical section: final

Lock is atomic, but creates undefined behavior if thread owns mutex
- Can't lock the same mutex on a thread twice.

SEQUENTIAL CONSISTENCY: Synchronization of locks for critical sections.

Also important to use a PREDICATE to ensure that wait does not end spuriously

Notification from sender is NOT LOST if consumer checks for notification, first, but does not see.
This is because of predicates + sequential consistency. 

*/
