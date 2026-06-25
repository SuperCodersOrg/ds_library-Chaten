#ifndef LIST_H
#define LIST_H

#include<initializer_list>
#include<cstddef>
#include<stdexcept>

template<typename T>
class List{
    public:
        struct Node{
            T data;
            Node* next;
            Node* prev;
            Node(const T& data): data(data), next(nullptr), prev(nullptr) {}
        };
    private:
        Node* head;
        Node* tail;
        size_t listSize;

        void copyFrom(const List& other);
        Node* getNodeAt(size_t index) const;
    public:
        List();
        List(const List& other);
        List& operator=(const List& other);
        ~List() noexcept;

        List(std::initializer_list<T> initList);

        void push_back(const T& value);
        void push_front(const T& value);
        void insert(size_t index, const T& value);

        void remove(size_t index);
        void pop_back();
        void pop_front();

        T& front();
        const T& front() const;

        T& back();
        const T& back() const;

        T& at(size_t index);
        const T& at(size_t index) const;

        bool empty() const noexcept;
        size_t size() const noexcept;
        void clear();

        bool contains(const T& value) const;
        Node* find(const T& value) const;
        void reverse();
};

#include "../src/List.cpp"

#endif