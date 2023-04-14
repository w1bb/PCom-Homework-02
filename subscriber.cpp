// Copyright Valentin-Ioan VINTILĂ 2023.
// All rights reserved.

// Standard
#include <cstdio>

// Other
#include "structs.hpp"
#include "utils.hpp"

using namespace std;

// - - - - -

int tcp_server_fd;

// Close the client peacefully
void close_client(int sig) {
    sig = sig; // Remove warning
    log("Closing the client (sig = %d)...\n", sig);
    close(tcp_server_fd);
    log("All OK\n");
    exit(EXIT_SUCCESS);
}

// Client entrypoint
int main(int argc, char *argv[]) {
    // Disable stdout buffer
    setvbuf(stdout, NULL, _IONBF, BUFSIZ);

    // Treat correctly Ctrl+C
    signal(SIGINT, close_client);

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
    tcp_server_fd = socket(AF_INET, SOCK_STREAM, 0);
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
    struct epoll_event event;
    struct epoll_event *events;
    int events_len = INITIAL_MAX_EPOLL_EVENTS;
    events = (epoll_event*)malloc(events_len * sizeof(struct epoll_event));
    if (!events) {
        log("malloc - Could not allocate memory for events\n");
        return -1;
    }
    int epoll_fd = epoll_create1(0);
    // Add STDIN
    event.data.fd = STDIN_FILENO;
    event.events = EPOLLIN;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, STDIN_FILENO, &event) < 0) {
        log("epoll_ctl - Could not add STDIN_FILENO\n");
        return -1;
    }
    // Add TCP listen
    event.data.fd = tcp_server_fd;
    event.events = EPOLLIN;
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
        int num_events = epoll_wait(epoll_fd, events, events_len, 0);
        for (int i = 0; i < num_events; ++i) {
            // Check for keyboard input
            if (events[i].data.fd == STDIN_FILENO) {
                if (fgets(buf, sizeof(buf), stdin) && !isspace(buf[0])) {
                    log("[ INPUT ] %s", buf);
                    vector<string> commands = split_command(buf);
                    if (commands.size() == 0)
                        continue;

                    // Check if "exit" was typed
                    if (commands[0] == "exit") {
                        forever = false;
                        break;
                    }

                    message_from_tcp_t message;
                    strcpy(message.unique_id, id.c_str());
                    strncpy(message.command, commands[0].c_str(), sizeof(message.command));

                    if (commands[0] == "subscribe") {
                        if (commands.size() < 3)
                            continue;
                        strncpy(message.topic, commands[1].c_str(), sizeof(message.topic));
                        message.store_and_forward = (commands[2][0] == '1');
                        rc = send(tcp_server_fd, &message, sizeof(message), 0);
                        if (rc < 0) {
                            log("send - Could not send message to server\n");
                            return -1;
                        }
                        printf("Subscribed to topic.\n");
                    } else if (commands[0] == "unsubscribe") {
                        if (commands.size() < 2)
                            continue;
                        strncpy(message.topic, commands[1].c_str(), sizeof(message.topic));
                        rc = send(tcp_server_fd, &message, sizeof(message), 0);
                        if (rc < 0) {
                            log("send - Could not send message to server\n");
                            return -1;
                        }
                        printf("Unsubscribed from topic.\n");
                    }
                    log("Sending a '%s %s %" SCNu8 "'\n",
                        message.command,
                        message.topic,
                        message.store_and_forward);
                }
            }

            // - - - - -
            
            // Check for new server message
            else if (events[i].data.fd == tcp_server_fd) {
                tcp_message_t message_from_server;
                memset(&message_from_server, 0, sizeof(message_from_server));
                rc = recv(tcp_server_fd, &message_from_server, sizeof(message_from_server), 0);
                if (rc < 0) {
                    log("recv - Could not receive message from server\n");
                    return -1;
                }

                if (!strncmp(message_from_server.topic, "stop", 4)) {
                    forever = false;
                    break;
                }
                
                strncpy(buf, message_from_server.topic, MAX_TOPIC_LEN);
                buf[MAX_TOPIC_LEN] = '\0';
                printf("%s:%u - %s - %s - %s\n",
                       message_from_server.from_ip,
                       message_from_server.from_port,
                       buf,
                       msg_type_to_string(message_from_server.message_type).c_str(),
                       message_from_server.payload);
            }
        }

        if (events_len == num_events) {
            // Try to double it
            struct epoll_event *new_events;
            new_events = (epoll_event*)realloc(events, events_len * 2);
            // If attempt failed, log but IGNORE!
            if (!new_events) {
                log("realloc - Could not double events. Ignoring...\n");
            } else {
                events = new_events;
                events_len *= 2;
            }
        }
    }

    // - - - - -

    close_client(0);
    return 0; // unreachable
}
