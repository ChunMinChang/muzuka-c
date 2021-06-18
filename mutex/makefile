CC = g++
CPPFLAGS = -Wall -std=c++17
RM=rm -f

all: spinlock_mutex_test data_mutex_test

spinlock_mutex_test: spinlock_mutex_test.cpp spinlock_mutex.h
	$(CC) $(CPPFLAGS) -o spinlock_mutex_test spinlock_mutex_test.cpp

data_mutex_test: data_mutex_test.cpp data_mutex.h
	$(CC) $(CPPFLAGS) -o data_mutex_test data_mutex_test.cpp

clean:
	$(RM) spinlock_mutex_test data_mutex_test
