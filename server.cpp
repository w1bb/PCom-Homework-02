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

int main() {
    setvbuf(stdout, NULL, _IONBF, BUFSIZ);

    log("sizeof(tcp_message_t) = %lu\n", sizeof(tcp_message_t));

    return 0;
}
