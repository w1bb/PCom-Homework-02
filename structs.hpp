#ifndef _WI_STRUCTS_HPP_
#define _WI_STRUCTS_HPP_

#include <cstdint>
#include <cstring>

#include "net_includes.hpp"
#include "utils.hpp"

#define MAX_IP_LEN 16 // At most 255.255.255.255 (15 chars + 1)
#define MAX_PAYLOAD_LEN 1600
#define MAX_TOPIC_LEN 50

using namespace std;


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
    void set_from(struct sockaddr_in udp_addr);
};


struct udp_message_t {
    uint8_t message_type;

    tcp_message_t to_tcp();
};

#endif // _WI_STRUCTS_HPP_
