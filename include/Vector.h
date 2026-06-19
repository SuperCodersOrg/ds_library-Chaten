#ifndef VECTOR_H
#define VECTOR_H

#include <initializer_list>
#include <stdexcept>
#include <cstddef>

template<typename T>
class Vector{
private:
    T* data;
    size_t currentSize;
    size_t currentCapacity;

    void resize();

public:
    Vector();
    Vector(const Vector& other);
    Vector& operator=(const Vector& other);
    ~Vector();

    Vector(std::initializer_list<T> initList);

    void push_back(const T& value);
    void pop_back();

    void insert(size_t index, const T& value);
    void remove(size_t index);

    void clear();

    void reserve(size_t newCapacity);
    void shrink_to_fit();

    T& operator[](size_t index);
    const T& operator[](size_t index) const;

    T& at(size_t index);
    const T& at(size_t index) const;

    T& front();
    const T& front() const;

    T& back();
    const T& back() const;
    bool empty() const;

    size_t size() const;
    size_t capacity() const;
    size_t find(const T& value) const;

    bool contains(const T& value) const;
};

#include "../src/Vector.cpp"

#endif