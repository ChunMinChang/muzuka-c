# Muzuka-c

This is a collection of some modern skills for multithreading and concurrency in C++

## Run the demo

Run `make` to build the examples and run `make clean` to delete the built examples

## Examples

- `SpinlockMutex`: A simple mutex implementation based on
  `std::atomic_flag::test_and_set`
  - Run `./spinlock_mutex_test`
- `DataMutex`: A Rust-style mutex in C++
  - Run `./data_mutex_test`
- `SimpleSerialTaskQueue`: A simple serial queue implementation
  - Run `./simple_serial_task_queue_test` 
- `TaskQueue`: A general task queue running tasks in parallel.
  The concept is similar to `SimpleSerialTaskQueue` but it runs the tasks in
  several threads at the same time instead of running them sequentially
  - Run `./task_queue_test`
- `SPSCRingBuffer`: A thread-safe single-producer-single-consumer circular buffer
  - Run `./ring_buffer_test`
