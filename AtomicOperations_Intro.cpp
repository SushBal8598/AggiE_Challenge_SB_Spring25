//AggiE Challenge Spring 2025

/*The most confusing topic in C++...
Review: sharing is NOT caring; but locks help ensure that calls to each variable are met with appropriate results. 
Race conditions: create undefined behavior, memory access issues. 

Memory model: threads
- Each read and write from directory. 
- Thread only writes to cache, creating penalties for other threads accessing memory results. 

Solutions: 
1) Don't share
2) Mutexes, locks
3) std::atomic (memory orders)
4) Abstraction */

//std::atomic examples
#include <atomic>

std::atomic<int> x_int(0); //read, increment, write process, occuring atomically

//Use parentheses or braces for assignment after initilization

void Increment_Atomic_1()
{
    x_int++;
}

/*No overlap in the read, write process between two threads.
Well-defined behavior when one thread is atomic, and another thread is reading from it. */

//Types of atomic variables; includes integral variables like characters
std::atomic<unsigned int> uint_atomic(0);
std::atomic<bool> bool_atomic(false);
std::atomic<double> double_atomic(0.0);
std::atomic<int*> int_ptr(0);
std::atomic<unsigned long long> ull_atomic(193939);

//Copy atomic<T> with memcpy(). No defined copy constructors, assignment, etc. Vectors are NOT atomic. 
class A
{
    int myInt; 
    int myInt2;
    long myLong;
    long myLong2;
    A(int val = 0){myInt = myInt2 = myLong = myLong2 = val;} //default constructor, so atomic
};

//Non-atomic (contains copy constructor)
class B_non
{
    int secondInt;
    int secondInt2;
    long secondLong;
    long secondLong2;
    B_non(const B_non&copy) : //custom copy constructor, not atomic
        secondInt(copy.secondInt),
        secondInt2(copy.secondInt2),
        secondLong(copy.secondLong),
        secondLong2(copy.secondLong2) {}
};

//Atomic class #2
class C
{
    int thirdInt;
    int thirdInt2; 
    long thirdLong;
    long thirdLong2;
    void IncrementInteger()
    {
        thirdInt++;
    }
};

//Proper way to copy atomic assignments
int main()
{
    std::atomic<int> a(1), b(2);
    a.store(b.load()); //stores the value of b to the variable a

    //The assingment works if both are of atomic type in a user-defined class
}

int switcharoo()
{
    //Explicit access: read OR write
    std::atomic<int> atomic_x(1); //atomic variable atomic_x set to 1
    int y(2);

    y = atomic_x.load();
    atomic_x.store(y);
}

int exchange()
{
    //Exchange: read and write at the SAME time
    std::atomic<int> atomic_x(1);
    int y = 2, z = 3;
    z = atomic_x.exchange(y);

    //Final z-x-y: 1 2 2 
}

//Compare exchange: based on a conditional sentry
int conditional_exchange()
{
    std::atomic<int> test_atomic(1);
    int expected = 2; 
    int desired = 3;
    bool success = test_atomic.compare_exchange_strong(expected, desired); //if the expected matches up with test_atomic, value 
    //is changed to desired

    //Expected variable is changed to the value of test_atomic if bool is false
    //Ensures that the value of the new variable is not taken if the mutex has been written to
}

/* Overall process for COMPARE EXCHANGE is: 
1) Read value
2) Calculate new value (modification) 
3) Update expected value IF value was modified. 
4) Write expected value IF value was not modified. 

Use WEAK when working with loops (can create false negatives) 

"LOCK-FREE" = Mutex-free */

//Check if lock-free:
bool check_if_lock()
{
    std::atomic<int> A(28);
    return A.is_lock_free();

    //Always lock-free?
    A.is_always_lock_free;
}
