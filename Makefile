# Copyright Valentin-Ioan VINTILĂ 2023.
# All rights reserved.

CC = g++
CFLAGS = -Wall -Wextra -std=c++20 -DLOG_ENABLE # Uncomment for debugging
LDFLAGS = -lm

# Non-files
.PHONY: build clean

# Build everything
build: clean dynamic_array.o structs.o server subscriber

# Build individually
dynamic_array.o: dynamic_array.cpp
	$(CC) $(CFLAGS) $(LDFLAGS) -c $< -o $@

structs.o: structs.cpp
	$(CC) $(CFLAGS) $(LDFLAGS) -c $< -o $@

server: server.cpp structs.o
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

subscriber: subscriber.cpp structs.o
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

# Cleanup
clean:
	rm -rf structs.o server subscriber *.o
