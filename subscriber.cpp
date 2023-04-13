// Copyright Valentin-Ioan VINTILĂ 2023.
// All rights reserved.

#include <stdio.h>

#include "utils.hpp"

int main() {
    setvbuf(stdout, NULL, _IONBF, BUFSIZ);

    return 0;
}
