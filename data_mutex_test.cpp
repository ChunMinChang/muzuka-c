#include "data_mutex.h"

#include <cassert>
#include <iostream>
#include <thread>
#include <chrono>

const std::chrono::duration<int, std::milli> TASK_DELAY(10);
const int DUMMY_COUNT = 10;
const int TASK_1_OFFSET = 3;
const int TASK_2_OFFSET = 5;

// Global variables
DataMutex<int> shared_data(60);

void dummy_task(int task_id, int& data, int offset) {
    for (int i = 0; i < DUMMY_COUNT; ++i) {
        data += offset;
        std::cout << "Task " << task_id << ": " << data << std::endl;
        std::this_thread::sleep_for(TASK_DELAY);
    }
}

void task_1() {
    {
        auto guard = shared_data.lock(); // Enter critical section
        dummy_task(1, guard.data(), TASK_1_OFFSET);
    } // Leave critical section
}

void task_2() {
    {
        auto guard = shared_data.lock(); // Enter critical section
        dummy_task(2, guard.data(), TASK_2_OFFSET);
    } // Leave critical section
}

int main() {
    std::thread t1(task_1);
    std::thread t2(task_2);

    t1.join();
    t2.join();

    {
        auto guard = shared_data.lock(); // Enter critical section
        assert(guard.data() == 
               (TASK_1_OFFSET + TASK_2_OFFSET) * DUMMY_COUNT + 60);
    } // Leave critical section

    return 0;
}