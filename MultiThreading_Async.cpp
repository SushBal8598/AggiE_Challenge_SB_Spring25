//AggiE Challenge Spring 2025

/*Working with:
- std::thread
- std::async for tasks (similar to C#)
- pointer to function, functors, and lambda functions
*/

//Example 1: Multithreading for large counter function
#include <iostream>
#include <thread>
#include <functional>
#include <numeric> // this will allow you to use accumulate (left fold)
using namespace std;

//Pass in function parameters by reference using std::ref()

void AccumulateRange(uint64_t &num, uint64_t start, uint64_t end)
{
    //Define standard counting function 
    uint64_t sum = 0; 
    for (uint64_t i = start; i < end; i++)
    {
        sum += i;
    }
}

//Two threads approach
int main(void)
{
    //Create the parameters to define multi-threading
    const int num_of_threads = 2;
    const int num_of_elements = 1000 * 1000 * 1000;
    const int step = num_of_elements / num_of_threads;

    std::vector<uint64_t> partial_sums(num_of_threads); // this will split the calculation into two
    
    std::thread t1(AccumulateRange, std::ref(partial_sums[0]), 0, 1000/2);

    //For the second thread, the ranges are specified by the boundaries created by the first
    std::thread t2(AccumulateRange, std::ref(partial_sums[1]), step, num_of_threads * step);

    t1.join();
    t2.join();

    uint64_t total = std::accumulate(partial_sums.begin(), partial_sums.end(), uint64_t(0)); //left fold on sums, 
    // which is a recursive operation 

    std::cout << "Total: " << total << std::endl;

}

//Vector multi-threading -- 10 threads...
int main(void)
{
    //Create the parameters to define multi-threading
    const int num_of_threads = 10;
    const int num_of_elements = 1000 * 1000 * 1000;

    //Create a vector of threads
    std::vector<std::thread> threads;
    const int step = num_of_elements / num_of_threads;

    std::vector<uint64_t> partial_sums(num_of_threads); // this will split the calculation into two

    for (u_int64_t i = 0; i < num_of_threads; i++)
    {
        //Create each thread's unique task. Follows the format: function, then params, passed as reference.
        threads.push_back(std::thread(AccumulateRange, std::ref(partial_sums[i]), i * step, (i + 1) * step));
    }

    for (std::thread &t : threads) // loop through all threads
    {
        if (t.joinable())
        {
            //if the thread hasn't been joined to main, join it
            t.join();

            //The thread will start as soon as it is created. 
        }
    }
    
    uint64_t total = std::accumulate(partial_sums.begin(), partial_sums.end(), uint64_t(0)); //left fold on sums, 
    // which is a recursive operation 

    std::cout << "Total: " << total << std::endl;

}

//Multithreading with functors (functions with overloaded operator())
class AccumulateFunctor {
    public: 
        void operator()(uint64_t start, uint64_t end)
        {
            _sum = 0; // the variable is created later
            for (auto i = start; i < end; i++)
            {
                // increment the hidden var with the sum
                _sum += i; 
            }

            std::cout << _sum << std::endl;
        }
        u_int64_t _sum; 
};

int main(void)
{
    //Functor multithreading: main
    const int num_of_threads = 10;
    const int num_of_elements = 1000 * 1000 * 1000;
    const int step = num_of_elements / num_of_threads; //these initializations are kept the same

    //Create vector of threads
    std::vector<std::thread> threads;

    //Create vector of functors (notice pointer logic for PBR)
    std::vector<AccumulateFunctor *> functors;

    //Iterate through threads
    for (int i = 0; i < num_of_threads; i++)
    {
        AccumulateFunctor *functor = new AccumulateFunctor(); //OOP class object
        threads.push_back(std::thread(std::ref(*functor), i * step, (i + 1) * step));
        functors.push_back(functor); //push back functor method
    }

    //Now, join threads by iterating through them
    for (std::thread &t : threads) //note PBR
    {
        if (t.joinable())
        {
            t.join();
        }
    }

    u_int64_t total = 0; 
    for (auto pf : functors) // the compiler will understand that pf = accumulate functor
    {
        total += pf -> _sum; //note that this private property is unique to AccumulateFunctor() class, but
        //we can access it using arrow notation. This is the benefit of auto declaration
    }

    std::cout << "total: " << total << std::endl;

    /* Main takeaways for functors: 
    - similar to pointer-to-function, but you need a VECTOR of FUNCTORS
    - can eliminate partial sum lists */
}  

/*Using lambda functions for threads. Lambda functions are definitions without identifiers -- short limited-use expressions. 
[capture](parameters) -> return_type {function_body} 
Lambda function can be passed as a parameter when creating a thread. */

//Lambda function for summation

int main(void)
{
    const int num_of_threads = 10;
    const int num_of_elements = 1000 * 1000 * 1000;
    const int step = num_of_elements / num_of_threads;

    std::vector<std::thread> threads;

    //New: create an integer array of partial sums. For lambda functions, you need to use some sort of reference array. 
    std::vector<uint64_t> partial_sums(num_of_threads);

    //Loop through, applying lambda function to each step
    for (uint64_t i = 0; i < num_of_threads; i++)
    {
        //Directly apply the lambda function to each summation, and create a thread with these passed arguments.
        threads.push_back(std::thread([i, &partial_sums, step]{
            for (uint64_t j = i * step; j < (i + 1) * step; j++)
            {
                partial_sums[i] += j;
            }
        }));
    }

    for (std::thread &t : threads)
    {
        if (t.joinable())
        {
            t.join();
        }
    }

    //for accumulation, create an ITERABLE with begin() and end()
    uint64_t total = std::accumulate(partial_sums.begin(), partial_sums.end(), uint64_t(0));
    
    //Since it's a vector, you could interate through the size of the vector and print out the element in each indice

    std::cout << "Total is: " << total << std::endl;
}

/* Creating threads with asynchronous tasks
Async allows your functions to RETURN THE VALUE at the end of the execution, as opposed to 
having to manually print it out at the end fo execution. 
Return values similar to dictionary calls: t.get()
*/

#include <future> //standard library for handling asynchronous tasks

//Define the range calculation function

uint64_t GetRangeSum(uint64_t start, uint64_t end)
{
    uint64_t sum = 0; 
    for (uint64_t i = start; i < end; i++)
    {
        sum += 1;
    }

    return sum;
}

int main(void)
{
    const int num_of_threads = 10;
    const int num_of_elements = 1000 * 1000 * 1000;
    const int step = num_of_elements / num_of_threads;

    //New: define a vector of Futures that will be used to keep track of tasks
    std::vector<std::future<uint64_t>> tasks;

    //What is a future? Describes tasks that will be done in the future. Placeholder type for asynchronous programming. 
    
    for (uint64_t i = 0; i < num_of_threads; i++)
    {
        //Add an asynchronous task to the vector of tasks. Same approach as used for multi-threading: functions, then args.
        tasks.push_back(std::async(GetRangeSum, i * step, (i + 1) * step));
    }

    uint64_t total = 0; 

    for (auto &t : tasks){ // each task must end before the final result can be retrieved
        total += t.get(); //retrieve task data like a key in dict
    }

    std::cout << "Total is: " << total << std::endl;

    /*Key takeaways: 
    - future values are returned from async()
    - future value, if not ready, will block the execution of other elements in program
    - default tasks start as soon as they are created
    - check out std::promise as return values to std::async */
}

//FOR THE FUTURE: Practice w/ task assignments using pointer to function, functors, and lambda. 

//NEXT: thread pool and memory access, management (i.e. mutex, lock...)

