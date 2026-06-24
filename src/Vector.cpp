#include <stdexcept>
#include <initializer_list>
#include <new>
#include "Vector.h"
#include <type_traits>
#include <cstring>
#include <utility>

template<typename T>
Vector<T>::Vector(){
    currentSize = 0;
    currentCapacity = 1;
    data = static_cast<T*>(malloc(currentCapacity * sizeof(T)));

    if(data == nullptr){
        throw std::bad_alloc();
    }
}

template<typename T>
Vector<T>::~Vector() noexcept {
    if(data != nullptr) {
        if constexpr (!std::is_trivially_destructible<T>::value) {
            for(size_t i=0; i<currentSize; ++i){
            data[i].~T();
            }
        }
        free(data);
        data = nullptr;
        currentSize = 0;
        currentCapacity = 0;
    }
}

template<typename T>
Vector<T>::Vector(const Vector& other){
    currentSize = other.currentSize;
    currentCapacity = currentSize > 0 ? currentSize : 1;

    data = static_cast<T*>(malloc(currentCapacity * sizeof(T)));

    if(data == nullptr) {
        throw std::bad_alloc();
    }

    if constexpr (std::is_trivially_copyable<T>::value){
        std::memcpy(data, other.data, currentSize*sizeof(T));
    } else {
        size_t constructedCount = 0;
        try {
            for(size_t i=0; i<currentSize; ++i){
                new(&data[i]) T(other.data[i]);
                constructedCount++;
            }
        } catch (...) {
            for(size_t i=0; i<constructedCount; i++){
                data[i].~T();
            }
            free(data);

            throw;
        }
    }
}

template<typename T>
Vector<T>& Vector<T>::operator=(const Vector& other){
    if(this!=&other){
        if(currentCapacity >= other.currentSize){
            if constexpr (!std::is_trivially_destructible<T>::value) {
                for(size_t i=0; i<currentSize; ++i){
                    data[i].~T();
                }
            }

            currentSize = other.currentSize;
            if constexpr (std::is_trivially_copyable<T>::value) {
                std::memcpy(data, other.data, currentSize * sizeof(T));
            } else {
                size_t constructedCount = 0;
                try {
                    for(size_t i=0; i<currentSize; ++i){
                        new(&data[i]) T(other.data[i]);
                        constructedCount++;
                    }
                } catch (...) {
                    for(size_t i=0; i<constructedCount; ++i) {
                        data[i].~T();
                    }
                    currentSize = 0;
                    throw;
                }
            }
         } else {
            size_t newCapacity = other.currentSize > 0 ? other.currentSize : 1;
            T* newData = static_cast<T*>(malloc(newCapacity * sizeof(T)));
            if(newData == nullptr) {
                throw std::bad_alloc();
            }

            if constexpr (std::is_trivially_copyable<T>::value){
                std::memcpy(newData, other.data, other.currentSize * sizeof(T));
            } else {
                size_t constructedCount = 0;
                try {
                    for(size_t i=0; i<other.currentSize; i++){
                        new(&newData[i]) T(other.data[i]);
                        constructedCount++;
                    }
                } catch (...) {
                    for(size_t i=0; i<constructedCount; ++i) {
                        newData[i].~T();
                    }
                    free(newData);
                    throw;
                }
            }
            if constexpr (!std::is_trivially_destructible<T>::value) {
                for(size_t i=0; i<currentSize; ++i) {
                    data[i].~T();
                }
            }
            free(data);

            data = newData;
            currentSize = other.currentSize;
            currentCapacity = newCapacity;
        }
    }
    return *this;
}

template<typename T>
Vector<T>::Vector(std::initializer_list<T> initList){
    currentSize = initList.size();
    currentCapacity = currentSize > 0 ? currentSize : 1;
    data = static_cast<T*>(malloc(currentCapacity * sizeof(T)));

    if(data == nullptr) {
        throw std::bad_alloc();
    }

    if constexpr (std::is_trivially_copyable<T>::value) {
        std::memcpy(data, initList.begin(), currentSize * sizeof(T));
    } else {
        size_t constructedCount = 0;
        try {
            for(const T& value: initList) {
                new(&data[constructedCount]) T(value);
                constructedCount++;
            }
        } catch (...) {
            for(size_t i=0; i<constructedCount; i++) {
                data[i].~T();
            }
            free(data);
            throw;
        }
    }
}

template<typename T>
void Vector<T>::resize(){
    currentCapacity *= 2;
    T* newData = static_cast<T*>(malloc(currentCapacity * sizeof(T)));

    if(newData == nullptr) {
        throw std::bad_alloc();
    }
    if constexpr (std::is_trivially_copyable<T>::value){
        if(currentSize > 0) {
            std::memcpy(newData, data, currentSize * sizeof(T));
        }
    } else {
        size_t constructedCount = 0;
        try {
            for(size_t i=0; i<currentSize; i++){
                new(&newData[i]) T(std::move_if_noexcept(data[i]));
                constructedCount++;
            }
        } catch (...) {
            for(size_t i=0; i<constructedCount; i++){
                newData[i].~T();
            }
            free(newData);
            throw;
        }
        for(size_t i=0; i<currentSize; i++){
            data[i].~T();
        }
    }
    free(data);
    data = newData;
}

template<typename T>
void Vector<T>::push_back(const T& value) {
    if(currentSize >= currentCapacity) {
        resize();
    }
    new(&data[currentSize]) T(value);
    ++currentSize;
}

template<typename T>
void Vector<T>::pop_back(){
    if(currentSize == 0){
        throw std::out_of_range("Vector is empty");
    }
    if constexpr (!std::is_trivially_destructible<T>::value) {
        data[currentSize-1].~T();
    }
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

    if(index == currentSize) {
        new(&data[currentSize]) T(value);
    } else {
        new(&data[currentSize]) T(std::move(data[currentSize-1]));
        for(size_t i=currentSize-1; i>index; --i) {
            data[i] = std::move(data[i-1]);
        }
        data[index] = value;
    }
    ++currentSize;
}

template<typename T>
void Vector<T>::remove(size_t index){
    if(index>=currentSize){
        throw std::out_of_range("Index out of range");
    }
    for(size_t i=index; i<currentSize-1; ++i){
        data[i] = std::move(data[i+1]);
    }
    if constexpr (!std::is_trivially_destructible<T>::value) {
        data[currentSize - 1].~T();
    }
    --currentSize;
}

template<typename T>
void Vector<T>::clear(){
    if constexpr(!std::is_trivially_destructible<T>::value){
        for(size_t i=0; i<currentSize; i++){
            data[i].~T();
        }
    }
    currentSize = 0;
}

template<typename T>
void Vector<T>::reserve(size_t newCapacity){
    if(newCapacity <= currentCapacity) return;
    T* newData = static_cast<T*>(malloc(newCapacity * sizeof(T)));

    if(newData == nullptr) {
        throw std::bad_alloc();
    }

    if constexpr (std::is_trivially_copyable<T>::value) {
        if(currentSize > 0) {
            std::memcpy(newData, data, currentSize * sizeof(T));
        }
    } else {
        size_t constructedCount = 0;
        try {
            for(size_t i=0; i<currentSize; ++i){
                new(&newData[i]) T(std::move_if_noexcept(data[i]));
                constructedCount++;
            }
        } catch (...) {
            for(size_t i=0; i<constructedCount; i++){
                newData[i].~T();
            }
            free(newData);
            throw;
        }
        for(size_t i=0; i<currentSize; i++){
            data[i].~T();
        }
    }
    free(data);
    data = newData;
    currentCapacity = newCapacity;
}

template<typename T>
void Vector<T>::shrink_to_fit(){
    if(currentCapacity > currentSize){
        size_t newCapacity = currentSize > 0 ? currentSize : 1;

        if (currentCapacity <= newCapacity) {
            return;
        }

        T* newData = static_cast<T*>(malloc(newCapacity * sizeof(T)));

        if (newData == nullptr) {
            throw std::bad_alloc();
        }

        if constexpr (std::is_trivially_copyable<T>::value) {
            if (currentSize > 0) {
                std::memcpy(newData, data, currentSize * sizeof(T));
            }
        }
        else {
            size_t constructedCount = 0;
            try {
                for(size_t i = 0; i < currentSize; ++i){
                    new(&newData[i]) T(std::move_if_noexcept(data[i]));
                    constructedCount++;
                }
            } catch (...) {
                for(size_t i = 0; i < constructedCount; ++i) {
                    newData[i].~T();
                }
                free(newData);
                throw;
            }
            for(size_t i = 0; i < currentSize; ++i){
                data[i].~T();
            }
        }

        free(data);
        data = newData;
        currentCapacity = newCapacity;
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