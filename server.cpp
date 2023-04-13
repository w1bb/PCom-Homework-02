// Copyright Valentin-Ioan VINTILĂ 2023.
// All rights reserved.

// TODO - update to something else
#include <bits/stdc++.h>

#include "structs.hpp"
#include "utils.hpp"

using namespace std;

// Map between ID and queue
unordered_map<string, queue<tcp_message_t> > subscriber_queues;

// This routine is called once a TCP client is connected
void tcp_client_connected(string user_id) {
    if (!subscriber_queues[user_id].empty()) {
        // Each message should be sent
    }
}

int main(int argc, char *argv[]) {
    // Disable stdout buffer
    setvbuf(stdout, NULL, _IONBF, BUFSIZ);

    int rc;
    
    // - - - - -

    // Check the number of arguments and save them
    if (argc != 2) {
        log("[ server ] Server expects exactly one parameter, not %d\n", argc - 1);
        return -1;
    }
    uint16_t server_port;
    rc = sscanf(argv[2], "%hu", &server_port);
    if (rc < 0) {
        log("[ server ] Invalid port (NaN)\n");
        return -1;
    }
    // Check if server_port is reserved (0 <= server_port <= 1023)
    if (server_port <= 1023) {
        log("[ server ] Invalid port number (%d < 1024)\n", server_port);
        return -1;
    }

    // - - - - -

    // UDP listen socket
    int udp_listen_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_listen_fd < 0) {
        log("[ server ] socket - UDP socket fail\n");
        return -1;
    }
    struct sockaddr_in udp_addr;
    memset((char *) &udp_addr, 0, sizeof(sockaddr_in));
	udp_addr.sin_family = AF_INET;
	udp_addr.sin_port = htons(server_port);
	udp_addr.sin_addr.s_addr = INADDR_ANY;
    
    // - - - - -

    // TCP listen socket
    int tcp_listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (tcp_listen_fd < 0) {
        log("[ server ] socket - TCP socket fail\n");
        return -1;
    }
    int disable_neagle = 1;
    rc = setsockopt(tcp_listen_fd, IPPROTO_TCP, TCP_NODELAY,
                    &disable_neagle, sizeof(int));
    if (rc < 0) {
        log("[ server ] setsockopt - Could not disable neagle\n");
        return -1;
    }
    struct sockaddr_in tcp_addr;
    memset((char *) &tcp_addr, 0, sizeof(sockaddr_in));
	tcp_addr.sin_family = AF_INET;
	tcp_addr.sin_port = htons(server_port);
	tcp_addr.sin_addr.s_addr = INADDR_ANY;

    return 0;
}
