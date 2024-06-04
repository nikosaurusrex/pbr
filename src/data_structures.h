#pragma once

#include <assert.h>
#include <stdlib.h>

#include <initializer_list>
#include <set>
#include <unordered_map>
#include <vector>

template <typename T> struct Array {
    T     *data     = 0;
    size_t count    = 0;
    size_t capacity = 0;

    Array() = default;
    Array(std::initializer_list<T> init_list)
    {
        array_reserve(this, init_list.size());

        for (const auto &element : init_list) {
            data[count++] = element;
        }
    }
    
    // Sets the initial size of the array, not the capacity!
    Array(uint32_t count)
    {
        array_resize(this, count);
    }

    ~Array()
    {
        array_reset(this);
    }

    T &
    operator[](int index)
    {
        assert(index >= 0 && index < count);
        return data[index];
    }
};

template <typename T>
static inline void
array_reserve(Array<T> *array, size_t amount)
{
    if (amount <= 0)
        amount = 16;
    if (amount <= array->capacity)
        return;

    T *new_mem = (T *)malloc(amount * sizeof(T));

    if (array->data) {
        memcpy(new_mem, array->data, array->count * sizeof(T));
        free((void *)array->data);
    }

    array->data     = new_mem;
    array->capacity = amount;
}

template <typename T>
static inline void
array_resize(Array<T> *array, size_t amount)
{
    array_reserve(array, amount);
    array->count = amount;
}

template <typename T>
static inline void
array_append(Array<T> *array, T value)
{
    if (array->count + 1 >= array->capacity)
        array_reserve(array, array->capacity * 2);

    array->data[array->count++] = value;
}

template <typename T>
static inline T
array_pop(Array<T> *array)
{
    assert(array->count > 0);
    T result = array->data[array->count - 1];
    array->count--;
    return result;
}

template <typename T>
static inline void
array_unordered_remove(Array<T> *array, size_t index)
{
    assert(index >= 0 && index < array->count);

    T last = array_pop(array);
    if (index < array->count) {
        array->data[index] = last;
    }

    return last;
}

template <typename T>
static inline void
array_ordered_remove(Array<T> *array, size_t index)
{
    assert(index >= 0 && index < array->count);

    T item = array->data[index];
    memmove(array->data + index, array->data + index + 1, ((array->count - index) - 1) * sizeof(T));

    array->count--;
    return item;
}

template <typename T>
static inline void
array_clear(Array<T> *array)
{
    array->count = 0;
}

template <typename T>
static inline void
array_reset(Array<T> *array)
{
    array->count    = 0;
    array->capacity = 0;

    if (array->data)
        free((void *)array->data);
    array->data = 0;
}

struct Set {
};

struct HashMap {
};