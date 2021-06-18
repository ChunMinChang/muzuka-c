#include "spinlock_mutex.h"

#include <cassert>
#include <iostream>
#include <thread>
#include <chrono>

const std::chrono::duration<int, std::milli> TASK_1_DELAY(10);
const std::chrono::duration<int, std::milli> TASK_2_DELAY(20);
const std::chrono::duration<int, std::milli> TASK_DELAY(10);
const int DUMMY_COUNT = 10;
const int TASK_1_OFFSET = 3;
const int TASK_2_OFFSET = -3;

// Global variables
SpinlockMutex MUTEX;
int SHARED_DATA = 60; // Protected by MUTEX

void dummy_task(int task_id, int& data, int offset) {
    for (int i = 0; i < DUMMY_COUNT; ++i) {
        data += offset;
        std::cout << "Task " << task_id << ": " << data << std::endl;
        std::this_thread::sleep_for(TASK_DELAY);
    }
}

void task_1() {
    MUTEX.lock(); // Enter critical section here
    std::cout << "Task 1: enter critical section" << std::endl;
    std::this_thread::sleep_for(TASK_1_DELAY);
    dummy_task(1, SHARED_DATA, TASK_1_OFFSET);
    MUTEX.unlock(); // Leave critical section here
}

void task_2() {
    MUTEX.lock(); // Enter critical section here
    std::cout << "Task 2: enter critical section" << std::endl;
    std::this_thread::sleep_for(TASK_2_DELAY);
    dummy_task(2, SHARED_DATA, TASK_2_OFFSET);
    MUTEX.unlock(); // Leave critical section here
}

int main() {
    std::thread t1(task_1);
    std::thread t2(task_2);

    t1.join();
    t2.join();

    assert(SHARED_DATA == (TASK_1_OFFSET + TASK_2_OFFSET) * DUMMY_COUNT + 60);

    return 0;
}