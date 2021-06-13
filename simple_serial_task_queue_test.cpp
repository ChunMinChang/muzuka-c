#include "simple_serial_task_queue.h"

#include <cassert>
#include <chrono>
#include <iostream>
#include <thread>

int main() {
    int number = 0;

	SimpleSerialTaskQueue q;

	q.dispatch([&]{
        std::cout << "Start task 1" << std::endl;
        std::this_thread::sleep_for (std::chrono::milliseconds(10));
        number += 1;
        std::cout << "End task 1" << std::endl;
    });

    q.dispatch([&]{
        std::cout << "Start task 2" << std::endl;
        std::this_thread::sleep_for (std::chrono::milliseconds(20));
        number += 3;
        std::cout << "End task 2" << std::endl;
    });

    q.dispatch([&]{
        std::cout << "Start task 3" << std::endl;
        std::this_thread::sleep_for (std::chrono::milliseconds(30));
        number += 5;
        std::cout << "End task 3" << std::endl;
    });

    // Do other works here on main thread: dispatch other tasks to other threads
    // maybe, or some critical jobs that must not be blocked ....

    // When we have availability, wait for the tasks
    q.wait();
    assert(number == 9);

    // Dispatch more jobs if needed
    q.dispatch([&]{
        std::cout << "Start task 4" << std::endl;
        std::this_thread::sleep_for (std::chrono::milliseconds(10));
        number += 7;
        std::cout << "End task 4" << std::endl;
    });

    q.dispatch([&]{
        std::cout << "Start task 5" << std::endl;
        std::this_thread::sleep_for (std::chrono::milliseconds(10));
        number += 9;
        std::cout << "End task 5" << std::endl;
    });

    q.wait();
    q.wait(); // It's ok to call it twice
    assert(number == 25);

    q.dispatch([&]{
        std::cout << "Start task 6" << std::endl;
        std::this_thread::sleep_for (std::chrono::milliseconds(50));
        number += 11;
        std::cout << "End task 6" << std::endl;
    });

    q.dispatch([&]{
        std::cout << "Start task 7" << std::endl;
        std::this_thread::sleep_for (std::chrono::milliseconds(10));
        number += 13;
        std::cout << "End task 7" << std::endl;
    });

    std::this_thread::sleep_for (std::chrono::milliseconds(60));

    // Without calling wait(), the value of number is unpredictable since we
    // don't know how many tasks are performed
    // assert(number == 49); // This will be hit frequently!

    // These two tasks are very likely to be dropped
    q.dispatch([&]{
        std::cout << "Start task 8" << std::endl;
        std::this_thread::sleep_for (std::chrono::milliseconds(10));
        number += 15;
        std::cout << "End task 8" << std::endl;
    });

    q.dispatch([&]{
        std::cout << "Start task 9" << std::endl;
        std::this_thread::sleep_for (std::chrono::milliseconds(10));
        number += 17;
        std::cout << "End task 9" << std::endl;
    });

	return 0;
}