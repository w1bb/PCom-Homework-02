// Copyright Valentin-Ioan VINTILĂ 2023.
// All rights reserved.

#include <bits/stdc++.h>

#include "structs.hpp"
#include "utils.hpp"

int main(int argc, char *argv[]) {
    // Disable stdout buffer
    setvbuf(stdout, NULL, _IONBF, BUFSIZ);

    int rc;
    char buf[2048]{};

    struct sockaddr_in server_address;
    memset((char *) &server_address, 0, sizeof(server_address));

    // - - - - -

    // Check the number of arguments and save them
    if (argc != 4) {
        log("Server expects exactly one parameter, not %d\n", argc - 1);
        return -1;
    }
    string id = argv[1];
    log("ID OK (%s)\n", id.c_str());
    rc = inet_pton(AF_INET, argv[2], &server_address.sin_addr);
    if (rc <= 0) {
        log("Wrong server IP address\n");
        return -1;
    }
    uint16_t server_port;
    rc = sscanf(argv[3], "%hu", &server_port);
    if (rc < 0) {
        log("Invalid port (NaN)\n");
        return -1;
    }
    // Check if server_port is reserved (0 <= server_port <= 1023)
    if (server_port <= 1023) {
        log("Invalid port number (%d < 1024)\n", server_port);
        return -1;
    }
    log("server_port OK (%u)\n", server_port);

    // - - - - -

    // General server connection details
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(server_port);
    int tcp_server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (tcp_server_fd < 0) {
        log("socket - TCP socket fail\n");
        return -1;
    }
    int disable_neagle = 1;
    rc = setsockopt(tcp_server_fd, IPPROTO_TCP, TCP_NODELAY,
                    &disable_neagle, sizeof(int));
    if (rc < 0) {
        log("setsockopt - Could not disable Neagle for client\n");
        return -1;
    }

    // - - - - -

    // Generate epoll
    struct epoll_event event, events[MAX_EPOLL_EVENTS];
    int epoll_fd = epoll_create1(0);
    // Add STDIN
    event.data.fd = STDIN_FILENO;
    event.events = EPOLLIN | EPOLLET;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, STDIN_FILENO, &event) < 0) {
        log("epoll_ctl - Could not add STDIN_FILENO\n");
        return -1;
    }
    // Add TCP listen
    event.data.fd = tcp_server_fd;
    event.events = EPOLLIN | EPOLLET;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, tcp_server_fd, &event) < 0) {
        log("epoll_ctl - Could not add tcp_server_fd\n");
        return -1;
    }
    log("Added stdin + server to poll\n");
    
    // - - - - -

    // Connect
    rc = connect(tcp_server_fd, (struct sockaddr *)&server_address, sizeof(server_address));
    if (rc < 0) {
        log("connect - Could not connect to server\n");
        return -1;
    }
    // Send ID to server
    memset(buf, 0, sizeof(buf));
    strcpy(buf, id.c_str());
    rc = send(tcp_server_fd, buf, sizeof(buf), 0);
    if (rc < 0) {
        log("send - Could not send ID to server\n");
        return -1;
    }

    // - - - - -

    bool forever = true;
    
    while (forever) {
        int num_events = epoll_wait(epoll_fd, events, MAX_EPOLL_EVENTS, -1);
        for (int i = 0; i < num_events; ++i) {
            if (!(events[i].events & EPOLLIN))
                continue;

            // - - - - -
            
            // Check for keyboard input
            if (events[i].data.fd == STDIN_FILENO) {
                if (fgets(buf, sizeof(buf), stdin) && !isspace(buf[0])) {
                    log("[ INPUT ] %s", buf);
                    // Check if "exit" was typed
                    if (!strncmp(buf, "exit", 4)) {
                        forever = false;
                        break;
                    }

                    message_from_tcp_t message;
                    strcpy(message.unique_id, id.c_str());
                    if (!strncmp(buf, "subscribe", 9)) {
                        sscanf(buf,
                               "%s %s %" SCNu8,
                               message.command,
                               message.topic,
                               &message.store_and_forward);
                        rc = send(tcp_server_fd, &message, sizeof(message), 0);
                        if (rc < 0) {
                            log("send - Could not send message to server\n");
                            return -1;
                        }
                        printf("Subscribed to topic.\n");
                    } else if (!strncmp(buf, "unsubscribe", 11)) {
                        sscanf(buf,
                               "%s %s",
                               message.command,
                               message.topic);
                        rc = send(tcp_server_fd, &message, sizeof(message), 0);
                        if (rc < 0) {
                            log("send - Could not send message to server\n");
                            return -1;
                        }
                        printf("Unsubscribed from topic.\n");
                    }
                    log("Sending a '%s %s %" SCNu8,
                        message.command,
                        message.topic,
                        message.store_and_forward);
                }
            }

            // - - - - -
            
            // Check for new server message
            else if (events[i].data.fd == tcp_server_fd) {
                tcp_message_t message_from_server;
                rc = recv(tcp_server_fd, &message_from_server, sizeof(message_from_server), 0);
                if (rc < 0) {
                    log("recv - Could not receive message from server\n");
                    return -1;
                }

                if (!strncmp(message_from_server.topic, "stop", 4)) {
                    forever = false;
                    break;
                }
                
                printf("%s:%hu - %s - %s - %s\n",
                       message_from_server.from_ip,
                       message_from_server.from_port,
                       message_from_server.topic,
                       msg_type_to_string(message_from_server.message_type).c_str(),
                       message_from_server.payload);
            }
        }
    }

    // - - - - -

    close(tcp_server_fd);

    log("All OK\n");
    return 0;
}
