// Copyright Valentin-Ioan VINTILĂ 2023.
// All rights reserved.

// TODO - update to something else
#include <bits/stdc++.h>

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

// This routine is called once a TCP client is connected
void tcp_client_connected(string user_id) {
    // if (!subscriber_queues[user_id].empty()) {
    //     // Each message should be sent
    // }
}

int main(int argc, char *argv[]) {
    // Disable stdout buffer
    setvbuf(stdout, NULL, _IONBF, BUFSIZ);

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
        return -1;
    }
    // Add UDP listen
    event.data.fd = udp_listen_fd;
    event.events = EPOLLIN | EPOLLET;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, udp_listen_fd, &event) < 0) {
        log("epoll_ctl - Could not add udp_listen_fd");
        return -1;
    }
    // Add TCP listen
    event.data.fd = tcp_listen_fd;
    event.events = EPOLLIN | EPOLLET;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, tcp_listen_fd, &event) < 0) {
        log("epoll_ctl - Could not add udp_listen_fd");
        return -1;
    }
    log("Added stdin + UDP + TCP to poll\n");

    // - - - - -

    unordered_set<int> connected_tcp_clients;

    while (1) {
        // int activity = poll(poll_fds.elems, poll_fds.size(), -1);
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
                    if (!strncmp(buf, "exit", 4))
                        break;
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

                // Check if this connection is valid
                // TODO

                // Add new connection
                event.data.fd = new_client_fd;
                event.events = EPOLLIN | EPOLLET;
                if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, new_client_fd, &event) < 0) {
                    log("epoll_ctl - Could not add new_client_fd (%d)", new_client_fd);
                    return -1;
                }
                connected_tcp_clients.insert(new_client_fd);
                // TODO - add in subscriber_with_id
                log("Successful addition of new TCP connection (%s %d)\n",
                    inet_ntoa(new_client_addr.sin_addr), ntohs(new_client_addr.sin_port));
            }

            // - - - - -
            
            // Check for new UDP message
            else if (events[i].data.fd == udp_listen_fd) {
                udp_message_t recv_udp_msg;

                rc = recvfrom(udp_listen_fd, &recv_udp_msg, sizeof(struct udp_message_t),
                              0, (sockaddr *) &udp_addr, &sock_len);
                if (rc < 0) {
                    log("recvfrom - Could not receive UDP message\n");
                    return -1;
                }
                tcp_message_t new_tcp_msg = recv_udp_msg.to_tcp();
                new_tcp_msg.set_from(udp_addr);
                
                if (!new_tcp_msg.check_valid())
                    continue; // Might need to throw
                
                // Check if anyone subscribed
                if (subscribers_of.find(recv_udp_msg.topic) == subscribers_of.end())
                    continue;

                for (string subscriber_id : subscribers_of[recv_udp_msg.topic]) {
                    // Check if said subscriber is even online
                    if (subscriber_with_id[subscriber_id].online_as >= 0) {
                        rc = send(
                            subscriber_with_id[subscriber_id].online_as,
                            &new_tcp_msg,
                            sizeof(tcp_message_t),
                            0
                        );
                        if (rc < 0) {
                            log("send - Could not send topic info to subscriber\n");
                            return -1;
                        }
                    } else if (subscriber_with_id[subscriber_id].subscriptions[recv_udp_msg.topic]) {
                        subscriber_with_id[subscriber_id].to_send.push(new_tcp_msg);
                    }
                }
            }

            // - - - - -
            
            // Check for TCP input
            else if (connected_tcp_clients.find(events[i].data.fd) != connected_tcp_clients.end()) {
                rc = recv(events[i].data.fd, &buf, sizeof(buf), 0);
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
                    memcpy(&message, &buf, sizeof(message));
                    string id = string(message.unique_id);

                    if (!strncmp(message.command, "exit", 4))
                        break;
                    
                    if (!strncmp(message.command, "subscribe", 9)) {
                        subscribers_of[message.topic].insert(id);
                        subscriber_with_id[id].subscriptions[message.topic]
                            = message.store_and_forward;
                    } else if (!strncmp(message.command, "unsubscribe", 11)) {
                        subscribers_of[message.topic].erase(id);
                    }
                }
            }
        }
    }

    // - - - - -

    for (auto client : connected_tcp_clients) {
        tcp_message_t stop_message;
        memset(stop_message.topic, 0, MAX_TOPIC_LEN);
        strcpy(stop_message.topic, "stop");
        rc = send(client, (char *) &stop_message, sizeof(stop_message), 0);
        if (rc < 0) {
            log("send - Could not send \"stop\" signal to TCP client %d\n", client);
            return -1;
        }
    }

    log("All OK\n");
    return 0;
}
