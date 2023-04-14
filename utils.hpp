// Copyright Valentin-Ioan VINTILĂ 2023.
// All rights reserved.

#ifndef _WI_UTILS_HPP_
#define _WI_UTILS_HPP_

#include <cstdio>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <vector>
#include <string>

using namespace std;

// Logging capabilities
#ifdef LOG_ENABLE
#define log(args...) \
    do { \
        using namespace std::chrono; \
        auto now = system_clock::now(); \
        auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000; \
        auto timer = system_clock::to_time_t(now); \
        tm bt = *std::localtime(&timer); \
        ostringstream oss; oss << std::put_time(&bt, "%H:%M:%S"); \
        fprintf(stderr, "[ log ] %s:%d (%s) -- ", __FILE__, __LINE__, oss.str().c_str()); \
        fprintf(stderr, args); \
    } while (0)
#else
#define log(args...)
#endif

#endif // _WI_UTILS_HPP_
