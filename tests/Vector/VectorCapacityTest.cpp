#include <gtest/gtest.h>
#include "Vector.h"


TEST(VectorSize, EmptyVectorSizeIsZero)
{
    Vector<int> v;
    EXPECT_EQ(v.size(), 0u);
}

TEST(VectorSize, SizeAfterPushBack)
{
    Vector<int> v;
    v.push_back(1);
    v.push_back(2);
    v.push_back(3);

    EXPECT_EQ(v.size(), 3u);
}

TEST(VectorSize, SizeAfterPopBack)
{
    Vector<int> v = {1, 2, 3, 4, 5};
    v.pop_back();
    v.pop_back();

    EXPECT_EQ(v.size(), 3u);
}

TEST(VectorSize, SizeAfterInsert)
{
    Vector<int> v = {1, 2, 3};
    v.insert(1, 99);

    EXPECT_EQ(v.size(), 4u);
}

TEST(VectorSize, SizeAfterRemove)
{
    Vector<int> v = {1, 2, 3, 4};
    v.remove(2);

    EXPECT_EQ(v.size(), 3u);
}

TEST(VectorSize, SizeAfterClear)
{
    Vector<int> v = {1, 2, 3};
    v.clear();

    EXPECT_EQ(v.size(), 0u);
}

TEST(VectorSize, InitListSize)
{
    Vector<int> v = {10, 20, 30, 40};

    EXPECT_EQ(v.size(), 4u);
}


TEST(VectorCapacity, DefaultCapacityAtLeastOne)
{
    Vector<int> v;

    EXPECT_GE(v.capacity(), 1u);
}

TEST(VectorCapacity, CapacityGrowsWithPush)
{
    Vector<int> v;
    size_t initialCap = v.capacity();

    for (size_t i = 0; i <= initialCap; ++i) {
        v.push_back(static_cast<int>(i));
    }

    EXPECT_GT(v.capacity(), initialCap);
}

TEST(VectorCapacity, CapacityDoublesOnResize)
{
    Vector<int> v;

    // Fill to capacity
    size_t cap = v.capacity();
    for (size_t i = 0; i < cap; ++i)
        v.push_back(static_cast<int>(i));

    size_t capBefore = v.capacity();
    v.push_back(999); // Trigger resize

    EXPECT_EQ(v.capacity(), capBefore * 2);
}

TEST(VectorCapacity, CapacityAlwaysGeSize)
{
    Vector<int> v;

    for (int i = 0; i < 100; ++i) {
        v.push_back(i);
        EXPECT_GE(v.capacity(), v.size());
    }
}

TEST(VectorEmpty, NewVectorIsEmpty)
{
    Vector<int> v;
    EXPECT_TRUE(v.empty());
}

TEST(VectorEmpty, NotEmptyAfterPush)
{
    Vector<int> v;
    v.push_back(1);

    EXPECT_FALSE(v.empty());
}

TEST(VectorEmpty, EmptyAfterClear)
{
    Vector<int> v = {1, 2, 3};
    v.clear();

    EXPECT_TRUE(v.empty());
}

TEST(VectorEmpty, EmptyAfterPopAll)
{
    Vector<int> v = {10};
    v.pop_back();

    EXPECT_TRUE(v.empty());
}

TEST(VectorEmpty, NotEmptyFromInitList)
{
    Vector<int> v = {1};

    EXPECT_FALSE(v.empty());
}


TEST(VectorReserve, ReserveIncreasesCapacity)
{
    Vector<int> v;
    v.reserve(100);

    EXPECT_GE(v.capacity(), 100u);
}

TEST(VectorReserve, ReserveDoesNotChangeSize)
{
    Vector<int> v = {1, 2, 3};
    v.reserve(100);

    EXPECT_EQ(v.size(), 3u);
}

TEST(VectorReserve, ReservePreservesElements)
{
    Vector<int> v = {10, 20, 30};
    v.reserve(50);

    EXPECT_EQ(v[0], 10);
    EXPECT_EQ(v[1], 20);
    EXPECT_EQ(v[2], 30);
}

TEST(VectorReserve, ReserveSmallerIsNoOp)
{
    Vector<int> v;
    v.reserve(100);
    size_t capAfterReserve = v.capacity();

    v.reserve(10);

    EXPECT_EQ(v.capacity(), capAfterReserve);
}

TEST(VectorReserve, ReserveSameCapacityIsNoOp)
{
    Vector<int> v;
    v.reserve(50);
    size_t cap = v.capacity();

    v.reserve(50);

    EXPECT_EQ(v.capacity(), cap);
}

TEST(VectorReserve, ReserveZeroIsNoOp)
{
    Vector<int> v = {1, 2, 3};
    size_t capBefore = v.capacity();

    v.reserve(0);

    EXPECT_EQ(v.capacity(), capBefore);
}

TEST(VectorShrinkToFit, ShrinksToSize)
{
    Vector<int> v;

    for (int i = 0; i < 50; ++i)
        v.push_back(i);

    v.shrink_to_fit();

    EXPECT_EQ(v.capacity(), v.size());
}

TEST(VectorShrinkToFit, PreservesElements)
{
    Vector<int> v;

    for (int i = 0; i < 20; ++i)
        v.push_back(i * 10);

    v.shrink_to_fit();

    EXPECT_EQ(v.size(), 20u);
    for (int i = 0; i < 20; ++i)
        EXPECT_EQ(v[i], i * 10);
}

TEST(VectorShrinkToFit, ShrinkEmptyVector)
{
    Vector<int> v;
    v.reserve(100);

    v.shrink_to_fit();

    EXPECT_GE(v.capacity(), 1u);
    EXPECT_TRUE(v.empty());
}

TEST(VectorShrinkToFit, ShrinkAfterPopBack)
{
    Vector<int> v = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    v.pop_back();
    v.pop_back();
    v.pop_back();

    v.shrink_to_fit();

    EXPECT_EQ(v.capacity(), v.size());
    EXPECT_EQ(v.size(), 7u);
}

TEST(VectorShrinkToFit, NoOpWhenCapacityEqualsSize)
{
    Vector<int> v = {1, 2, 3};
    size_t capBefore = v.capacity();

    v.shrink_to_fit();

    EXPECT_EQ(v.capacity(), capBefore);
}
