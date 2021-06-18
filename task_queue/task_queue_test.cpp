#include "task_queue.h"

#include <atomic>
#include <cassert>
#include <iostream>
#include <vector>

void test_queue_example() {
    std::cout << "\n----- " << __func__ << " -----" << std::endl;

    std::atomic<int> shared_number(0);

    const size_t THREADS = 3;
    const size_t TASKS = 2 * THREADS + 1;

    {
        TaskQueue q(THREADS);

        std::vector<std::future<int>> futures(TASKS);
        for (size_t id = 0 ; id < futures.size() ; ++id) {
            futures[id] = q.dispatch([&, id] {
                std::cout << "Start task " << id << std::endl;
                shared_number += id % 2? -1 : 1;
                std::cout << "End task " << id << std::endl;
                return shared_number.load();
            });
        }
        
        std::vector<int> results(futures.size());
        for (size_t id = 0 ; id < futures.size() ; ++id) {
            results[id] = futures[id].get();
        }

        std::cout << "shared_number: " << shared_number << std::endl;
        assert(shared_number == TASKS % 2);

        std::cout << "\nRun another " << TASKS 
            << " tasks, but they are very likely to be dropped" << std::endl;
        futures.clear();
        for (size_t id = 0 ; id < TASKS ; ++id) {
            futures.emplace_back(q.dispatch([&, id] {
                std::cout << "Start task " << id << std::endl;
                shared_number += id % 2? -1 : 1;
                std::cout << "End task " << id << std::endl;
                return shared_number.load();
            }));
        }
    }
}

void test_serial_queue_example() {
    std::cout << "\n----- " << __func__ << " -----" << std::endl;

    int number = 0;
    const size_t TASKS = 7;

    {
        SerialTaskQueue q;

        std::vector<std::future<int>> futures(TASKS);
        for (size_t id = 0 ; id < futures.size() ; ++id) {
            futures[id] = q.dispatch([&, id] {
                std::cout << "Start task " << id << std::endl;
                number += id % 2? -1 : 1;
                std::cout << "End task " << id << std::endl;
                return number;
            });
        }

        int last = futures.back().get();

        std::cout << "last: " << last << ", number: " << number << std::endl;
        assert(number == TASKS % 2);
        assert(number == last);

        std::cout << "\nRun another " << TASKS 
            << " tasks, but they are very likely to be dropped" << std::endl;
        futures.clear();
        for (size_t id = 0 ; id < TASKS ; ++id) {
            futures.emplace_back(q.dispatch([&, id] {
                std::cout << "Start task " << id << std::endl;
                number += id % 2? -1 : 1;
                std::cout << "End task " << id << std::endl;
                return number;
            }));
        }
    }
}

int main() {
    test_queue_example();
    test_serial_queue_example();
	return 0;
}