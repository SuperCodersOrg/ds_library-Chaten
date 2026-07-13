#include <gtest/gtest.h>
#include "Vector.h"
#include <string>


class VectorInsertTest : public ::testing::Test {
protected:
    Vector<int> v;

    void SetUp() override {
        for (int i = 1; i <= 5; i++)
            v.push_back(i);
    }
};

TEST_F(VectorInsertTest, InsertAtBeginning)
{
    v.insert(0, 100);

    EXPECT_EQ(v.size(), 6u);
    EXPECT_EQ(v[0], 100);
    EXPECT_EQ(v[1], 1);
    EXPECT_EQ(v[2], 2);
    EXPECT_EQ(v[3], 3);
    EXPECT_EQ(v[4], 4);
    EXPECT_EQ(v[5], 5);
}

TEST_F(VectorInsertTest, InsertAtMiddle)
{
    v.insert(2, 99);

    EXPECT_EQ(v.size(), 6u);
    EXPECT_EQ(v[0], 1);
    EXPECT_EQ(v[1], 2);
    EXPECT_EQ(v[2], 99);
    EXPECT_EQ(v[3], 3);
    EXPECT_EQ(v[4], 4);
    EXPECT_EQ(v[5], 5);
}

TEST_F(VectorInsertTest, InsertAtEnd)
{
    v.insert(v.size(), 200);

    EXPECT_EQ(v.size(), 6u);
    EXPECT_EQ(v[5], 200);
}

TEST_F(VectorInsertTest, InsertPreservesExistingOrder)
{
    v.insert(3, 77);

    EXPECT_EQ(v[0], 1);
    EXPECT_EQ(v[1], 2);
    EXPECT_EQ(v[2], 3);
    EXPECT_EQ(v[3], 77);
    EXPECT_EQ(v[4], 4);
    EXPECT_EQ(v[5], 5);
}


TEST(VectorInsertStandalone, InsertIntoEmptyVector)
{
    Vector<int> v;
    v.insert(0, 10);

    EXPECT_EQ(v.size(), 1u);
    EXPECT_EQ(v[0], 10);
}

TEST(VectorInsertStandalone, InsertTriggersResize)
{
    Vector<int> v;

    v.reserve(2);
    v.push_back(1);
    v.push_back(2);

    size_t oldCap = v.capacity();

    v.insert(1, 99);

    EXPECT_GT(v.capacity(), oldCap);
    EXPECT_EQ(v.size(), 3u);

    EXPECT_EQ(v[0], 1);
    EXPECT_EQ(v[1], 99);
    EXPECT_EQ(v[2], 2);
}

TEST(VectorInsertStandalone, InsertPastEndThrows)
{
    Vector<int> v;
    v.push_back(10);

    EXPECT_THROW(v.insert(2, 5), std::out_of_range);
}

TEST(VectorInsertStandalone, InsertOutOfRangeOnEmptyThrows)
{
    Vector<int> v;

    EXPECT_THROW(v.insert(1, 42), std::out_of_range);
}

TEST(VectorInsertStandalone, InsertLargeDataset)
{
    Vector<int> v;

    for (int i = 0; i < 1000; i++)
        v.push_back(i);

    v.insert(500, -1);

    EXPECT_EQ(v.size(), 1001u);
    EXPECT_EQ(v[500], -1);

    for (int i = 0; i < 500; i++)
        EXPECT_EQ(v[i], i);

    for (int i = 501; i < 1001; i++)
        EXPECT_EQ(v[i], i - 1);
}

TEST(VectorInsertStandalone, InsertMultipleTimesAtSameIndex)
{
    Vector<int> v = {1, 2, 3};

    v.insert(1, 10);
    v.insert(1, 20);
    v.insert(1, 30);

    EXPECT_EQ(v.size(), 6u);
    EXPECT_EQ(v[0], 1);
    EXPECT_EQ(v[1], 30);
    EXPECT_EQ(v[2], 20);
    EXPECT_EQ(v[3], 10);
    EXPECT_EQ(v[4], 2);
    EXPECT_EQ(v[5], 3);
}

TEST(VectorInsertStandalone, InsertStringType)
{
    Vector<std::string> v = {"A", "C"};

    v.insert(1, "B");

    EXPECT_EQ(v.size(), 3u);
    EXPECT_EQ(v[0], "A");
    EXPECT_EQ(v[1], "B");
    EXPECT_EQ(v[2], "C");
}
