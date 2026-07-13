#include <gtest/gtest.h>
#include "Vector.h"
#include <string>


class VectorFindTest : public ::testing::Test {
protected:
    Vector<int> v{10, 20, 30, 40, 50};
};

TEST_F(VectorFindTest, FindExistingElement)
{
    EXPECT_EQ(v.find(10), 0u);
    EXPECT_EQ(v.find(30), 2u);
    EXPECT_EQ(v.find(50), 4u);
}

TEST_F(VectorFindTest, FindNonExistingReturnsSize)
{
    EXPECT_EQ(v.find(99), v.size());
    EXPECT_EQ(v.find(-1), v.size());
}

TEST_F(VectorFindTest, FindFirstOccurrence)
{
    Vector<int> v2;
    v2.push_back(5);
    v2.push_back(10);
    v2.push_back(5);
    v2.push_back(10);

    EXPECT_EQ(v2.find(5), 0u);
    EXPECT_EQ(v2.find(10), 1u);
}

TEST(VectorFindStandalone, FindInEmptyVector)
{
    Vector<int> v;

    EXPECT_EQ(v.find(42), v.size());
}

TEST(VectorFindStandalone, FindSingleElement)
{
    Vector<int> v = {42};

    EXPECT_EQ(v.find(42), 0u);
    EXPECT_EQ(v.find(99), v.size());
}

TEST(VectorFindStandalone, FindStringType)
{
    Vector<std::string> v = {"Apple", "Banana", "Cherry"};

    EXPECT_EQ(v.find("Banana"), 1u);
    EXPECT_EQ(v.find("Date"), v.size());
}

TEST(VectorFindStandalone, FindAfterRemove)
{
    Vector<int> v = {1, 2, 3, 4, 5};
    v.remove(2); // Remove 3

    EXPECT_EQ(v.find(3), v.size());
    EXPECT_EQ(v.find(4), 2u);
}

TEST(VectorFindStandalone, FindAfterInsert)
{
    Vector<int> v = {1, 2, 3};
    v.insert(1, 99);

    EXPECT_EQ(v.find(99), 1u);
    EXPECT_EQ(v.find(2), 2u);
}

class VectorContainsTest : public ::testing::Test {
protected:
    Vector<int> v{100, 200, 300, 400, 500};
};

TEST_F(VectorContainsTest, ContainsExistingElement)
{
    EXPECT_TRUE(v.contains(100));
    EXPECT_TRUE(v.contains(300));
    EXPECT_TRUE(v.contains(500));
}

TEST_F(VectorContainsTest, DoesNotContainMissingElement)
{
    EXPECT_FALSE(v.contains(0));
    EXPECT_FALSE(v.contains(150));
    EXPECT_FALSE(v.contains(999));
}

TEST(VectorContainsStandalone, EmptyVectorContainsNothing)
{
    Vector<int> v;

    EXPECT_FALSE(v.contains(0));
    EXPECT_FALSE(v.contains(42));
}

TEST(VectorContainsStandalone, ContainsSingleElement)
{
    Vector<int> v = {7};

    EXPECT_TRUE(v.contains(7));
    EXPECT_FALSE(v.contains(8));
}

TEST(VectorContainsStandalone, ContainsAfterPush)
{
    Vector<int> v;
    v.push_back(42);

    EXPECT_TRUE(v.contains(42));
}

TEST(VectorContainsStandalone, ContainsAfterPop)
{
    Vector<int> v = {1, 2, 3};
    v.pop_back();

    EXPECT_FALSE(v.contains(3));
    EXPECT_TRUE(v.contains(1));
    EXPECT_TRUE(v.contains(2));
}

TEST(VectorContainsStandalone, ContainsAfterClear)
{
    Vector<int> v = {10, 20, 30};
    v.clear();

    EXPECT_FALSE(v.contains(10));
    EXPECT_FALSE(v.contains(20));
    EXPECT_FALSE(v.contains(30));
}

TEST(VectorContainsStandalone, ContainsStringType)
{
    Vector<std::string> v = {"Hello", "World"};

    EXPECT_TRUE(v.contains("Hello"));
    EXPECT_TRUE(v.contains("World"));
    EXPECT_FALSE(v.contains("Foo"));
}

TEST(VectorContainsStandalone, ContainsWithDuplicates)
{
    Vector<int> v = {5, 5, 5};

    EXPECT_TRUE(v.contains(5));
    EXPECT_FALSE(v.contains(6));
}
