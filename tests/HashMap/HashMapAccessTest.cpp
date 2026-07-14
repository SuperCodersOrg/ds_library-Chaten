#include <gtest/gtest.h>
#include "HashMap.h"
#include <string>


TEST(HashMapAccess, GetExistingKey)
{
    HashMap<std::string, std::string> m;
    m.set("Hello", "World");

    EXPECT_EQ(m.get("Hello"), "World");
}

TEST(HashMapAccess, GetMissingKeyThrows)
{
    HashMap<std::string, int> m;

    EXPECT_THROW(m.get("Missing"), std::out_of_range);
}

TEST(HashMapAccess, ConstGetAccess)
{
    HashMap<std::string, int> m;
    m.set("Key", 100);

    const HashMap<std::string, int>& constMap = m;

    EXPECT_EQ(constMap.get("Key"), 100);
    EXPECT_THROW(constMap.get("Missing"), std::out_of_range);
}

TEST(HashMapAccess, ContainsReturnsCorrectBoolean)
{
    HashMap<std::string, int> m;

    EXPECT_FALSE(m.contains("Key"));

    m.set("Key", 42);
    EXPECT_TRUE(m.contains("Key"));

    m.remove("Key");
    EXPECT_FALSE(m.contains("Key"));
}

TEST(HashMapAccess, ConstContainsAccess)
{
    HashMap<std::string, int> m;
    m.set("Key", 10);

    const HashMap<std::string, int>& constMap = m;

    EXPECT_TRUE(constMap.contains("Key"));
    EXPECT_FALSE(constMap.contains("Missing"));
}

TEST(HashMapAccess, EmptyAndSize)
{
    HashMap<std::string, int> m;

    EXPECT_TRUE(m.empty());
    EXPECT_EQ(m.size(), 0u);

    m.set("A", 1);
    EXPECT_FALSE(m.empty());
    EXPECT_EQ(m.size(), 1u);

    m.set("B", 2);
    EXPECT_EQ(m.size(), 2u);

    m.remove("A");
    EXPECT_EQ(m.size(), 1u);
}

TEST(HashMapAccess, LoadFactor)
{
    HashMap<std::string, int> m;

    EXPECT_EQ(m.loadFactor(), 0.0f);

    m.set("A", 1);
    EXPECT_EQ(m.loadFactor(), 1.0f / 16.0f);

    m.set("B", 2);
    EXPECT_EQ(m.loadFactor(), 2.0f / 16.0f);
}
