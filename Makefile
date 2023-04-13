# Copyright Valentin-Ioan VINTILĂ 2023.
# All rights reserved.

CC = g++ 
CFLAGS = -Wall -Wextra -std=c++17 -DLOG_ENABLE # Uncomment for debugging
LDFLAGS = -lm 

# Non-files
.PHONY: build clean

# - - - - -

# Build everything
build: structs.o server subscriber

# Build individually
structs.o: structs.cpp
	$(CC) $(CFLAGS) $(LDFLAGS) -c $< -o $@

server: server.cpp structs.o
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

subscriber: subscriber.cpp structs.o
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

# - - - - -

# Testing
IP_SERVER = 127.0.0.1
PORT = 12345

# Run the server
run_server:
	./server ${PORT}

# Run the client
run_client:
	./subscriber abcdef ${IP_SERVER} ${PORT}

# - - - - -

# Cleanup
clean:
	rm -rf server subscriber *.o

# - - - - -

# TODO
pack:
	exit