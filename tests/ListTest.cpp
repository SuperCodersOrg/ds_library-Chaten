#include <iostream>
#include <string>
#include <cassert>
#include <stdexcept>
#include "list.h"

struct CustomType {
    int id;
    std::string name;

    CustomType() : id(0), name("") {}
    CustomType(int i, std::string n) : id(i), name(std::move(n)) {}

    bool operator==(const CustomType& other) const {
        return id == other.id && name == other.name;
    }
};

void testPrimitiveType() {
    List<int> l;

    assert(l.empty());
    assert(l.size() == 0);

    l.push_back(10);
    l.push_back(20);
    l.push_back(30);

    assert(l.size() == 3);
    assert(l.front() == 10);
    assert(l.back() == 30);
    assert(l.at(1) == 20);

    l.push_front(5);
    assert(l.size() == 4);
    assert(l.front() == 5);
    assert(l.at(1) == 10);

    l.pop_back();
    assert(l.size() == 3);
    assert(l.back() == 20);

    l.pop_front();
    assert(l.size() == 2);
    assert(l.front() == 10);

    l.insert(1, 15);
    assert(l.size() == 3);
    assert(l.at(1) == 15);

    l.remove(1);
    assert(l.size() == 2);
    assert(l.at(1) == 20);

    l.clear();
    assert(l.empty());
    assert(l.size() == 0);
}

void testComplexType() {
    List<std::string> l = {"Apple", "Banana", "Cherry"};

    assert(l.size() == 3);
    assert(l.front() == "Apple");
    assert(l.back() == "Cherry");

    l.push_back("Date");
    assert(l.size() == 4);
    assert(l.back() == "Date");

    List<std::string> lCopy = l;
    assert(lCopy.size() == 4);
    assert(lCopy.at(1) == "Banana");

    lCopy.at(0) = "Apricot";
    assert(l.front() == "Apple");
    assert(lCopy.front() == "Apricot");

    List<std::string> lAssign;
    lAssign = l;
    assert(lAssign.size() == 4);
    assert(lAssign.at(2) == "Cherry");

    lAssign.pop_back();
    assert(lAssign.size() == 3);
    assert(lAssign.contains("Banana"));
    assert(!lAssign.contains("Date"));
}

void testCustomType() {
    List<CustomType> l;

    l.push_back(CustomType(1, "Alice"));
    l.push_back(CustomType(2, "Bob"));

    assert(l.size() == 2);
    assert(l.at(0).name == "Alice");

    l.insert(1, CustomType(3, "Charlie"));
    assert(l.size() == 3);
    assert(l.at(1).name == "Charlie");

    assert(l.contains(CustomType(2, "Bob")));

    List<CustomType>::Node* foundNode = l.find(CustomType(1, "Alice"));
    assert(foundNode != nullptr);
    assert(foundNode->data.name == "Alice");

    assert(!l.contains(CustomType(99, "Nobody")));
    assert(l.find(CustomType(99, "Nobody")) == nullptr);

    l.clear();
    assert(l.empty());
}

void testReversal() {
    List<int> l = {1, 2, 3, 4, 5};
    l.reverse();
    assert(l.front() == 5);
    assert(l.back() == 1);
    assert(l.at(1) == 4);
    assert(l.at(3) == 2);

    l = {10, 20, 30, 40};
    l.reverse();
    assert(l.front() == 40);
    assert(l.back() == 10);
    assert(l.at(2) == 20);

    l.clear();
    l.push_back(42);
    l.reverse();
    assert(l.front() == 42);
    assert(l.back() == 42);
    assert(l.size() == 1);

    l.clear();
    l.reverse();
    assert(l.empty());
}

void testEdgeCasesAndExceptions() {
    List<int> l;
    bool caught;

    l.pop_front();
    l.pop_back();
    assert(l.empty());

    caught = false;
    try { l.front(); }
    catch(const std::out_of_range&) { caught = true; }
    assert(caught);

    caught = false;
    try { l.back(); }
    catch(const std::out_of_range&) { caught = true; }
    assert(caught);

    caught = false;
    try { l.at(0); }
    catch(const std::out_of_range&) { caught = true; }
    assert(caught);

    caught = false;
    try { l.insert(1, 10); }
    catch(const std::out_of_range&) { caught = true; }
    assert(caught);

    caught = false;
    try { l.remove(0); }
    catch(const std::out_of_range&) { caught = true; }
    assert(caught);

    l.push_back(100);
    assert(l.front() == 100);
    assert(l.back() == 100);

    l.remove(0);
    assert(l.empty());

    l.push_front(200);
    assert(l.front() == 200);
    assert(l.back() == 200);

    l.pop_back();
    assert(l.empty());

    l = {1, 2, 3};
    caught = false;
    try { l.at(3); }
    catch(const std::out_of_range&) { caught = true; }
    assert(caught);

    caught = false;
    try { l.remove(3); }
    catch(const std::out_of_range&) { caught = true; }
    assert(caught);
}

int main() {
    testPrimitiveType();
    testComplexType();
    testCustomType();
    testReversal();
    testEdgeCasesAndExceptions();

    std::cout << "All comprehensive tests passed successfully!\n";
    return 0;
}