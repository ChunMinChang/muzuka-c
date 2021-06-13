#ifndef SimpleSimpleSerialTaskQueue_h
#define SimpleSimpleSerialTaskQueue_h

#include <cassert>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <utility>

// SimpleSerialTaskQueue
//     A task queue that runs the tasks serially by the order they are
//     submitted. The submitted task will be run on the worker thread created
//     by the SimpleSerialTaskQueue itself
//
//     SimpleSerialTaskQueue(), ~SimpleSerialTaskQueue(), ::dispatch() and
//     ::wait() should be run on the same thread
//
// Usage:
//     int number = 0;
//     {
//         SimpleSerialTaskQueue q;
//         q.dispatch([&] { number += 1 }); // Runs on worker thread
//         q.dispatch([&] { number += 2 }); // Runs on worker thread
//         q.wait(); // Block the current thread until all the tasks are done
//     }
//     // Without calling wait(), the value of number is unpredictable since we
//     // don't know how many tasks are performed
//     assert(number == 3);
class SimpleSerialTaskQueue final {
public:
    // Main thread APIs
    SimpleSerialTaskQueue(): destroyed(false), waiting(false) {
        worker = std::thread(&SimpleSerialTaskQueue::work, this);
    }

    ~SimpleSerialTaskQueue() {
        {
            std::lock_guard<std::mutex> guard(mutex); // Enter critical section
            assert(!destroyed);
            destroyed = true; // Drop the unprocessed tasks
        } // Leave critical section

        // Wake up worker to terminate the work
        cv.notify_one();

        // Wait for the worker's termination
        worker.join();
    }

    template<class F>
    void dispatch(F&& function) {
        {
            std::lock_guard<std::mutex> guard(mutex); // Enter critical section
            // ~SimpleSerialTaskQueue should be called on same thread
            assert(!destroyed);
            // wait() should be called on same thread
            assert(!waiting);
            queue.emplace(std::move(function));
        } // Leave critical section

        // Wake up the woker to perform the task if it's in waiting mode
        cv.notify_one();
    }
    
    // Block the current thread until all the tasks are done
    // dispatch() and wait() should run on the same thread so no more tasks
    // will be appended to the queue when wait() is blocking the thread.
    void wait() {
        std::unique_lock<std::mutex> lock(mutex); // Enter critical section
        waiting = true;
        // while (!queue.empty()) {
        //     cv.wait(lock); // Release the lock and leave the critical section
        //     // Take the lock and enter critical section
        // }
        // Does same as above: queue will be accessed only in the critical
        // section 
        cv.wait(lock, [this]{
            return queue.empty();
        });
        // Now we are in the critical section
        waiting = false;
    }

    // Disallowed operations
    SimpleSerialTaskQueue(const SimpleSerialTaskQueue& other) = delete;
	SimpleSerialTaskQueue(SimpleSerialTaskQueue&& other) = delete;
    SimpleSerialTaskQueue& operator=(const SimpleSerialTaskQueue& other) = delete;
	SimpleSerialTaskQueue& operator=(SimpleSerialTaskQueue&& other) = delete;

private:
    // Perform the task in worker thread
    void work() {
        while (true) {
            std::unique_lock<std::mutex> lock(mutex); // Enter critical section
            // while (queue.empty() && !destroyed) {
            //     // Release the lock and leave the critical section
            //     cv.wait(lock);
            //     // Take the lock and enter critical section
            // }
            // Does same as above: queue and destroyed will be accessed only in
            // the critical section 
            cv.wait(lock, [this]{
                return queue.size() || destroyed;
            });
            // Now we are in the critical section
            
            if (destroyed) {
                // Terminate the work. Drop the unprocessed tasks
                break;
            }
            
            Task task = std::move(queue.front());
            // The queue.pop() will be called once the task is done
            // so !queue.empty() indicates there are pending or running tasks
            lock.unlock(); // Leave critical section

            // Run the task on worker thread now
            task();

            lock.lock(); // Enter critical section
            // The task is done. Remove it from the queue
            queue.pop();
            // See if we need to wake up the thread calling wait()
            bool wakeup_wait = waiting && queue.empty();
            lock.unlock(); // Leave critical section

            // Wake up the thread blocked by wait()
            if (wakeup_wait) {
                cv.notify_one();
            }

            // Once the task is done, worker will acquire the mutex again and
            // see if it has works to do. If SimpleSerialTaskQueue is being 
            // destroyed, then worker will leave the loop and terminate the
            // execution. If the SimpleSerialTaskQueue is still alive and there
            // are pending tasks, then worker will perform the tasks. Otherwise,
            // the worker will go to idle state and wait for someone waking it up
        }
    }

    typedef std::function<void()> Task;

    std::mutex mutex;
    std::queue<Task> queue; // Protected by mutex
    bool destroyed; // Protected by mutex
    bool waiting; // Protected by mutex
    
    std::condition_variable cv;

    std::thread worker;
};

#endif // SimpleSimpleSerialTaskQueue_h