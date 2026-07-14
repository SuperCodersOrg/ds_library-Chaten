#include <gtest/gtest.h>
#include "List.h"
#include <string>


class ListAtTest : public ::testing::Test {
protected:
    List<int> l{100, 200, 300};
};

TEST_F(ListAtTest, AccessValidIndex)
{
    EXPECT_EQ(l.at(0), 100);
    EXPECT_EQ(l.at(1), 200);
    EXPECT_EQ(l.at(2), 300);
}

TEST_F(ListAtTest, ModifyViaAt)
{
    l.at(1) = 999;
    EXPECT_EQ(l.at(1), 999);
}

TEST_F(ListAtTest, AtOutOfRangeThrows)
{
    EXPECT_THROW(l.at(3), std::out_of_range);
    EXPECT_THROW(l.at(100), std::out_of_range);
}

TEST(ListAtStandalone, AtOnEmptyListThrows)
{
    List<int> l;

    EXPECT_THROW(l.at(0), std::out_of_range);
}

TEST(ListAtStandalone, ConstAtAccess)
{
    const List<int> l = {7, 14, 21};

    EXPECT_EQ(l.at(0), 7);
    EXPECT_EQ(l.at(1), 14);
    EXPECT_EQ(l.at(2), 21);
}

TEST(ListAtStandalone, ConstAtThrows)
{
    const List<int> l = {1, 2};

    EXPECT_THROW(l.at(2), std::out_of_range);
}

TEST(ListFront, FrontReturnsFirstElement)
{
    List<int> l = {42, 99, 7};

    EXPECT_EQ(l.front(), 42);
}

TEST(ListFront, FrontModifiable)
{
    List<int> l = {1, 2, 3};
    l.front() = 100;

    EXPECT_EQ(l.at(0), 100);
}

TEST(ListFront, FrontOnSingleElement)
{
    List<int> l = {77};

    EXPECT_EQ(l.front(), 77);
}

TEST(ListFront, FrontOnEmptyThrows)
{
    List<int> l;

    EXPECT_THROW(l.front(), std::out_of_range);
}

TEST(ListFront, ConstFront)
{
    const List<int> l = {10, 20};

    EXPECT_EQ(l.front(), 10);
}

TEST(ListFront, ConstFrontOnEmptyThrows)
{
    const List<int> l = {};

    EXPECT_THROW(l.front(), std::out_of_range);
}

TEST(ListFront, FrontStringType)
{
    List<std::string> l = {"First", "Second"};

    EXPECT_EQ(l.front(), "First");
}

TEST(ListBack, BackReturnsLastElement)
{
    List<int> l = {10, 20, 30};

    EXPECT_EQ(l.back(), 30);
}

TEST(ListBack, BackModifiable)
{
    List<int> l = {1, 2, 3};
    l.back() = 999;

    EXPECT_EQ(l.at(2), 999);
}

TEST(ListBack, BackOnSingleElement)
{
    List<int> l = {55};

    EXPECT_EQ(l.back(), 55);
}

TEST(ListBack, BackOnEmptyThrows)
{
    List<int> l;

    EXPECT_THROW(l.back(), std::out_of_range);
}

TEST(ListBack, ConstBack)
{
    const List<int> l = {5, 10, 15};

    EXPECT_EQ(l.back(), 15);
}

TEST(ListBack, ConstBackOnEmptyThrows)
{
    const List<int> l = {};

    EXPECT_THROW(l.back(), std::out_of_range);
}

TEST(ListBack, FrontEqualsBackOnSingleElement)
{
    List<int> l = {42};

    EXPECT_EQ(l.front(), l.back());
}

TEST(ListBack, BackStringType)
{
    List<std::string> l = {"X", "Y", "Z"};

    EXPECT_EQ(l.back(), "Z");
}

TEST(ListSizeAndEmpty, EmptyReturnsCorrectBoolean)
{
    List<int> l;
    EXPECT_TRUE(l.empty());

    l.push_back(1);
    EXPECT_FALSE(l.empty());

    l.pop_back();
    EXPECT_TRUE(l.empty());
}

TEST(ListSizeAndEmpty, SizeReturnsCorrectCount)
{
    List<int> l;
    EXPECT_EQ(l.size(), 0u);

    l.push_back(10);
    EXPECT_EQ(l.size(), 1u);

    l.push_back(20);
    EXPECT_EQ(l.size(), 2u);

    l.pop_back();
    EXPECT_EQ(l.size(), 1u);
}

TEST(ListClear, ClearRemovesAllElements)
{
    List<int> l = {1, 2, 3, 4, 5};
    l.clear();

    EXPECT_TRUE(l.empty());
    EXPECT_EQ(l.size(), 0u);
    EXPECT_THROW(l.front(), std::out_of_range);
    EXPECT_THROW(l.back(), std::out_of_range);
}
