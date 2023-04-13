// Copyright Valentin-Ioan VINTILĂ 2023.
// All rights reserved.

#ifndef _WI_UTILS_HPP_
#define _WI_UTILS_HPP_

#include <cstdio>

// Logging capabilities
#ifdef LOG_ENABLE
#define log(args...) \
    do { \
        fprintf(stderr, "[ log ] %s:%d (%s) -- ", __FILE__, __LINE__, __TIME__); \
        fprintf(stderr, args); \
    } while (0)
#else
#define log(args...)
#endif

#endif // _WI_UTILS_HPP_
