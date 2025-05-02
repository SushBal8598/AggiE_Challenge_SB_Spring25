//AggiE Challenge Spring 2025

//Create a queue that will not be disrupted by multithreading. Used in overall ASIO client-server framework. 
namespace common
{
    namespace net {
        //Create the template for the queue
        template<typename T> 
        class thread_queue 
        {
            public: 
                //Apply default constructor to queue
                thread_queue() = default;
                thread_queue(const thread_queue<T>&) = delete;

                //Add destructor for memory management...
                virtual ~thread_queue() {clear();}

                //Create front and back queue item maintenance. 
                const T & front()
                {
                    //RAII lock for the queue
                    std::scoped_lock lock(QueueMutex);
                    
                    //Return the item at the front of the queue
                    return DeqQueue.front();
                }

                const T & back()
                {
                    //RAII lock for the queue
                    std::scoped_lock lock(QueueMutex);
                    
                    //Return the item at the front of the queue
                    return DeqQueue.back();
                }

                //Push operations for queue (enqueue, dequeue in C#)
                void push_back(const T& item)
                {
                    std::scoped_lock lock(QueueMutex);
                    DeqQueue.emplace_back(std::move(item)); //Places at the back of the queue

                    //Notify the conditional variable
                    std::unique_lock<std::mutex> u1(blockingMux);
                    cvBlocking.wait(u1);
                }

                void push_front(const T& item)
                {
                    std::scoped_lock lock(QueueMutex);
                    DeqQueue.emplace_front(std::move(item)); //Places at the front of the queue

                    //Notify the conditional variable
                    std::unique_lock<std::mutex> u1(blockingMux);
                    cvBlocking.wait(u1);
                }

                //Queue convenience functions: determines the size of the queue, number of items, etc. 
                bool empty() 
                {
                   std::scoped_lock lock(QueueMutex); //Since scoped lock follows RAII model, the lock state will only depend on the calling of the method
                   return DeqQueue.empty(); 
                }

                size_t count() {
                    std::scoped_lock lock(QueueMutex);
                    return DeqQueue.size();
                }

                void clear() {
                    std::scoped_lock lock(QueueMutex);
                    DeqQueue.clear();
                }

                //Pop function: delete the item at front or end and return value. Overloaded functions.
                T pop_front()
                {
                    std::scoped_lock lock(QueueMutex);
                    auto t = std::move(DeqQueue.front());
                    DeqQueue.pop_front();
                    return T; 
                }

                T pop_back()
                {
                    std::scoped_lock lock(QueueMutex);
                    auto t = std::move(DeqQueue.back());
                    DeqQueue.pop_back();
                    return T; 
                }


            protected:

                //Store in a "deck" 
                std::mutex QueueMutex;
                std::deque<T> DeqQueue; //C++ double-ended queue object
        };
    }
}
