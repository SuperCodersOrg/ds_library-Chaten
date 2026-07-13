#include <gtest/gtest.h>
#include "Vector.h"
#include <string>


class VectorPopBackTest : public ::testing::Test {
protected:
    Vector<int> v{10, 20, 30, 40, 50};
};

TEST_F(VectorPopBackTest, PopReducesSize)
{
    v.pop_back();

    EXPECT_EQ(v.size(), 4u);
}

TEST_F(VectorPopBackTest, PopRemovesLastElement)
{
    v.pop_back();

    EXPECT_EQ(v.back(), 40);
}

TEST_F(VectorPopBackTest, PopMultipleTimes)
{
    v.pop_back();
    v.pop_back();
    v.pop_back();

    EXPECT_EQ(v.size(), 2u);
    EXPECT_EQ(v[0], 10);
    EXPECT_EQ(v[1], 20);
}

TEST_F(VectorPopBackTest, PopAllElements)
{
    for (int i = 0; i < 5; ++i) {
        v.pop_back();
    }

    EXPECT_TRUE(v.empty());
    EXPECT_EQ(v.size(), 0u);
}

TEST(VectorPopBackStandalone, PopEmptyVectorThrows)
{
    Vector<int> v;

    EXPECT_THROW(v.pop_back(), std::out_of_range);
}

TEST(VectorPopBackStandalone, PopAfterPushBack)
{
    Vector<int> v;
    v.push_back(1);
    v.push_back(2);
    v.pop_back();

    EXPECT_EQ(v.size(), 1u);
    EXPECT_EQ(v[0], 1);
}

TEST(VectorPopBackStandalone, PopThenPushAgain)
{
    Vector<int> v = {5, 10, 15};
    v.pop_back();
    v.push_back(99);

    EXPECT_EQ(v.size(), 3u);
    EXPECT_EQ(v[2], 99);
}

TEST(VectorPopBackStandalone, PopStringType)
{
    Vector<std::string> v = {"One", "Two", "Three"};
    v.pop_back();

    EXPECT_EQ(v.size(), 2u);
    EXPECT_EQ(v.back(), "Two");
}

TEST(VectorPopBackStandalone, PopAllThenThrows)
{
    Vector<int> v = {42};
    v.pop_back();

    EXPECT_THROW(v.pop_back(), std::out_of_range);
}
