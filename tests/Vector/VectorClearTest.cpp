#include <gtest/gtest.h>
#include "Vector.h"
#include <string>


TEST(VectorClear, ClearNonEmptyVector)
{
    Vector<int> v = {1, 2, 3, 4, 5};
    v.clear();

    EXPECT_TRUE(v.empty());
    EXPECT_EQ(v.size(), 0u);
}

TEST(VectorClear, ClearEmptyVectorIsNoOp)
{
    Vector<int> v;
    v.clear();

    EXPECT_TRUE(v.empty());
    EXPECT_EQ(v.size(), 0u);
}

TEST(VectorClear, ClearPreservesCapacity)
{
    Vector<int> v = {1, 2, 3, 4, 5};
    size_t capBefore = v.capacity();

    v.clear();

    EXPECT_EQ(v.capacity(), capBefore);
}

TEST(VectorClear, PushAfterClear)
{
    Vector<int> v = {10, 20, 30};
    v.clear();

    v.push_back(100);
    v.push_back(200);

    EXPECT_EQ(v.size(), 2u);
    EXPECT_EQ(v[0], 100);
    EXPECT_EQ(v[1], 200);
}

TEST(VectorClear, ClearStringVector)
{
    Vector<std::string> v = {"Hello", "World"};
    v.clear();

    EXPECT_TRUE(v.empty());
}

TEST(VectorClear, ClearThenReuse)
{
    Vector<int> v;
    for (int i = 0; i < 100; ++i)
        v.push_back(i);

    v.clear();
    EXPECT_TRUE(v.empty());

    for (int i = 0; i < 50; ++i)
        v.push_back(i * 10);

    EXPECT_EQ(v.size(), 50u);
    EXPECT_EQ(v[0], 0);
    EXPECT_EQ(v[49], 490);
}

TEST(VectorClear, MultipleClearCalls)
{
    Vector<int> v = {1, 2, 3};
    v.clear();
    v.clear();
    v.clear();

    EXPECT_TRUE(v.empty());
}
