#ifndef _WI_DYNAMIC_ARRAY_HPP_
#define _WI_DYNAMIC_ARRAY_HPP_

#include <cstdint>
#include <cstdlib>

template <typename T>
struct dynamic_array_t {
    T* elems;

    dynamic_array_t();
    ~dynamic_array_t();

    uint32_t size();
    void pushBack(T& elem);
    void remove(uint32_t i);

private:
    uint32_t current_size;
    uint32_t capacity;
    const uint32_t initial_capacity = 4;
    const uint32_t multiplier = 2;
    const uint32_t threshold = 2;

    void multiply_capacity();
};

#endif // _WI_DYNAMIC_ARRAY_HPP_
