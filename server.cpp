// Copyright Valentin-Ioan VINTILĂ 2023.
// All rights reserved.

// TODO - update to something else
#include <bits/stdc++.h>

#include "dynamic_array.hpp"
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
        log("Server expects exactly one parameter, not %d\n", argc - 1);
        return -1;
    }
    uint16_t server_port;
    rc = sscanf(argv[1], "%hu", &server_port);
    if (rc < 0) {
        log("Invalid port (NaN)\n");
        return -1;
    }
    // Check if server_port is reserved (0 <= server_port <= 1023)
    if (server_port <= 1023) {
        log("Invalid port number (%d < 1024)\n", server_port);
        return -1;
    }
    log("server_port OK\n");

    // - - - - -

    // UDP listen socket
    int udp_listen_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_listen_fd < 0) {
        log("socket - UDP socket fail\n");
        return -1;
    }
    struct sockaddr_in udp_addr;
    memset((char *) &udp_addr, 0, sizeof(sockaddr_in));
	udp_addr.sin_family = AF_INET;
	udp_addr.sin_port = htons(server_port);
	udp_addr.sin_addr.s_addr = INADDR_ANY;
    log("UDP OK\n");
    
    // - - - - -

    // TCP listen socket
    int tcp_listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (tcp_listen_fd < 0) {
        log("socket - TCP socket fail\n");
        return -1;
    }
    int disable_neagle = 1;
    rc = setsockopt(tcp_listen_fd, IPPROTO_TCP, TCP_NODELAY,
                    &disable_neagle, sizeof(int));
    if (rc < 0) {
        log("setsockopt - Could not disable Neagle\n");
        return -1;
    }
    struct sockaddr_in tcp_addr;
    memset((char *) &tcp_addr, 0, sizeof(sockaddr_in));
	tcp_addr.sin_family = AF_INET;
	tcp_addr.sin_port = htons(server_port);
	tcp_addr.sin_addr.s_addr = INADDR_ANY;
    log("TCP OK\n");

    // - - - - -

    // Generate epoll
    struct epoll_event event, events[MAX_EPOLL_EVENTS];
    int epoll_fd = epoll_create1(0);
    // Add STDIN
    event.data.fd = STDIN_FILENO;
    event.events = EPOLLIN | EPOLLET;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, STDIN_FILENO, &event) < 0) {
        log("epoll_ctl - Could not add STDIN_FILENO");
        return 1;
    }
    // Add UDP listen
    event.data.fd = udp_listen_fd;
    event.events = EPOLLIN | EPOLLET;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, udp_listen_fd, &event) < 0) {
        log("epoll_ctl - Could not add udp_listen_fd");
        return 1;
    }
    // Add TCP listen
    event.data.fd = tcp_listen_fd;
    event.events = EPOLLIN | EPOLLET;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, tcp_listen_fd, &event) < 0) {
        log("epoll_ctl - Could not add udp_listen_fd");
        return 1;
    }
    // struct pollfd aux_pollfd;
    // dynamic_array<struct pollfd> poll_fds;
    // // Add STDIN
    // aux_pollfd.fd = STDIN_FILENO;
    // aux_pollfd.events = POLLIN;
    // poll_fds.push_back(aux_pollfd);
    // // Add UDP listen
    // aux_pollfd.fd = udp_listen_fd;
    // aux_pollfd.events = POLLIN;
    // poll_fds.push_back(aux_pollfd);
    // // Add TCP listen
    // aux_pollfd.fd = tcp_listen_fd;
    // aux_pollfd.events = POLLIN;
    // poll_fds.push_back(aux_pollfd);
    log("Added stdin + UDP + TCP to poll\n");

    // - - - - -

    while (1) {
        // int activity = poll(poll_fds.elems, poll_fds.size(), -1);
        int num_events = epoll_wait(epoll_fd, events, MAX_EPOLL_EVENTS, -1);
        for (int i = 0; i < num_events; ++i) {
            if (events[i].events & EPOLLIN) {
                std::cout << "Input available on stdin" << std::endl;
            }
        }
    }

    log("All OK\n");
    return 0;
}
