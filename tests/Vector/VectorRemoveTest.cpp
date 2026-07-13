#include <gtest/gtest.h>
#include "Vector.h"
#include <string>


class VectorRemoveTest : public ::testing::Test {
protected:
    Vector<int> v{10, 20, 30, 40, 50};
};

TEST_F(VectorRemoveTest, RemoveAtBeginning)
{
    v.remove(0);

    EXPECT_EQ(v.size(), 4u);
    EXPECT_EQ(v[0], 20);
    EXPECT_EQ(v[1], 30);
    EXPECT_EQ(v[2], 40);
    EXPECT_EQ(v[3], 50);
}

TEST_F(VectorRemoveTest, RemoveAtMiddle)
{
    v.remove(2);

    EXPECT_EQ(v.size(), 4u);
    EXPECT_EQ(v[0], 10);
    EXPECT_EQ(v[1], 20);
    EXPECT_EQ(v[2], 40);
    EXPECT_EQ(v[3], 50);
}

TEST_F(VectorRemoveTest, RemoveAtEnd)
{
    v.remove(4);

    EXPECT_EQ(v.size(), 4u);
    EXPECT_EQ(v.back(), 40);
}

TEST_F(VectorRemoveTest, RemoveShiftsElementsCorrectly)
{
    v.remove(1);

    for (size_t i = 0; i < v.size(); ++i) {
        EXPECT_NE(v[i], 20);
    }
}

TEST_F(VectorRemoveTest, RemoveAllOneByOne)
{
    while (!v.empty()) {
        v.remove(0);
    }

    EXPECT_TRUE(v.empty());
    EXPECT_EQ(v.size(), 0u);
}

TEST(VectorRemoveStandalone, RemoveSingleElement)
{
    Vector<int> v = {42};
    v.remove(0);

    EXPECT_TRUE(v.empty());
}

TEST(VectorRemoveStandalone, RemoveOutOfRangeThrows)
{
    Vector<int> v = {1, 2, 3};

    EXPECT_THROW(v.remove(3), std::out_of_range);
    EXPECT_THROW(v.remove(100), std::out_of_range);
}

TEST(VectorRemoveStandalone, RemoveFromEmptyThrows)
{
    Vector<int> v;

    EXPECT_THROW(v.remove(0), std::out_of_range);
}

TEST(VectorRemoveStandalone, RemoveStringType)
{
    Vector<std::string> v = {"A", "B", "C", "D"};
    v.remove(1);

    EXPECT_EQ(v.size(), 3u);
    EXPECT_EQ(v[0], "A");
    EXPECT_EQ(v[1], "C");
    EXPECT_EQ(v[2], "D");
}

TEST(VectorRemoveStandalone, RemoveThenInsert)
{
    Vector<int> v = {1, 2, 3, 4, 5};
    v.remove(2);
    v.insert(2, 99);

    EXPECT_EQ(v.size(), 5u);
    EXPECT_EQ(v[2], 99);
}
