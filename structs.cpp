#include "structs.hpp"

void tcp_message_t::set_from(struct sockaddr_in udp_addr) {
    strncpy(this->from_ip, inet_ntoa(udp_addr.sin_addr), MAX_IP_LEN);
    this->from_port = ntohs(udp_addr.sin_port);
}

tcp_message_t udp_message_t::to_tcp() {
    tcp_message_t tcp_message;
    
    switch (this->message_type) {
    default:
        log("[ udp_message_t::to_tcp() ] Invalid message_type (%u)", this->message_type);
    }

    return tcp_message;
}
