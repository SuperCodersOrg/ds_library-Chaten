#include <iostream>
#include <string>
#include <cassert>
#include <stdexcept>
#include "Vector.h"

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
    Vector<int> v;

    assert(v.empty());
    assert(v.size() == 0);

    v.push_back(10);
    v.push_back(20);
    v.push_back(30);

    assert(v.size() == 3);
    assert(v.front() == 10);
    assert(v.back() == 30);
    assert(v[1] == 20);
    assert(v.at(2) == 30);

    v.pop_back();
    assert(v.size() == 2);
    assert(v.back() == 20);

    v.insert(0, 5);
    assert(v.front() == 5);
    assert(v.size() == 3);

    v.insert(2, 15);
    assert(v[2] == 15);

    v.remove(1);
    assert(v[1] == 15);

    v.clear();
    assert(v.empty());
    assert(v.size() == 0);
}

void testComplexType() {
    Vector<std::string> v = {"Apple", "Banana", "Cherry"};

    assert(v.size() == 3);
    assert(v.front() == "Apple");
    assert(v.back() == "Cherry");

    v.push_back("Date");
    assert(v.size() == 4);
    assert(v.back() == "Date");

    Vector<std::string> vCopy = v;
    assert(vCopy.size() == 4);
    assert(vCopy[1] == "Banana");

    vCopy[0] = "Apricot";
    assert(v[0] == "Apple");
    assert(vCopy[0] == "Apricot");

    Vector<std::string> vAssign;
    vAssign = v;
    assert(vAssign.size() == 4);
    assert(vAssign[2] == "Cherry");

    vAssign.pop_back();
    assert(vAssign.size() == 3);
    assert(vAssign.contains("Banana"));
    assert(!vAssign.contains("Date"));
}

void testCustomType() {
    Vector<CustomType> v;

    v.push_back(CustomType(1, "Alice"));
    v.push_back(CustomType(2, "Bob"));

    assert(v.size() == 2);
    assert(v[0].name == "Alice");

    v.insert(1, CustomType(3, "Charlie"));
    assert(v.size() == 3);
    assert(v[1].name == "Charlie");

    assert(v.contains(CustomType(2, "Bob")));
    assert(v.find(CustomType(1, "Alice")) == 0);

    v.remove(0);
    assert(v[0].name == "Charlie");

    v.clear();
    assert(v.empty());
}

void testCapacityAndMemory() {
    Vector<int> v;
    size_t initialCap = v.capacity();

    v.reserve(100);
    assert(v.capacity() >= 100);

    for(int i = 0; i < 50; ++i) {
        v.push_back(i);
    }

    v.shrink_to_fit();
    assert(v.capacity() == 50);

    v.reserve(10);
    assert(v.capacity() == 50);
}

void testExceptions() {
    Vector<int> v;
    bool caught;

    caught = false;
    try { v.pop_back(); }
    catch(const std::out_of_range&) { caught = true; }
    assert(caught);

    caught = false;
    try { v.front(); }
    catch(const std::out_of_range&) { caught = true; }
    assert(caught);

    caught = false;
    try { v.back(); }
    catch(const std::out_of_range&) { caught = true; }
    assert(caught);

    caught = false;
    try { v.at(0); }
    catch(const std::out_of_range&) { caught = true; }
    assert(caught);

    caught = false;
    try { v.insert(1, 10); }
    catch(const std::out_of_range&) { caught = true; }
    assert(caught);

    caught = false;
    try { v.remove(0); }
    catch(const std::out_of_range&) { caught = true; }
    assert(caught);

    v.push_back(100);

    caught = false;
    try { v.at(1); }
    catch(const std::out_of_range&) { caught = true; }
    assert(caught);

    caught = false;
    try { v.remove(1); }
    catch(const std::out_of_range&) { caught = true; }
    assert(caught);
}

int main() {
    testPrimitiveType();
    testComplexType();
    testCustomType();
    testCapacityAndMemory();
    testExceptions();

    std::cout << "All comprehensive tests passed successfully!\n";
    return 0;
}