#include <iostream>
#include <string>
#include <cassert>
#include <stdexcept>
#include "HashMap.h"

void testBasic() {
    HashMap<std::string, int> map;
    assert(map.empty());
    assert(map.size() == 0);

    map.set("A", 10);
    map.set("B", 20);
    assert(!map.empty());
    assert(map.size() == 2);
    assert(map.get("A") == 10);
    assert(map.get("B") == 20);
}

void testUpdate() {
    HashMap<std::string, int> map;
    map.set("X", 100);
    map.set("X", 200);
    assert(map.size() == 1);
    assert(map.get("X") == 200);
}

void testRemove() {
    HashMap<std::string, int> map;
    map.set("One", 1);
    map.set("Two", 2);
    map.set("Three", 3);

    map.remove("Two");
    assert(map.size() == 2);
    assert(!map.contains("Two"));
    assert(map.contains("One"));
    assert(map.contains("Three"));

    map.remove("One");
    map.remove("Three");
    assert(map.empty());
    assert(map.size() == 0);
}

void testExceptions() {
    HashMap<std::string, int> map;
    bool caught = false;

    try { map.get("Missing"); }
    catch(const std::out_of_range&) { caught = true; }
    assert(caught);

    caught = false;
    try { map.remove("Missing"); }
    catch(const std::out_of_range&) { caught = true; }
    assert(caught);
}

void testContainsAndClear() {
    HashMap<std::string, int> map;
    map.set("Hello", 1);
    assert(map.contains("Hello"));
    assert(!map.contains("World"));

    map.clear();
    assert(map.empty());
    assert(map.size() == 0);
    assert(!map.contains("Hello"));
}

void testRehash() {
    HashMap<std::string, int> map;
    for(int i = 0; i < 1000; ++i) {
        map.set(std::to_string(i), i * 2);
    }
    assert(map.size() == 1000);
    for(int i = 0; i < 1000; ++i) {
        assert(map.get(std::to_string(i)) == i * 2);
    }
}

void testRuleOfThree() {
    HashMap<std::string, int> map1;
    map1.set("Base", 100);
    map1.set("Data", 200);

    HashMap<std::string, int> map2 = map1;
    assert(map2.size() == 2);
    assert(map2.get("Base") == 100);

    map2.set("Base", 999);
    assert(map1.get("Base") == 100);
    assert(map2.get("Base") == 999);

    HashMap<std::string, int> map3;
    map3.set("Temp", 0);
    map3 = map1;
    assert(map3.size() == 2);
    assert(map3.get("Data") == 200);
    assert(!map3.contains("Temp"));

    map3 = map3;
    assert(map3.size() == 2);
    assert(map3.get("Data") == 200);
}

int main() {
    testBasic();
    testUpdate();
    testRemove();
    testExceptions();
    testContainsAndClear();
    testRehash();
    testRuleOfThree();

    std::cout << "All tests passed successfully!\n";
    return 0;
}