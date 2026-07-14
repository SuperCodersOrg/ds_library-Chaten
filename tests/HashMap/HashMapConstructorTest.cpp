#include <gtest/gtest.h>
#include "HashMap.h"
#include <string>


TEST(HashMapDefaultConstructor, CreatesEmptyMap)
{
    HashMap<std::string, int> m;

    EXPECT_TRUE(m.empty());
    EXPECT_EQ(m.size(), 0u);
}

TEST(HashMapCopyConstructor, CopiesAllElements)
{
    HashMap<std::string, int> original;
    original.set("One", 1);
    original.set("Two", 2);
    original.set("Three", 3);

    HashMap<std::string, int> copy(original);

    EXPECT_EQ(copy.size(), original.size());
    EXPECT_EQ(copy.get("One"), 1);
    EXPECT_EQ(copy.get("Two"), 2);
    EXPECT_EQ(copy.get("Three"), 3);
}

TEST(HashMapCopyConstructor, CopyIsIndependent)
{
    HashMap<std::string, int> original;
    original.set("One", 1);

    HashMap<std::string, int> copy(original);
    copy.set("One", 999);

    EXPECT_EQ(original.get("One"), 1);
    EXPECT_EQ(copy.get("One"), 999);
}

TEST(HashMapCopyConstructor, CopyEmptyMap)
{
    HashMap<std::string, int> original;
    HashMap<std::string, int> copy(original);

    EXPECT_TRUE(copy.empty());
    EXPECT_EQ(copy.size(), 0u);
}

TEST(HashMapCopyAssignment, AssignsAllElements)
{
    HashMap<std::string, int> original;
    original.set("A", 10);
    original.set("B", 20);

    HashMap<std::string, int> assigned;
    assigned = original;

    EXPECT_EQ(assigned.size(), original.size());
    EXPECT_EQ(assigned.get("A"), 10);
    EXPECT_EQ(assigned.get("B"), 20);
}

TEST(HashMapCopyAssignment, AssignedIsIndependent)
{
    HashMap<std::string, int> original;
    original.set("X", 100);

    HashMap<std::string, int> assigned;
    assigned = original;
    assigned.set("X", 200);

    EXPECT_EQ(original.get("X"), 100);
    EXPECT_EQ(assigned.get("X"), 200);
}

TEST(HashMapCopyAssignment, SelfAssignment)
{
    HashMap<std::string, int> m;
    m.set("Key", 42);

    m = m;

    EXPECT_EQ(m.size(), 1u);
    EXPECT_EQ(m.get("Key"), 42);
}

TEST(HashMapCopyAssignment, AssignToNonEmpty)
{
    HashMap<std::string, int> m1;
    m1.set("A", 1);
    m1.set("B", 2);

    HashMap<std::string, int> m2;
    m2.set("C", 3);

    m2 = m1;

    EXPECT_EQ(m2.size(), 2u);
    EXPECT_EQ(m2.get("A"), 1);
    EXPECT_EQ(m2.get("B"), 2);
    EXPECT_FALSE(m2.contains("C"));
}

TEST(HashMapCopyAssignment, AssignEmptyMap)
{
    HashMap<std::string, int> m1;
    HashMap<std::string, int> m2;
    m2.set("Key", 10);

    m2 = m1;

    EXPECT_TRUE(m2.empty());
    EXPECT_EQ(m2.size(), 0u);
}
