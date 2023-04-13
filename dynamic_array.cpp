#include "dynamic_array.hpp"

template<typename T>
dynamic_array_t<T>::dynamic_array_t() {
    this->capacity = initial_capacity;
    this->elems = malloc(this->capacity * sizeof(T));
    this->current_size = 0;
}

template<typename T>
dynamic_array_t<T>::~dynamic_array_t() {
    free(this->elems);
}

template<typename T>
uint32_t dynamic_array_t<T>::size() {
    return this->current_size;
}

template<typename T>
void dynamic_array_t<T>::pushBack(T& elem) {
    while (this->current_size * this->threshold >= this->capacity) {
        this->capacity *= this->multiplier;
        this->elems = realloc(elems, this->capacity * sizeof(T));
        if (!this->elems) {
            // TODO - throw
        }
    }
    this->elems[this->current_size] = elem;
    ++(this->current_size);
}

template<typename T>
void dynamic_array_t<T>::remove(uint32_t i) {
    if (i >= this->current_size) {
        // TODO - throw
    }
    --(this->current_size);
    elems[i] = elems[this->current_size];
}
