//AggiE Challenge Spring 2025

/* std::atomic memory functions, moving away from constants (previous) 
1) Fetch: obtains value from pointer, then applies atomic operation
2) Operator (++, --...): increments, decrements
3) Operator (+=, -=, &=): performs bitwise comparison and assignment, or addition, subtraction...
*/

#include <atomic>
#include <vector>
#include <thread>
#include <iostream>
#include <cassert> //for assertion example
#include <string>

//Using fetch_add()
std::atomic<long long> data_1, data_2;

void add_1()
{
    data_1++;
}

void add_2()
{
    data_2.fetch_add(1);
}

int main()
{
    std::vector<std::thread> threads; //create a vector of threads for multithreading
    for (int i = 0; i < 5; i++)
    {
        threads.push_back(std::thread(add_1));
        threads.push_back(std::thread(add_2)); //Create a thread tied to each operation
    }

    for (std::thread &t : threads) //for thread in vector db...
    {
        if (t.joinable())
        {
            t.join(); //join the thread if it hasn't been joined yet
        }
    }

    std::cout << "Data of 1 is: " << data_1 << std::endl;
    std::cout << "Data of 2 is: " << data_2 << std::endl;

    //Use an assert to test equivalence
    assert(data_1 == data_2);
}

//Atomic operations of fetch_add
int fetch_add_overview(int x, int y)
{
    int tmp = x.load();
    tmp += 1;
    x.store(tmp);
    y = x.load();
}  

//Atomic operations should run as a single individual unit each. 

/* Specialized member functions only apply to specific types. 
1) Fetch applied to integer, floating point pointers. 
2) Fetch_and, or, not applied to integers only. 
3) Operators++, --, etc. applied to integer pointers only. 
4) Operator+= also applies to floating point pointers. 
5) Operator &= applies to integers. */

//Program order is sequenced-before: operations run sequentially, even if compiler changes order to "tune"

//The message wanted...
int order_wanted()
{
    int b, x, y;
    int flag = 1;
    b+= flag;
    x = 3;
    y += x;
    std::string msg;
    msg = "Hello, world...";

    std::cout << "Message is: " << msg << std::endl;
    std::cout << "B: " << b << "; y: " << y << std::endl;
}

//Creates an alternate order
int order_alternate()
{
    int b, x, y;
    int flag = 1;
    y += x;
    b += flag;
    std::string msg;
    msg = "Hello, world...";
    flag = 1;
    x = 3;

    std::cout << "Message is: " << msg << std::endl;
    std::cout << "B: " << b << "; y: " << y << std::endl; //When this is multithreaded, one thread "sees" through the other threads
}

/*Have one thread perform an operation using other thread's data by checking until sentry variable equals, or doesn't equal, some constant.
Program order is undefined when working through multiple threads. 

HAPPENS-BEFORE: The effects of operation on A, if performed before B, are made available to thread performing B. 

SOLUTION: Make global variables ATOMIC. 
*/

std::atomic<int> flag2(0);
std::atomic<std::string> msg_2;
int b2 = 0, x2 = 0, y2 = 0;
std::string text_2;

//Atomically change in sequential order
int change_atomics()
{
    text_2 = "Hi";

    while (flag2 != 1)
    {
        text_2 = msg_2;
    }

    std::cout << "text_2 is: " << text_2 << std::endl;
}

/* Using tags for sequential operations*/

std::atomic<int> atomic_2(1);
int y(2);

int atomic_args()
{
    y = atomic_2.load(std::memory_order_seq_cst);
    atomic_2.store(y, std::memory_order_seq_cst); //Strongest memory order, ensuring that all threads operate along the same order
}

//Using memory order relaxed
void store_message_relaxed()
{
    msg_2.store(std::string("Hello, world..."), std::memory_order_relaxed);
    flag2.store(1, std::memory_order_relaxed);
}

//There is no synchronization during the read-opp. 
void read_message()
{
    while (flag2.load(std::memory_order_relaxed) == 0) //Weak memory rule, requiring that everything perform as one atomic unit
    {
        if (msg_2.load(std::memory_order_relaxed) != "Hello, world...")
        {
            bool c = true;
        }
    }
}

/* Store and release used as a pair on the same variable
Area between lock, unlock of mutex is the CRTICIAL SECTION. Operations can move IN, but not OUT.

When one thread's mutex is locked, the other thread's mutex is UNLOCKED. (Performs synchronization)
Mutex methods will generally run faster than ++ atomics when thread count increases.
*/
