//AggiE Challenge Spring 2025

//Requires CPP 11 or higher. (This isn't a problem.)

/*Every application is at LEAST single-threaded (main is a single thread)
- Created threads must be threaded into main().
- Can create a thread with pointers. 
- Can create a thread with functor. (Function object, created by overloading operator)
- Can create a thread with member functions. 
- Can create a thread with static member functions.

What is a PROCESS?
- Instance of program execution
- Threads are lighter-weight than standard processes. 

Reference: https://www.youtube.com/watch?v=TPVH_coGAQs
*/

//Example: Finding sum of even numbers between 0 and some number i
#include <iostream> 
#include <thread> //functionality to create, manage thread usage
#include <chrono> //timing types, functions
#include <algorithm> //algorithmic toolkit

using namespace std;
using namespace std::chrono;
typedef unsigned long long ull; //create a custom ull type for use -- this is just a recombination

ull OddSum = 0; 
ull EvenSum = 0;

void findEven(ull start, ull end)
{
    //This part is standard counting procedure. 
    for (ull i = start; i < end; ++i)
    {
        if ((i & 1) == 0) //perform a bitwise comparison between numbers
        {
            EvenSum += i;
        }
    }
}

void findOdd(ull start, ull end) //analog
{
    //This part is standard counting procedure. 
    for (ull i = start; i < end; ++i)
    {

        if ((i & 1) == 1) //perform a bitwise comparison between numbers
        {
            OddSum += i;
        }
    }
}

int main(void)
{
    //Set up counters and run program.
    ull start = 0;
    ull end = 1900000000;

    auto start_time = high_resolution_clock::now(); //get the instantaneous time 

    //Introduce threading
    std::thread t1(findEven, start, end); //creates a callable object
    std::thread t2(findOdd, start, end); //using function pointers

    //Join threads to main program execution
    t1.join();

    //Runs in parallel with t1. Time is essentially halved.
    t2.join();

    // findOdd(start, end);
    // findEven(start, end);

    auto stop_time = high_resolution_clock::now(); //get the instantaneous time 
    auto duration = duration_cast<microseconds>(stop_time - start_time); //Calculate time in microseconds

    cout << "The odd sum is: " << OddSum << endl;
    cout << "The even sum is: " << EvenSum << endl;
    cout << "Program duration: " << duration.count() / 1000000 << endl;
    return 0;

    //Visualized as a SINGLE THREAD: the program runs linearly in main, referencing functions declared above.
    //Compile with -pthread flag in terminal
}
