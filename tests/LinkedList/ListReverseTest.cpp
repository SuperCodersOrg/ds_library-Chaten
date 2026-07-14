#include <gtest/gtest.h>
#include "List.h"
#include <string>


TEST(ListReverse, EmptyList)
{
    List<int> l;
    l.reverse();

    EXPECT_TRUE(l.empty());
    EXPECT_EQ(l.size(), 0u);
}

TEST(ListReverse, SingleElement)
{
    List<int> l = {42};
    l.reverse();

    EXPECT_EQ(l.size(), 1u);
    EXPECT_EQ(l.front(), 42);
    EXPECT_EQ(l.back(), 42);
}

TEST(ListReverse, MultipleElements)
{
    List<int> l = {1, 2, 3, 4, 5};
    l.reverse();

    EXPECT_EQ(l.size(), 5u);
    EXPECT_EQ(l.at(0), 5);
    EXPECT_EQ(l.at(1), 4);
    EXPECT_EQ(l.at(2), 3);
    EXPECT_EQ(l.at(3), 2);
    EXPECT_EQ(l.at(4), 1);

    EXPECT_EQ(l.front(), 5);
    EXPECT_EQ(l.back(), 1);
}

TEST(ListReverse, StringType)
{
    List<std::string> l = {"First", "Second", "Third"};
    l.reverse();

    EXPECT_EQ(l.size(), 3u);
    EXPECT_EQ(l.at(0), "Third");
    EXPECT_EQ(l.at(1), "Second");
    EXPECT_EQ(l.at(2), "First");
}

TEST(ListReverse, DoubleReverseRestoresOrder)
{
    List<int> l = {10, 20, 30};
    l.reverse();
    l.reverse();

    EXPECT_EQ(l.size(), 3u);
    EXPECT_EQ(l.at(0), 10);
    EXPECT_EQ(l.at(1), 20);
    EXPECT_EQ(l.at(2), 30);
}
