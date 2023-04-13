# Copyright Valentin-Ioan VINTILĂ 2023.
# All rights reserved.

CC = g++
CFLAGS = -Wall -Wextra -std=c++17 -DLOG_ENABLE # Uncomment for debugging
LDFLAGS = -lm

# Non-files
.PHONY: build clean

# Build everything
build: clean server subscriber

# Build individually
server: server.cpp
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

subscriber: subscriber.cpp
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

# Cleanup
clean:
	rm -rf server subscriber *.o
