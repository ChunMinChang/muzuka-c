CC = g++
CPPFLAGS = -Wall -std=c++17
RM=rm -f

all: simple_serial_task_queue_test task_queue_test

simple_serial_task_queue_test: simple_serial_task_queue_test.cpp simple_serial_task_queue.h
	$(CC) $(CPPFLAGS) -o simple_serial_task_queue_test simple_serial_task_queue_test.cpp

task_queue_test: task_queue_test.cpp task_queue.h
	$(CC) $(CPPFLAGS) -o task_queue_test task_queue_test.cpp

clean:
	$(RM) simple_serial_task_queue_test task_queue_test
