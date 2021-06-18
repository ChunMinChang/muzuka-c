CC = g++
CPPFLAGS = -Wall -std=c++17
RM=rm -f

all: ring_buffer_test

ring_buffer_test: ring_buffer_test.cpp ring_buffer.h
	$(CC) $(CPPFLAGS) -o ring_buffer_test ring_buffer_test.cpp

clean:
	$(RM) ring_buffer_test
