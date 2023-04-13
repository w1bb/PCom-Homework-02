#include "structs.hpp"

string msg_type_to_string(uint8_t msg_type) {
    if (msg_type == MSG_TYPE_INT)
        return "INT";
    if (msg_type == MSG_TYPE_SHORT_REAL)
        return "SHORT_REAL";
    if (msg_type == MSG_TYPE_FLOAT)
        return "FLOAT";
    if (msg_type == MSG_TYPE_STRING)
        return "STRING";
    return "";
}

void tcp_message_t::set_from(struct sockaddr_in& udp_addr) {
    strncpy(this->from_ip, inet_ntoa(udp_addr.sin_addr), MAX_IP_LEN);
    this->from_port = ntohs(udp_addr.sin_port);
}

bool tcp_message_t::check_valid() {
    return (this->message_type == MSG_TYPE_INT) ||
           (this->message_type == MSG_TYPE_SHORT_REAL) ||
           (this->message_type == MSG_TYPE_FLOAT) ||
           (this->message_type == MSG_TYPE_STRING);
}

tcp_message_t udp_message_t::to_tcp() {
    tcp_message_t tcp_message;
    
    if (this->message_type == MSG_TYPE_INT) {
        // Find sign
        bool is_negative = (this->payload[0] == 1);
        // Get the number
        uint32_t number;
        memcpy(&number, this->payload + 1, sizeof(uint32_t));
        number = ntohl(number);
        // Save the number
        if (is_negative)
            sprintf(tcp_message.payload, "-%u", number);
        else
            sprintf(tcp_message.payload, "%u", number);
    } else if (this->message_type == MSG_TYPE_SHORT_REAL) {
        // Get the original number
        uint16_t number;
        memcpy(&number, this->payload, sizeof(uint16_t));
        number = ntohs(number);
        // Check if there are decimal points and output correspondingly
        if (number % 100u)
            sprintf(tcp_message.payload, "%u.%02u", number / 100u, number % 100u);
        else
            sprintf(tcp_message.payload, "%u", number / 100u);
    } else if (this->message_type == MSG_TYPE_FLOAT) {
        // Find sign
        bool is_negative = (this->payload[0] == 1);
        // Get the original number
        uint32_t number;
        memcpy(&number, this->payload + 1, sizeof(uint32_t));
        number = ntohl(number);
        // Convert the number into float
        double d = (double)number / pow(10, this->payload[1 + sizeof(uint32_t)]);
        // Save the number
        if (is_negative)
            sprintf(tcp_message.payload, "-%lf", d);
        else
            sprintf(tcp_message.payload, "%lf", d);
    } else if (this->message_type == MSG_TYPE_STRING) {
        size_t payload_len = strlen(this->payload);
        strncpy(tcp_message.payload, this->payload, payload_len);
    } else {
        log("[ udp_message_t::to_tcp() ] Invalid message_type (%u)", this->message_type);
    }

    tcp_message.message_type = this->message_type;
    return tcp_message;
}

subscriber_t::subscriber_t() {
    this->online_as = -1;
}
