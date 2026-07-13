#include <gtest/gtest.h>
#include "Vector.h"
#include <string>


TEST(VectorPushBack, PushSingleElement)
{
    Vector<int> v;
    v.push_back(42);

    EXPECT_EQ(v.size(), 1u);
    EXPECT_EQ(v[0], 42);
    EXPECT_FALSE(v.empty());
}

TEST(VectorPushBack, PushMultipleElements)
{
    Vector<int> v;

    v.push_back(10);
    v.push_back(20);
    v.push_back(30);

    EXPECT_EQ(v.size(), 3u);
    EXPECT_EQ(v[0], 10);
    EXPECT_EQ(v[1], 20);
    EXPECT_EQ(v[2], 30);
}

TEST(VectorPushBack, PushIncreasesSize)
{
    Vector<int> v;

    for (int i = 0; i < 10; ++i) {
        EXPECT_EQ(v.size(), static_cast<size_t>(i));
        v.push_back(i);
    }
    EXPECT_EQ(v.size(), 10u);
}

TEST(VectorPushBack, PushTriggersResize)
{
    Vector<int> v;
    size_t initialCap = v.capacity();
    for (size_t i = 0; i <= initialCap; ++i) {
        v.push_back(static_cast<int>(i));
    }

    EXPECT_GT(v.capacity(), initialCap);
    EXPECT_EQ(v.size(), initialCap + 1);
}

TEST(VectorPushBack, PushPreservesExistingElements)
{
    Vector<int> v = {1, 2, 3};
    v.push_back(4);

    EXPECT_EQ(v[0], 1);
    EXPECT_EQ(v[1], 2);
    EXPECT_EQ(v[2], 3);
    EXPECT_EQ(v[3], 4);
}

TEST(VectorPushBack, PushStringType)
{
    Vector<std::string> v;

    v.push_back("Hello");
    v.push_back("World");

    EXPECT_EQ(v.size(), 2u);
    EXPECT_EQ(v[0], "Hello");
    EXPECT_EQ(v[1], "World");
}

TEST(VectorPushBack, PushLargeDataset)
{
    Vector<int> v;

    for (int i = 0; i < 10000; ++i) {
        v.push_back(i);
    }

    EXPECT_EQ(v.size(), 10000u);

    for (int i = 0; i < 10000; ++i) {
        EXPECT_EQ(v[i], i);
    }
}

TEST(VectorPushBack, PushAfterClear)
{
    Vector<int> v = {1, 2, 3};
    v.clear();

    v.push_back(100);

    EXPECT_EQ(v.size(), 1u);
    EXPECT_EQ(v[0], 100);
}
