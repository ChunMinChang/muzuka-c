# Muzuka-c

This is a collection of some modern skills for multithreading and concurrency in C++

- [Mutex][mutex_dir]
  - [`SpinlockMutex`][spinlock]: A simple mutex implementation based on `std::atomic_flag::test_and_set`
  - [`DataMutex`][data_mutex]: A Rust-style mutex in C++
- [Task Queue][task_queue_dir]
  - [`SimpleSerialTaskQueue`][simple_serial_task_queue]: A simple serial queue implementation
  - [`TaskQueue`][task_queue]: A general task queue running tasks in parallel. The concept is similar to `SimpleSerialTaskQueue` but it runs the tasks in several threads at the same time instead of running them sequentially
- [Ring Buffer][ring_buffer_dir]
  - [`SPSCRingBuffer`][ring_buffer]: A thread-safe single-producer-single-consumer circular buffer

## Run the demo

Run `run.sh <FOLDER_NAME>` to play the examples, where the `<FOLDER_NAME>` is `mutex`, `task_queue`, or `ring_buffer`. Or you can simply go to those folder then build examples by running `make` and clean them by `make clean`.

[mutex_dir]: mutex
[spinlock]: mutex/spinlock_mutex.h
[data_mutex]: mutex/data_mutex.h

[task_queue_dir]: task_queue
[simple_serial_task_queue]: task_queue/simple_serial_task_queue.h
[task_queue]: task_queue/task_queue.h

[ring_buffer_dir]: ring_buffer
[ring_buffer]: ring_buffer/ring_buffer.h