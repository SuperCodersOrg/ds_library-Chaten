#include <gtest/gtest.h>
#include "List.h"
#include <string>


TEST(ListPushBack, PushSingleElement)
{
    List<int> l;
    l.push_back(42);

    EXPECT_EQ(l.size(), 1u);
    EXPECT_EQ(l.front(), 42);
    EXPECT_EQ(l.back(), 42);
    EXPECT_FALSE(l.empty());
}

TEST(ListPushBack, PushMultipleElements)
{
    List<int> l;

    l.push_back(10);
    l.push_back(20);
    l.push_back(30);

    EXPECT_EQ(l.size(), 3u);
    EXPECT_EQ(l.at(0), 10);
    EXPECT_EQ(l.at(1), 20);
    EXPECT_EQ(l.at(2), 30);
}

TEST(ListPushBack, PushIncreasesSize)
{
    List<int> l;

    for (int i = 0; i < 10; ++i) {
        EXPECT_EQ(l.size(), static_cast<size_t>(i));
        l.push_back(i);
    }
    EXPECT_EQ(l.size(), 10u);
}

TEST(ListPushBack, PushStringType)
{
    List<std::string> l;

    l.push_back("Hello");
    l.push_back("World");

    EXPECT_EQ(l.size(), 2u);
    EXPECT_EQ(l.at(0), "Hello");
    EXPECT_EQ(l.at(1), "World");
}

TEST(ListPushBack, PushLargeDataset)
{
    List<int> l;

    for (int i = 0; i < 1000; ++i) {
        l.push_back(i);
    }

    EXPECT_EQ(l.size(), 1000u);

    for (int i = 0; i < 1000; ++i) {
        EXPECT_EQ(l.at(i), i);
    }
}

TEST(ListPushBack, PushAfterClear)
{
    List<int> l = {1, 2, 3};
    l.clear();

    l.push_back(100);

    EXPECT_EQ(l.size(), 1u);
    EXPECT_EQ(l.at(0), 100);
}

TEST(ListPushFront, PushSingleElement)
{
    List<int> l;
    l.push_front(42);

    EXPECT_EQ(l.size(), 1u);
    EXPECT_EQ(l.front(), 42);
    EXPECT_EQ(l.back(), 42);
    EXPECT_FALSE(l.empty());
}

TEST(ListPushFront, PushMultipleElements)
{
    List<int> l;

    l.push_front(10);
    l.push_front(20);
    l.push_front(30);

    EXPECT_EQ(l.size(), 3u);
    EXPECT_EQ(l.at(0), 30);
    EXPECT_EQ(l.at(1), 20);
    EXPECT_EQ(l.at(2), 10);
}

TEST(ListPushFront, PushStringType)
{
    List<std::string> l;

    l.push_front("World");
    l.push_front("Hello");

    EXPECT_EQ(l.size(), 2u);
    EXPECT_EQ(l.at(0), "Hello");
    EXPECT_EQ(l.at(1), "World");
}

TEST(ListPushFront, PushAfterClear)
{
    List<int> l = {1, 2, 3};
    l.clear();

    l.push_front(100);

    EXPECT_EQ(l.size(), 1u);
    EXPECT_EQ(l.at(0), 100);
}

class ListPopBackTest : public ::testing::Test {
protected:
    List<int> l{10, 20, 30, 40, 50};
};

TEST_F(ListPopBackTest, PopReducesSize)
{
    l.pop_back();

    EXPECT_EQ(l.size(), 4u);
}

TEST_F(ListPopBackTest, PopRemovesLastElement)
{
    l.pop_back();

    EXPECT_EQ(l.back(), 40);
}

TEST_F(ListPopBackTest, PopMultipleTimes)
{
    l.pop_back();
    l.pop_back();
    l.pop_back();

    EXPECT_EQ(l.size(), 2u);
    EXPECT_EQ(l.at(0), 10);
    EXPECT_EQ(l.at(1), 20);
}

TEST_F(ListPopBackTest, PopAllElements)
{
    for (int i = 0; i < 5; ++i) {
        l.pop_back();
    }

    EXPECT_TRUE(l.empty());
    EXPECT_EQ(l.size(), 0u);
}

TEST(ListPopBackStandalone, PopEmptyListDoesNotThrow)
{
    List<int> l;

    EXPECT_NO_THROW(l.pop_back());
    EXPECT_TRUE(l.empty());
}

TEST(ListPopBackStandalone, PopAfterPushBack)
{
    List<int> l;
    l.push_back(1);
    l.push_back(2);
    l.pop_back();

    EXPECT_EQ(l.size(), 1u);
    EXPECT_EQ(l.at(0), 1);
}

TEST(ListPopBackStandalone, PopThenPushAgain)
{
    List<int> l = {5, 10, 15};
    l.pop_back();
    l.push_back(99);

    EXPECT_EQ(l.size(), 3u);
    EXPECT_EQ(l.at(2), 99);
}

TEST(ListPopBackStandalone, PopStringType)
{
    List<std::string> l = {"One", "Two", "Three"};
    l.pop_back();

    EXPECT_EQ(l.size(), 2u);
    EXPECT_EQ(l.back(), "Two");
}

class ListPopFrontTest : public ::testing::Test {
protected:
    List<int> l{10, 20, 30, 40, 50};
};

TEST_F(ListPopFrontTest, PopReducesSize)
{
    l.pop_front();

    EXPECT_EQ(l.size(), 4u);
}

TEST_F(ListPopFrontTest, PopRemovesFirstElement)
{
    l.pop_front();

    EXPECT_EQ(l.front(), 20);
}

TEST_F(ListPopFrontTest, PopMultipleTimes)
{
    l.pop_front();
    l.pop_front();
    l.pop_front();

    EXPECT_EQ(l.size(), 2u);
    EXPECT_EQ(l.at(0), 40);
    EXPECT_EQ(l.at(1), 50);
}

TEST_F(ListPopFrontTest, PopAllElements)
{
    for (int i = 0; i < 5; ++i) {
        l.pop_front();
    }

    EXPECT_TRUE(l.empty());
    EXPECT_EQ(l.size(), 0u);
}

TEST(ListPopFrontStandalone, PopEmptyListDoesNotThrow)
{
    List<int> l;

    EXPECT_NO_THROW(l.pop_front());
    EXPECT_TRUE(l.empty());
}
