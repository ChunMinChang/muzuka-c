CC = g++
CFLAGS = -Wall -std=c++17
RM=rm -f

all:\
	spinlock_mutex_test\
	data_mutex_test\
	simple_serial_task_queue_test\
	task_queue_test\
	ring_buffer_test

spinlock_mutex_test: spinlock_mutex_test.cpp spinlock_mutex.h
	$(CC) $(CFLAGS) -o spinlock_mutex_test spinlock_mutex_test.cpp

data_mutex_test: data_mutex_test.cpp data_mutex.h
	$(CC) $(CFLAGS) -o data_mutex_test data_mutex_test.cpp

simple_serial_task_queue_test: simple_serial_task_queue_test.cpp simple_serial_task_queue.h
	$(CC) $(CFLAGS) -o simple_serial_task_queue_test simple_serial_task_queue_test.cpp

task_queue_test: task_queue_test.cpp task_queue.h
	$(CC) $(CFLAGS) -o task_queue_test task_queue_test.cpp

ring_buffer_test: ring_buffer_test.cpp ring_buffer.h
	$(CC) $(CFLAGS) -o ring_buffer_test ring_buffer_test.cpp

clean:
	$(RM) spinlock_mutex_test data_mutex_test simple_serial_task_queue_test task_queue_test ring_buffer_test
