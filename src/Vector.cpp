#include <iostream>
#include <stdexcept>
#include <initializer_list>
#include "Vector.h"

template<typename T>
Vector<T>::Vector(){
    currentSize = 0;
    currentCapacity = 1;
    data = new T[currentCapacity];
}

template<typename T>
Vector<T>::~Vector(){
    delete[] data;
}

template<typename T>
Vector<T>::Vector(const Vector& other){
    currentSize = other.currentSize;
    currentCapacity = other.currentCapacity;
    data = new T[currentCapacity];

    for(size_t i=0; i<currentSize; ++i){
        data[i] = other.data[i];
    }
}

template<typename T>
Vector<T>& Vector<T>::operator=(const Vector& other){
    if(this != &other){
        delete []data;

        currentSize = other.currentSize;
        currentCapacity = other.currentCapacity;
        data = new T[currentCapacity];

        for(size_t i=0; i<currentSize; ++i){
            data[i] = other.data[i];
        }
    }
    return *this;
}

template<typename T>
Vector<T>::Vector(std::initializer_list<T> initList){
    currentSize = initList.size();
    currentCapacity = currentSize > 0 ? currentSize : 1;
    data = new T[currentCapacity];

    size_t index = 0;
    for(const T& value: initList) {
        data[index++] = value;
    }
}

template<typename T>
void Vector<T>::resize(){
    currentCapacity *=2;
    T* newData = new T[currentCapacity];
    for(size_t i=0; i<currentSize; ++i){
        newData[i] = data[i];
    }
    delete[] data;
    data = newData;
}

template<typename T>
void Vector<T>::push_back(const T& value) {
    if(currentSize >= currentCapacity) {
        // std::cout << "Array full, resizing..." << std::endl;
        resize();
    }
    data[currentSize++] = value;
}

template<typename T>
void Vector<T>::pop_back(){
    if(currentSize == 0){
        throw std::out_of_range("Vector is empty");
    }
    // if(currentSize <= currentCapacity / 4 && currentCapacity > 1) {
    //     currentCapacity /= 2;
    //     T* newData = new T[currentCapacity];
    //     for(size_t i=0; i<currentSize-1; ++i){
    //         newData[i] = data[i];
    //     }
    //     delete[] data;
    //     data = newData;
    // }
    --currentSize;
}

template<typename T>
void Vector<T>::insert(size_t index, const T& value){
    if(index > currentSize) {
        throw std::out_of_range("Index out of range");
    }
    if(currentSize >= currentCapacity) {
        resize();
    }
    for(size_t i=currentSize; i>index; --i){
        data[i] = data[i-1];
    }
    data[index] = value;
    ++currentSize;
}

template<typename T>
void Vector<T>::remove(size_t index){
    if(index>=currentSize){
        throw std::out_of_range("Index out of range");
    }
    for(size_t i=index; i<currentSize-1; ++i){
        data[i] = data[i+1];
    }
    --currentSize;
}

template<typename T>
void Vector<T>::clear(){
    currentSize=  0;
}

template<typename T>
void Vector<T>::reserve(size_t newCapacity){
    if(newCapacity > currentCapacity){
        currentCapacity = newCapacity;
        T* newData = new T[currentCapacity];
        for(size_t i=0; i<currentSize; ++i){
            newData[i] =data[i];
        }
        delete[] data;
        data = newData;
    } else if(newCapacity < currentSize){
        throw std::invalid_argument("New capacity cannot be less than current size");
    }
}

template<typename T>
void Vector<T>::shrink_to_fit(){
    if(currentCapacity > currentSize){
        currentCapacity = currentSize;
        T* newData = new T[currentCapacity];
        for(size_t i=0; i<currentSize; ++i){
            newData[i] = data[i];
        }
        delete[] data;
        data = newData;
    }
}

template<typename T>
T& Vector<T>::operator[](size_t index){
    return data[index];
}

template<typename T>
const T& Vector<T>::operator[](size_t index) const{
    return data[index];
}

template<typename T>
T& Vector<T>::at(size_t index){
    if(index >= currentSize){
        throw std::out_of_range("Index out of range");
    }
    return data[index];
}

template<typename T>
const T& Vector<T>::at(size_t index) const{
    if(index >= currentSize){
        throw std::out_of_range("Index out of range");
    }
    return data[index];
}

template<typename T>
T& Vector<T>::front(){
    if(currentSize == 0){
        throw std::out_of_range("Vector is empty");
    }
    return data[0];
}

template<typename T>
const T& Vector<T>::front() const{
    if(currentSize == 0){
        throw std::out_of_range("Vector is empty");
    }
    return data[0];
}

template<typename T>
T& Vector<T>::back(){
    if(currentSize == 0){
        throw std::out_of_range("Vector is empty");
    }
    return data[currentSize-1];
}

template<typename T>
const T& Vector<T>::back() const{
    if(currentSize == 0){
        throw std::out_of_range("Vector is empty");
    }
    return data[currentSize-1];
}

template<typename T>
bool Vector<T>::empty() const{
    return currentSize == 0;
}

template<typename T>
size_t Vector<T>::size() const {
    return currentSize;
}

template<typename T>
size_t Vector<T>::capacity() const {
    return currentCapacity;
}

template<typename T>
size_t Vector<T>::find(const T& value) const {
    for(size_t i=0; i<currentSize; ++i){
        if(data[i] == value) {
            return i;
        }
    }
    return currentSize;
}

template<typename T>
bool Vector<T>::contains(const T& value) const {
    return find(value) != currentSize;
}

