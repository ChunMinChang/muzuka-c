#ifndef TaskQueue_h
#define TaskQueue_h

#include <cassert>
#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <utility>

// TaskQueue
//     A task queue that runs the tasks in parallel as much as it can. The
//     submitted task will be run on one of the worker thread created by the
//     TaskQueue itself
//
// Usage:
//     std::atomic<int> shared_number(0); // Or use int with mutex instead
//
//     {
//         TaskQueue q(2);
//
//         auto f1 = q.dispatch([&] { // Task 1
//             shared_number += 1;
//             return shared_number.load()
//         });
//         auto f2 = q.dispatch([&] { // Task 2
//             shared_number -= 1;
//             return shared_number.load()
//         });
//         auto f3 = q.dispatch([&] { // Task 3
//             shared_number += 1;
//             return shared_number.load()
//         });
//
//         f1.wait(); // Block the current thread until task 1 is done
//         f2.wait(); // Block the current thread until task 2 is done
//         // The value of shared_number is unpredictable. We don't know if
//         // task 3 is done or not.
//
//         int v = f3.get(); // Block the current thread until task 3 is done
//         // Now we know what shared_number is since task 1, 2, and 3 are all
//         // done!
//         assert(shared_number == 1);
//         // However, we don't know how the tasks are scheduled exactly. We
//         // have no idea about when the tasks are finshed. so the value of v
//         // is unpredictable.
//
//         auto f4 = q.dispatch([&] { // Task 4
//             shared_number += 1;
//             return shared_number.load()
//         });
//         auto f5 = q.dispatch([&] { // Task 5
//             shared_number += 1;
//             return shared_number.load()
//         });
//         auto f6 = q.dispatch([&] { // Task 5
//             shared_number += 1;
//             return shared_number.load()
//         });
//         // Task 4, 5 and 6 are very likely to be dropped 
//     } // q is dropped
//
//     // The value of shared_number is unpredictable since we don't know task
//     // 4, 5 and 6 are done or not. They are very likely to be dropped when q
//     // was deconstructed.
//
// TODO:
// Use thread-local work queues to avoid contention on the global work queue
class TaskQueue {
public:
    // Main thread APIs
    explicit TaskQueue(size_t threads): destroyed(false) {
        // TODO: Any benefit to clamp threads?
        // threads = std::min(threads, std::thread::hardware_concurrency());
        while (threads--) {
            workers.emplace_back(std::thread(&TaskQueue::work, this));
        }
    }

    ~TaskQueue() {
        {
            std::lock_guard<std::mutex> guard(mutex); // Enter critical section
            assert(!destroyed);
            destroyed = true; // Drop the unprocessed tasks
        } // Leave critical section

        // Wake up workers to terminate the works
        cv.notify_all();

        // Wait for the workers' terminations
        for (std::thread& worker: workers) {
            worker.join();
        }
    }

    template<class F>
    std::future<typename std::result_of<F()>::type> dispatch(F function) {
        typedef typename std::result_of<F()>::type Result;

        std::packaged_task<Result()> task(std::move(function));
        std::future<Result> result(task.get_future());

        {
            std::lock_guard<std::mutex> guard(mutex); // Enter critical section
            queue.emplace(std::move(task));
        } // Leave critical section

        // Wake up one woker to perform the task if it's in waiting mode
        cv.notify_one();
        return result;
    }

    // Disallowed operations
    TaskQueue(const TaskQueue& rhs) = delete;
	TaskQueue(TaskQueue&& rhs) = delete;
    TaskQueue& operator=(const TaskQueue& rhs) = delete;
	TaskQueue& operator=(TaskQueue&& rhs) = delete;

protected:
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
            
            MoveOnlyTask task = std::move(queue.front());
            queue.pop();
            lock.unlock(); // Leave critical section

            // Run the task on worker thread now
            task();

            // Once the task is done, worker will acquire the mutex again and
            // see if it has works to do. If TaskQueue is being destroyed, then
            // worker will leave the loop and terminate the execution. If the
            // TaskQueue is still alive and there are pending tasks, then 
            // worker will perform the tasks. Otherwise, the worker will go to
            // idle state and wait for someone waking it up
        }
    }

    // We will wrap the task into std::packaged_task<> and put it into the queue
    // when the task is submitted. std::packaged_task<> instance is only movable
    // and non-copyable. Thus, we create a type-ignored, movable-only class to 
    // store the submitted task in the queue.
    class MoveOnlyTask {
    public:
        template<class F>
        MoveOnlyTask(F&& f): runner(new RunnerImpl<F>(std::move(f))) {}

        MoveOnlyTask(MoveOnlyTask&& other): runner(std::move(other.runner)) {}

        MoveOnlyTask& operator=(MoveOnlyTask&& other) {
            runner = std::move(other.runner);
            return *this;
        }

        void operator()() { runner->call(); }

        // Disallowed operations
        MoveOnlyTask() = delete;
        MoveOnlyTask(const MoveOnlyTask& other) = delete;
        MoveOnlyTask(MoveOnlyTask& other) = delete;
        MoveOnlyTask& operator=(const MoveOnlyTask& other) = delete;

    private:
        struct Runner {
            virtual void call() = 0;
            virtual ~Runner() {}
        };

        template<typename F>
        struct RunnerImpl: Runner {
            F func;
            RunnerImpl(F&& f): func(std::move(f)) {}
            void call() { func(); }
        };

        std::unique_ptr<Runner> runner;
    };

    std::mutex mutex;
    std::queue<MoveOnlyTask> queue; // Protected by mutex
    bool destroyed; // Protected by mutex
    
    std::condition_variable cv;

    std::vector<std::thread> workers;
};

class SerialTaskQueue final: public TaskQueue {
public:
    SerialTaskQueue(): TaskQueue(1) {}
    ~SerialTaskQueue() = default;
};

#endif // TaskQueue_h