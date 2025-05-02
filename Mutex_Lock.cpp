//AggiE Challenge Spring 2025

//Following tutorial created by Prof. Ari Seif: https://www.youtube.com/watch?v=jwJ4Eh_2Umo

/* RACE CONDITION: arises when two or more threads compete for access to a shared resource (such as physical state)
- Example: Two threads t1, t2 race to write to a variable of the same name.*/

//Race conditions a thousand times
#include <mutex>
#include <numeric>
#include <thread>
#include <vector>
#include <iostream>

unsigned long f_x;

int main(void) {
    for (int i = 0; i < 1000; i++)
    {
        auto t1 = std::thread([]() {f_x = 1;});
        auto t2 = std::thread([]() {f_x = 2;}); //simple race condition

        //Joining threads together
        t1.join();
        t2.join();

        //Relatively undefined behavior (who wins? not deterministic)
        std::cout << "f_x is: " << f_x << std::endl;
    }
}

//Incrementing threads with vector
unsigned long g_counter = 0; 

void incrementer() 
{
    for (unsigned int i = 0; i < 100; i++)
    {
        g_counter++; //increment the counter, which creates race conditions when multithreading
    }
}

int main() {
    std::vector<std::thread> threads; //vector of threads

    for (int i = 0; i < 100; i++)
    {
        threads.push_back(std::thread(incrementer)); //add a thread paired with function
    }

    for (std::thread &t : threads)
    {
        t.join(); //for thread in threads, join to main
    }

    std::cout << "The value of g is: " << g_counter << std::endl;
    //Output: most results are 10,000, as expected, but a number deviate because of race condition.
}

/* What went wrong? 
- For each increment, thread must first read variable, then increment, then return value. 
- Undeterministic in interleaving because of different reads at the same time.
- What is written is NOT what is executed -- compiler will optimize code. Visible when data is shared, i.e. in multithreading.
- <atomic> operations work as a single unit. 
*/

/* Mutex and Lock
1) lock, unlock
2) lock_guard
3) unique_lock
4) shared_lock
5) scoped_lock */

/*Mutex starter
- Protect a section called critical section, which only one thread has access to at a time 
- Considered an ATOMIC operation, and mutually exclusive. 
*/

//1) create shared variable of type mutex
std::mutex mutex_var; //this is like a bool for protected, unprotected...
unsigned long mutex_counter; 

void incrementer_mutex()
{
    for (int i = 0; i < 100; i++)
    {
        mutex_var.lock(); //start the critical setion, sandwiched in the middle
        mutex_counter++;
        mutex_var.unlock(); //unlock data, unprotected. 1:1

        //WILL STAY LOCKED IF ENCOUNTERING EXCEPTION

        //DEADLOCK: Threads are locked in standstill, waiting for other thread to finish with resources.
    }
}

//std::lock_guard: RAII model, automatically locking or unlocking. (Keeps locked)

std::mutex mutex_locked; 
unsigned long mutex_locked_counter; 

void increment_locked()
{
    for (int i = 0; i < 100; i++)
    {
        //Constructor called to close mutex lock. Released when out of scope, such as when destructor called or exception thrown.
        std::lock_guard<std::mutex> guard(mutex_locked); //RAII model and approach to resource aquisition
        mutex_locked_counter++;
    }
}

//Unique lock: Lock guard + manual unlock, lock for "tranafer of ownership" approach

std::mutex mutex_unique_locked; 
unsigned long mutex_locked_counter_unique;

void increment_unique()
{
    for (int i = 0; i < 100; i++)
    {
        std::unique_lock<std::mutex> u1(mutex_unique_locked);
        mutex_locked_counter_unique++;

        //Manual lock and unlock
        u1.unlock(); //note default state

        std::cout << "Placeholder for non-critical action" << std::endl;

        u1.lock();
    }
}

//Shared mutex: used for read-only requirements

std::shared_mutex value_shared;
unsigned long shared_lock_read;

void increment_shared()
{
    for (int i = 0; i < 100; i++)
    {
        std::unique_lock<std::shared_mutex> u1(value_shared);

        //This space is only for a single thread
        shared_lock_read++;
    }
}

//Read-only. Need to check CPP version...
void read_only()
{
    for (int i = 0; i < 100; i++)
    {
        std::shared_lock<std::shared_mutex> s1(value_shared);

        //Space for multiple readers if no thread is locked. 
        std::cout << "The value of the counter is: " << shared_lock_read << std::endl;
    }
}

/* Can also use multiple mutex locks, but creates a problem: 
1) Multiple threads running operations in the reverse order
2) Creates race condition because threads are waiting for locks; DEADLOCK
*/

//All-or-nothing std::lock 

std::mutex g1, g2;
unsigned long counter_final;

void increment_allOrNothing()
{
    for (int i = 0; i < 100; i++)
    {
        std::lock(g1, g2); //locks both mutexes
    }

    std::lock_guard<std::mutex> lock1(g1, std::adopt_lock); //adopt_lock assumes the mutex is locked
    std::lock_guard<std::mutex> lock2(g2, std::adopt_lock); //adopt_lock assumes the mutex is locked
    counter_final++;

}

//Alternative approach: scope lock

void scopeIncrement() {
    for (int i = 0; i < 100; i++)
    {
        std::scoped_lock scope_lock_manual(g1, g2);
        counter_final++;
    }
}

/* Lesson summary
- std::mutex: manual lock/unlock
- std::unique_lock: RAII, scoped lock/unlock
- std::shared_lock: reader permissions
- std::scoped_lock: multiple locks without deadlock

*/

