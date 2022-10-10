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

## Concurrent programming references

- [Review of many Mutex implementations](http://cbloomrants.blogspot.com/2011/07/07-15-11-review-of-many-mutex.html)
- [sysprog21 / concurrent-programs](https://github.com/sysprog21/concurrent-programs)

### Lock-free

- [An Introduction to Lock-Free Programming](https://preshing.com/20120612/an-introduction-to-lock-free-programming/)
- [A Fast Lock-Free Queue for C++](https://moodycamel.com/blog/2013/a-fast-lock-free-queue-for-c++.htm)
- [Data Plane Development Kit (DPDK)'s Ring Library](https://doc.dpdk.org/guides/prog_guide/ring_lib.html)
  - Code [here](https://github.com/DPDK/dpdk/tree/main/lib/ring)
- [Linux Lockless Ring Buffer Design](https://lwn.net/Articles/340400/)

#### Code Examples
##### SPSC

- [cameron314 / readerwriterqueue](https://github.com/cameron314/readerwriterqueue)
- [facebook / folly / ProducerConsumerQueue](https://github.com/facebook/folly/blob/fe37daacf861700585d303b277138cb18a92686c/folly/ProducerConsumerQueue.h)

##### MPMC

- [cameron314 / concurrentqueue](https://github.com/cameron314/concurrentqueue)
- [facebook / folly / MPMCQueue](https://github.com/facebook/folly/blob/fe37daacf861700585d303b277138cb18a92686c/folly/MPMCQueue.h)
- Ring Implementation in FreeBSD*
  - [bufring.h in FreeBSD](https://svnweb.freebsd.org/base/release/8.0.0/sys/sys/buf_ring.h?revision=199625&amp;view=markup)
  - [bufring.c in FreeBSD](https://svnweb.freebsd.org/base/release/8.0.0/sys/kern/subr_bufring.c?revision=199625&amp;view=markup)
##### MPSC

- [rmind / ringbuf](https://github.com/rmind/ringbuf)
- [lemonrock / lock-free-multi-producer-single-consumer-ring-buffer](https://github.com/lemonrock/lock-free-multi-producer-single-consumer-ring-buffer)

[mutex_dir]: mutex
[spinlock]: mutex/spinlock_mutex.h
[data_mutex]: mutex/data_mutex.h

[task_queue_dir]: task_queue
[simple_serial_task_queue]: task_queue/simple_serial_task_queue.h
[task_queue]: task_queue/task_queue.h

[ring_buffer_dir]: ring_buffer
[ring_buffer]: ring_buffer/ring_buffer.h