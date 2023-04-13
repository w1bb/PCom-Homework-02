#ifndef _WI_DYNAMIC_ARRAY_HPP_
#define _WI_DYNAMIC_ARRAY_HPP_

#include <cstdint>
#include <cstdlib>

template <typename T>
struct dynamic_array {
    T* elems;

    dynamic_array();
    ~dynamic_array();

    uint32_t size();
    void pushBack(const T& elem);
    void remove(uint32_t i);

private:
    uint32_t current_size;
    uint32_t capacity;
    const uint32_t initial_capacity = 4;
    const uint32_t multiplier = 2;
    const uint32_t threshold = 2;

    void multiply_capacity();
};

// - - - - -

template<typename T>
dynamic_array<T>::dynamic_array() {
    this->capacity = initial_capacity;
    this->elems = static_cast<T*>(malloc(this->capacity * sizeof(T)));
    this->current_size = 0;
}

template<typename T>
dynamic_array<T>::~dynamic_array() {
    free(this->elems);
}

template<typename T>
uint32_t dynamic_array<T>::size() {
    return this->current_size;
}

template<typename T>
void dynamic_array<T>::pushBack(const T& elem) {
    while (this->current_size * this->threshold >= this->capacity) {
        this->capacity *= this->multiplier;
        this->elems = static_cast<T*>(realloc(elems, this->capacity * sizeof(T)));
        if (!this->elems) {
            // TODO - throw
        }
    }
    this->elems[this->current_size] = elem;
    ++(this->current_size);
}

template<typename T>
void dynamic_array<T>::remove(uint32_t i) {
    if (i >= this->current_size) {
        // TODO - throw
    }
    --(this->current_size);
    elems[i] = elems[this->current_size];
}

#endif // _WI_DYNAMIC_ARRAY_HPP_
