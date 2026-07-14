#include <gtest/gtest.h>
#include "List.h"
#include <string>


class ListInsertTest : public ::testing::Test {
protected:
    List<int> l;

    void SetUp() override {
        for (int i = 1; i <= 5; i++)
            l.push_back(i);
    }
};

TEST_F(ListInsertTest, InsertAtBeginning)
{
    l.insert(0, 100);

    EXPECT_EQ(l.size(), 6u);
    EXPECT_EQ(l.at(0), 100);
    EXPECT_EQ(l.at(1), 1);
    EXPECT_EQ(l.at(2), 2);
    EXPECT_EQ(l.at(3), 3);
    EXPECT_EQ(l.at(4), 4);
    EXPECT_EQ(l.at(5), 5);
}

TEST_F(ListInsertTest, InsertAtMiddle)
{
    l.insert(2, 99);

    EXPECT_EQ(l.size(), 6u);
    EXPECT_EQ(l.at(0), 1);
    EXPECT_EQ(l.at(1), 2);
    EXPECT_EQ(l.at(2), 99);
    EXPECT_EQ(l.at(3), 3);
    EXPECT_EQ(l.at(4), 4);
    EXPECT_EQ(l.at(5), 5);
}

TEST_F(ListInsertTest, InsertAtEnd)
{
    l.insert(l.size(), 200);

    EXPECT_EQ(l.size(), 6u);
    EXPECT_EQ(l.back(), 200);
}

TEST_F(ListInsertTest, InsertPreservesExistingOrder)
{
    l.insert(3, 77);

    EXPECT_EQ(l.at(0), 1);
    EXPECT_EQ(l.at(1), 2);
    EXPECT_EQ(l.at(2), 3);
    EXPECT_EQ(l.at(3), 77);
    EXPECT_EQ(l.at(4), 4);
    EXPECT_EQ(l.at(5), 5);
}

TEST(ListInsertStandalone, InsertIntoEmptyList)
{
    List<int> l;
    l.insert(0, 10);

    EXPECT_EQ(l.size(), 1u);
    EXPECT_EQ(l.at(0), 10);
}

TEST(ListInsertStandalone, InsertPastEndThrows)
{
    List<int> l;
    l.push_back(10);

    EXPECT_THROW(l.insert(2, 5), std::out_of_range);
}

TEST(ListInsertStandalone, InsertOutOfRangeOnEmptyThrows)
{
    List<int> l;

    EXPECT_THROW(l.insert(1, 42), std::out_of_range);
}

TEST(ListInsertStandalone, InsertLargeDataset)
{
    List<int> l;

    for (int i = 0; i < 1000; i++)
        l.push_back(i);

    l.insert(500, -1);

    EXPECT_EQ(l.size(), 1001u);
    EXPECT_EQ(l.at(500), -1);

    for (int i = 0; i < 500; i++)
        EXPECT_EQ(l.at(i), i);

    for (int i = 501; i < 1001; i++)
        EXPECT_EQ(l.at(i), i - 1);
}

TEST(ListInsertStandalone, InsertMultipleTimesAtSameIndex)
{
    List<int> l = {1, 2, 3};

    l.insert(1, 10);
    l.insert(1, 20);
    l.insert(1, 30);

    EXPECT_EQ(l.size(), 6u);
    EXPECT_EQ(l.at(0), 1);
    EXPECT_EQ(l.at(1), 30);
    EXPECT_EQ(l.at(2), 20);
    EXPECT_EQ(l.at(3), 10);
    EXPECT_EQ(l.at(4), 2);
    EXPECT_EQ(l.at(5), 3);
}

TEST(ListInsertStandalone, InsertStringType)
{
    List<std::string> l = {"A", "C"};

    l.insert(1, "B");

    EXPECT_EQ(l.size(), 3u);
    EXPECT_EQ(l.at(0), "A");
    EXPECT_EQ(l.at(1), "B");
    EXPECT_EQ(l.at(2), "C");
}

class ListRemoveTest : public ::testing::Test {
protected:
    List<int> l;

    void SetUp() override {
        for (int i = 1; i <= 5; i++)
            l.push_back(i);
    }
};

TEST_F(ListRemoveTest, RemoveAtBeginning)
{
    l.remove(0);

    EXPECT_EQ(l.size(), 4u);
    EXPECT_EQ(l.at(0), 2);
    EXPECT_EQ(l.at(1), 3);
    EXPECT_EQ(l.at(2), 4);
    EXPECT_EQ(l.at(3), 5);
}

TEST_F(ListRemoveTest, RemoveAtMiddle)
{
    l.remove(2);

    EXPECT_EQ(l.size(), 4u);
    EXPECT_EQ(l.at(0), 1);
    EXPECT_EQ(l.at(1), 2);
    EXPECT_EQ(l.at(2), 4);
    EXPECT_EQ(l.at(3), 5);
}

TEST_F(ListRemoveTest, RemoveAtEnd)
{
    l.remove(l.size() - 1);

    EXPECT_EQ(l.size(), 4u);
    EXPECT_EQ(l.at(0), 1);
    EXPECT_EQ(l.at(1), 2);
    EXPECT_EQ(l.at(2), 3);
    EXPECT_EQ(l.at(3), 4);
}

TEST(ListRemoveStandalone, RemoveLastElementMakesEmpty)
{
    List<int> l;
    l.push_back(42);
    l.remove(0);

    EXPECT_TRUE(l.empty());
    EXPECT_EQ(l.size(), 0u);
}

TEST(ListRemoveStandalone, RemoveOutOfRangeThrows)
{
    List<int> l = {1, 2, 3};

    EXPECT_THROW(l.remove(3), std::out_of_range);
    EXPECT_THROW(l.remove(99), std::out_of_range);
}

TEST(ListRemoveStandalone, RemoveOnEmptyThrows)
{
    List<int> l;

    EXPECT_THROW(l.remove(0), std::out_of_range);
}

TEST(ListRemoveStandalone, RemoveStringType)
{
    List<std::string> l = {"Apple", "Banana", "Cherry"};
    l.remove(1);

    EXPECT_EQ(l.size(), 2u);
    EXPECT_EQ(l.at(0), "Apple");
    EXPECT_EQ(l.at(1), "Cherry");
}
