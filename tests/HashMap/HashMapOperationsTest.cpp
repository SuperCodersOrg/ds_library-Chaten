#include <gtest/gtest.h>
#include "HashMap.h"
#include <string>


TEST(HashMapOperations, SetAndGet)
{
    HashMap<std::string, int> m;
    m.set("Key", 42);

    EXPECT_EQ(m.size(), 1u);
    EXPECT_EQ(m.get("Key"), 42);
}

TEST(HashMapOperations, SetUpdatesExistingKey)
{
    HashMap<std::string, int> m;
    m.set("Key", 10);
    m.set("Key", 20);

    EXPECT_EQ(m.size(), 1u);
    EXPECT_EQ(m.get("Key"), 20);
}

TEST(HashMapOperations, SetTriggersRehash)
{
    HashMap<std::string, int> m;

    for (int i = 1; i <= 20; ++i) {
        m.set("Key_" + std::to_string(i), i * 10);
    }

    EXPECT_EQ(m.size(), 20u);

    for (int i = 1; i <= 20; ++i) {
        EXPECT_EQ(m.get("Key_" + std::to_string(i)), i * 10);
    }
}

TEST(HashMapOperations, RemoveExistingKey)
{
    HashMap<std::string, int> m;
    m.set("A", 1);
    m.set("B", 2);

    m.remove("A");

    EXPECT_EQ(m.size(), 1u);
    EXPECT_FALSE(m.contains("A"));
    EXPECT_TRUE(m.contains("B"));
}

TEST(HashMapOperations, RemoveMissingKeyThrows)
{
    HashMap<std::string, int> m;

    EXPECT_THROW(m.remove("Missing"), std::out_of_range);
}

TEST(HashMapOperations, RemoveHandlesCollisions)
{
    HashMap<std::string, int> m;

    m.set("a", 1);
    m.set("b", 2);
    m.set("c", 3);

    m.remove("b");

    EXPECT_EQ(m.size(), 2u);
    EXPECT_TRUE(m.contains("a"));
    EXPECT_FALSE(m.contains("b"));
    EXPECT_TRUE(m.contains("c"));
}

TEST(HashMapOperations, ClearRemovesAllElements)
{
    HashMap<std::string, int> m;
    m.set("X", 100);
    m.set("Y", 200);

    m.clear();

    EXPECT_TRUE(m.empty());
    EXPECT_EQ(m.size(), 0u);
    EXPECT_FALSE(m.contains("X"));
    EXPECT_FALSE(m.contains("Y"));
}
