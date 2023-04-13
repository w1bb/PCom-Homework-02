// Copyright Valentin-Ioan VINTILĂ 2023.
// All rights reserved.

#ifndef _WI_STRUCTS_HPP_
#define _WI_STRUCTS_HPP_

#include <cstdint>
#include <cstring>
#include <string>
#include <cmath>
#include <queue>
#include <map>

#include "net_includes.hpp"
#include "utils.hpp"

using namespace std;

// - - - - -

#define MAX_IP_LEN 16        // At most 255.255.255.255 (15 chars + 1)
#define MAX_PAYLOAD_LEN 1500
#define MAX_TOPIC_LEN 50
#define MAX_EPOLL_EVENTS 64
#define MAX_CLIENT_ID_SIZE 11
#define MAX_CMD_SIZE 12

// - - - - -

#define MSG_TYPE_INT 0
#define MSG_TYPE_SHORT_REAL 1
#define MSG_TYPE_FLOAT 2
#define MSG_TYPE_STRING 3

string msg_type_to_string(uint8_t msg_type);

// - - - - -

struct tcp_message_t {
    // Details about message's content
    uint8_t message_type;
    char topic[MAX_TOPIC_LEN];
    char payload[MAX_PAYLOAD_LEN];

    // Details about message's origin
    char from_ip[MAX_IP_LEN];
    uint16_t from_port;

    // - - - - - - - - - - - - - - - - - - - -

    // Provide information for origin
    void set_from(struct sockaddr_in& udp_addr);

    // Check validity
    bool check_valid();
};

// - - - - -

struct udp_message_t {
	char topic[MAX_TOPIC_LEN];
    uint8_t message_type;
    char payload[MAX_PAYLOAD_LEN];

    tcp_message_t to_tcp();
};

// - - - - -

struct subscriber_t {
    // string unique_id;
    queue<tcp_message_t> to_send;
    int online_as;
    map<string, bool> subscriptions;

    subscriber_t();
};

// - - - - -

struct message_from_tcp_t {
    char command[MAX_CMD_SIZE];
    char topic[MAX_TOPIC_LEN];
    char unique_id[MAX_CLIENT_ID_SIZE];
    int8_t store_and_forward;
} __attribute__((packed, aligned(1)));

#endif // _WI_STRUCTS_HPP_
