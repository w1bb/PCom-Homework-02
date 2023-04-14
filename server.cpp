// Copyright Valentin-Ioan VINTILĂ 2023.
// All rights reserved.

// TODO - update to something else
#include <bits/stdc++.h>
#include <signal.h>

#include "dynamic_array.hpp"
#include "structs.hpp"
#include "utils.hpp"

using namespace std;

// Map between topic and IDs
unordered_map< string, set<string> > subscribers_of;

// Map between ID and subscriber_t
unordered_map<string, subscriber_t> subscriber_with_id;

// Map between fd and ID
unordered_map<int, string> id_with_fd;

// A set of all the connected subscribers
unordered_set<int> connected_tcp_clients;

void close_server(int sig) {
    log("Closing the server (sig = %d)...\n", sig);
    for (auto client : connected_tcp_clients) {
        tcp_message_t stop_message;
        memset(stop_message.topic, 0, MAX_TOPIC_LEN);
        strcpy(stop_message.topic, "stop");
        int rc = send(client, (char *) &stop_message, sizeof(stop_message), 0);
        if (rc < 0) {
            log("send - Could not send \"stop\" signal to TCP client %d\n", client);
            // No need to force stop now, just ignore
            // exit(EXIT_FAILURE);
        }
    }
    log("All OK\n");
    exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[]) {
    // Disable stdout buffer
    setvbuf(stdout, NULL, _IONBF, BUFSIZ);

    // Treat correctly Ctrl+C
    signal(SIGINT, close_server);

    int rc;
    char buf[2048]{};
    socklen_t sock_len = sizeof(struct sockaddr);

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
    memset((char *) &udp_addr, 0, sizeof(udp_addr));
	udp_addr.sin_family = AF_INET;
	udp_addr.sin_port = htons(server_port);
	udp_addr.sin_addr.s_addr = INADDR_ANY;
    int reuse_socket_asap = 1;
    rc = setsockopt(udp_listen_fd, SOL_SOCKET, SO_REUSEADDR,
                    &reuse_socket_asap , sizeof(int));
    if (rc < 0) {
        log("setsockopt - Could not enable fast reallocation\n");
        return -1;
    }
    // Bind
    rc = bind(udp_listen_fd, (struct sockaddr *) &udp_addr, sock_len);
	if (rc < 0) {
        log("bind - UDP bind fail\n");
        return -1;
    }
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
        log("setsockopt - Could not disable Neagle for server\n");
        return -1;
    }
    reuse_socket_asap = 1;
    rc = setsockopt(tcp_listen_fd, SOL_SOCKET, SO_REUSEADDR,
                    &reuse_socket_asap , sizeof(int));
    if (rc < 0) {
        log("setsockopt - Could not enable fast reallocation\n");
        return -1;
    }
    struct sockaddr_in tcp_addr;
    memset((char *) &tcp_addr, 0, sizeof(sockaddr_in));
	tcp_addr.sin_family = AF_INET;
	tcp_addr.sin_port = htons(server_port);
	tcp_addr.sin_addr.s_addr = INADDR_ANY;
    // Bind
    rc = bind(tcp_listen_fd, (struct sockaddr *) &tcp_addr, sock_len);
	if (rc < 0) {
        log("bind - TCP bind fail\n");
        return -1;
    }
    // Listen
    rc = listen(tcp_listen_fd, MAX_CLIENTS);
    if (rc < 0) {
        log("listen - TCP listen fail\n");
        return -1;
    }
    log("TCP OK\n");

    // - - - - -

    // Generate epoll
    struct epoll_event event, events[MAX_EPOLL_EVENTS];
    int epoll_fd = epoll_create1(0);
    // Add STDIN
    event.data.fd = STDIN_FILENO;
    event.events = EPOLLIN;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, STDIN_FILENO, &event) < 0) {
        log("epoll_ctl - Could not add STDIN_FILENO\n");
        return -1;
    }
    // Add UDP listen
    event.data.fd = udp_listen_fd;
    event.events = EPOLLIN;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, udp_listen_fd, &event) < 0) {
        log("epoll_ctl - Could not add udp_listen_fd\n");
        return -1;
    }
    // Add TCP listen
    event.data.fd = tcp_listen_fd;
    event.events = EPOLLIN;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, tcp_listen_fd, &event) < 0) {
        log("epoll_ctl - Could not add tcp_listen_fd\n");
        return -1;
    }
    log("Added stdin + UDP + TCP to poll\n");

    // - - - - -

    bool forever = true;

    while (forever) {
        int num_events = epoll_wait(epoll_fd, events, MAX_EPOLL_EVENTS, 0);
        for (int i = 0; i < num_events; ++i) {
            // if (!(events[i].events & EPOLLIN))
            //     continue;

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
                }
            }

            // - - - - -
            
            // Check for new TCP connection
            else if (events[i].data.fd == tcp_listen_fd) {
                struct sockaddr_in new_client_addr;
                socklen_t new_client_addr_len = sizeof(new_client_addr);
                int new_client_fd = accept(
                    tcp_listen_fd,
                    (struct sockaddr *)&new_client_addr,
                    &new_client_addr_len
                );
                if (new_client_fd < 0) {
                    log("accept - Could not accept new TCP connection\n");
                    return -1;
                }
                disable_neagle = 1;
                rc = setsockopt(new_client_fd, IPPROTO_TCP, TCP_NODELAY,
                                &disable_neagle, sizeof(int));
                if (rc < 0) {
                    log("setsockopt - Could not disable Neagle for client\n");
                    return -1;
                }

                // Get the expected ID
                memset(buf, 0, sizeof(buf));
                rc = recv(new_client_fd, buf, sizeof(buf), 0);
                if (rc < 0) {
                    log("recv - Could not receive TCP client initial message\n");
                    return -1;
                }
                string id = string(buf);

                // Check if the ID is already in use
                if (subscriber_with_id[id].online_as >= 0) {
                    printf("Client %s already connected.\n", id.c_str());
                    // Close connection instead
                    tcp_message_t stop_message;
                    memset(stop_message.topic, 0, MAX_TOPIC_LEN);
                    strcpy(stop_message.topic, "stop");
                    rc = send(new_client_fd, (char *) &stop_message, sizeof(stop_message), 0);
                    if (rc < 0) {
                        log("send - Could not send \"stop\" signal to TCP client %d\n", new_client_fd);
                        return -1;
                    }
                    continue;
                }

                // Add new connection
                event.data.fd = new_client_fd;
                event.events = EPOLLIN;
                if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, new_client_fd, &event) < 0) {
                    log("epoll_ctl - Could not add new_client_fd (%d)", new_client_fd);
                    return -1;
                }
                connected_tcp_clients.insert(new_client_fd);
                subscriber_with_id[id].online_as = new_client_fd;
                id_with_fd[new_client_fd] = id;
                log("Successful addition of new TCP connection (%s %d)\n",
                    inet_ntoa(new_client_addr.sin_addr), ntohs(new_client_addr.sin_port));
                printf("New client %s connected from %s:%u.\n",
                       id.c_str(),
                       inet_ntoa(new_client_addr.sin_addr),
                       ntohs(new_client_addr.sin_port));
            }

            // - - - - -
            
            // Check for new UDP message
            else if (events[i].data.fd == udp_listen_fd) {
                log("Received UDP message\n");
                udp_message_t recv_udp_msg;
                memset(&recv_udp_msg, 0, sizeof(recv_udp_msg));

                rc = recvfrom(udp_listen_fd, &recv_udp_msg, sizeof(recv_udp_msg),
                              0, (sockaddr *) &udp_addr, &sock_len);
                if (rc < 0) {
                    log("recvfrom - Could not receive UDP message\n");
                    return -1;
                }
                tcp_message_t new_tcp_msg = recv_udp_msg.to_tcp();
                // log("TCP payload set to '%s'\n", new_tcp_msg.payload);
                // log("TCP topic set to '%s'\n", new_tcp_msg.topic);
                
                new_tcp_msg.set_from(udp_addr);
                
                if (!new_tcp_msg.check_valid()) {
                    log("Invalid TCP message\n");
                    continue; // Might need to throw
                }
                
                // Check if anyone subscribed
                strncpy(buf, recv_udp_msg.topic, MAX_TOPIC_LEN);
                buf[sizeof(recv_udp_msg.topic)] = '\0';
                if (subscribers_of.find(buf) == subscribers_of.end()) {
                    // log("Note: no subscribers found!\n");
                    continue;
                }

                log("TCP payload set to '%s'\n", new_tcp_msg.payload);
                log("TCP topic set to '%s'\n", buf);

                for (string subscriber_id : subscribers_of[buf]) {
                    // Check if said subscriber is even online
                    if (subscriber_with_id[subscriber_id].online_as >= 0) {
                        log("Sending NOW to %s\n", subscriber_id.c_str());
                        rc = send(
                            subscriber_with_id[subscriber_id].online_as,
                            &new_tcp_msg,
                            sizeof(new_tcp_msg),
                            0
                        );
                        if (rc < 0) {
                            log("send - Could not send topic info to subscriber\n");
                            return -1;
                        }
                    } else if (subscriber_with_id[subscriber_id].subscriptions[buf]) {
                        log("Sending LATER to %s\n", subscriber_id.c_str());
                        subscriber_with_id[subscriber_id].to_send.push(new_tcp_msg);
                    }
                }
            }

            // - - - - -
            
            // Check for TCP input
            else if (connected_tcp_clients.find(events[i].data.fd) != connected_tcp_clients.end()) {
                memset(buf, 0, sizeof(buf));
                rc = recv(events[i].data.fd, buf, sizeof(buf), 0);
                if (rc < 0) {
                    log("recv - Could not receive TCP input\n");
                    return -1;
                }
                
                if (rc == 0) {
                    // Disconnect client
                    string id = id_with_fd[events[i].data.fd];
                    printf("Client %s disconnected.\n", id.c_str());
                    subscriber_with_id[id].online_as = -1;
                    id_with_fd[events[i].data.fd] = -1;
                    if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, nullptr) < 0) {
                        log("epoll_ctl - Could not remove connection\n");
                        return -1;
                    }
                    close(events[i].data.fd);
                } else {
                    message_from_tcp_t message;
                    memcpy(&message, buf, sizeof(message));
                    string id = string(message.unique_id);

                    if (!strncmp(message.command, "exit", 4))
                        break; // TODO - this might be useless
                    
                    if (!strncmp(message.command, "subscribe", 9)) {
                        strncpy(buf, message.topic, MAX_TOPIC_LEN);
                        buf[sizeof(message.topic)] = '\0';
                        log("Subscribing %s to '%s'\n", id.c_str(), buf);
                        subscribers_of[buf].insert(id);
                        subscriber_with_id[id].subscriptions[buf]
                            = message.store_and_forward;
                    } else if (!strncmp(message.command, "unsubscribe", 11)) {
                        subscribers_of[buf].erase(id);
                    }
                }
            }

            // - - - - -

            else {
                log("How is this even possible?\n");
            }
        }
    }

    // - - - - -

    close_server(0);
    return 0; // unreachable
}
