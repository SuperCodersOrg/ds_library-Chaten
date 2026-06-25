#include "List.h"
#include <utility>
#include <stdexcept>

template<typename T>
List<T>::List() : head(nullptr), tail(nullptr), listSize(0) {}

template<typename T>
List<T>::List(std::initializer_list<T> initList): head(nullptr), tail(nullptr), listSize(0) {
    for(const T& value: initList){
        push_back(value);
    }
}

template<typename T>
List<T>::List(const List& other): head(nullptr), tail(nullptr), listSize(0) {
    copyFrom(other);
}

template<typename T>
List<T>& List<T>::operator=(const List& other){
    if(this != &other){
        clear();
        copyFrom(other);
    }
    return *this;
}

template<typename T>
List<T>::~List() noexcept {
    clear();
}

template<typename T>
void List<T>::copyFrom(const List& other){
    Node* current = other.head;
    while(current){
        push_back(current->data);
        current=current->next;
    }
}

template<typename T>
typename List<T>::Node* List<T>::getNodeAt(size_t index) const {
    Node* current;
    if(index < listSize/2){
        current = head;
        for(size_t i=0; i<index; ++i){
            current=current->next;
        }
    } else {
        current = tail;
        for(size_t i=listSize-1; i>index; --i){
            current=current->prev;
        }
    }
    return current;
}

template<typename T>
void List<T>::push_front(const T& value){
    Node* newNode = new Node(value);
    if(empty()){
        head = tail = newNode;
    } else {
        newNode->next = head;
        head->prev = newNode;
        head = newNode;
    }
    ++listSize;
}

template<typename T>
void List<T>::push_back(const T& value){
    Node* newNode = new Node(value);
    if(empty()){
        head = tail = newNode;
    } else {
        newNode->prev =tail;
        tail->next = newNode;
        tail = newNode;
    }
    listSize++;
}

template<typename T>    
void List<T>::insert(size_t index, const T& value) {
    if(index > listSize) {
        throw std::out_of_range("Index out of range in insert()");
    }
    if(index == 0){
        push_front(value);
    } else if(index == listSize) {
        push_back(value);
    } else {
        Node* B = getNodeAt(index);
        Node* A = B->prev;
        Node* newNode = new Node(value);
        newNode->prev = A;
        newNode->next = B;

        A->next =newNode;
        B->prev = newNode;
        ++listSize;
    }
}

template<typename T>
void List<T>::pop_back(){
    if(empty()) return ;
    Node* toDelete = tail;
    tail = tail->prev;

    if(tail != nullptr){
        tail->next = nullptr;
    } else {
        head = nullptr;
    }
    delete toDelete;
    --listSize;
}

template<typename T>
void List<T>::pop_front(){
    if(empty()) return ;
    Node* toDelete = head;
    head = head->next;

    if(head != nullptr){
        head->prev = nullptr;
    } else {
        tail = nullptr;
    }
    delete toDelete;
    --listSize;
}

template<typename T>
void List<T>::remove(size_t index){
    if(index >= listSize) {
        throw std::out_of_range("Index out of range in remove()");
    }
    if(index == 0){
        pop_front();
    } else if(index == listSize-1){
        pop_back();
    } else {
        Node* N = getNodeAt(index);
        Node* A = N->prev;
        Node* B = N->next;

        A->next = B;
        B->prev = A;

        delete N;
        --listSize;
    }
}

template<typename T>
T& List<T>::front() {
    if (empty()) throw std::out_of_range("List is empty");
    return head->data;
}

template<typename T>
const T& List<T>::front() const {
    if (empty()) throw std::out_of_range("List is empty");
    return head->data;
}

template<typename T>
T& List<T>::back() {
    if (empty()) throw std::out_of_range("List is empty");
    return tail->data;
}

template<typename T>
const T& List<T>::back() const {
    if (empty()) throw std::out_of_range("List is empty");
    return tail->data;
}

template<typename T>
T& List<T>::at(size_t index) {
    if (index >= listSize) throw std::out_of_range("Index out of range in at()");
    return getNodeAt(index)->data;
}

template<typename T>
const T& List<T>::at(size_t index) const {
    if (index >= listSize) throw std::out_of_range("Index out of range in at()");
    return getNodeAt(index)->data;
}


template<typename T>
bool List<T>::empty() const noexcept {
    return listSize == 0;
}

template<typename T>
size_t List<T>::size() const noexcept {
    return listSize;
}

template<typename T>
void List<T>::clear() {
    Node* current = head;
    while (current != nullptr) {
        Node* next = current->next;
        delete current;
        current = next;
    }
    head = nullptr;
    tail = nullptr;
    listSize = 0;
}

template<typename T>
bool List<T>::contains(const T& value) const {
    return find(value) != nullptr;
}

template<typename T>
typename List<T>::Node* List<T>::find(const T& value) const {
    Node* current = head;
    while (current != nullptr) {
        if (current->data == value) {
            return current;
        }
        current = current->next;
    }
    return nullptr;
}

template<typename T>
void List<T>::reverse() {
    Node* current = head;
    while(current){
        std::swap(current->next, current->prev);
        current = current->prev;
    }
    std::swap(head, tail);
}

