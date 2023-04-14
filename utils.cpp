// Copyright Valentin-Ioan VINTILĂ 2023.
// All rights reserved.

#include "utils.hpp"

vector<string> split_command(string buf) {
    vector<string> rez;
    stringstream ss(buf);
    while (ss >> buf)
        rez.push_back(buf);
    return rez;
}
