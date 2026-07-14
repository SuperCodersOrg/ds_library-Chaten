#include <gtest/gtest.h>
#include "List.h"
#include <string>


class ListFindTest : public ::testing::Test {
protected:
    List<int> l{10, 20, 30, 40, 50};
};

TEST_F(ListFindTest, FindExistingElement)
{
    typename List<int>::Node* n1 = l.find(10);
    ASSERT_NE(n1, nullptr);
    EXPECT_EQ(n1->data, 10);

    typename List<int>::Node* n2 = l.find(30);
    ASSERT_NE(n2, nullptr);
    EXPECT_EQ(n2->data, 30);

    typename List<int>::Node* n3 = l.find(50);
    ASSERT_NE(n3, nullptr);
    EXPECT_EQ(n3->data, 50);
}

TEST_F(ListFindTest, FindNonExistingReturnsNull)
{
    EXPECT_EQ(l.find(99), nullptr);
    EXPECT_EQ(l.find(-1), nullptr);
}

TEST_F(ListFindTest, FindFirstOccurrence)
{
    List<int> l2;
    l2.push_back(5);
    l2.push_back(10);
    l2.push_back(5);
    l2.push_back(10);

    typename List<int>::Node* n = l2.find(5);
    ASSERT_NE(n, nullptr);
    EXPECT_EQ(n, l2.find(5));
}

TEST(ListFindStandalone, FindInEmptyList)
{
    List<int> l;

    EXPECT_EQ(l.find(42), nullptr);
}

TEST(ListFindStandalone, FindSingleElement)
{
    List<int> l = {42};

    typename List<int>::Node* n = l.find(42);
    ASSERT_NE(n, nullptr);
    EXPECT_EQ(n->data, 42);

    EXPECT_EQ(l.find(99), nullptr);
}

TEST(ListFindStandalone, FindStringType)
{
    List<std::string> l = {"Apple", "Banana", "Cherry"};

    typename List<std::string>::Node* n = l.find("Banana");
    ASSERT_NE(n, nullptr);
    EXPECT_EQ(n->data, "Banana");

    EXPECT_EQ(l.find("Date"), nullptr);
}

TEST(ListFindStandalone, FindAfterRemove)
{
    List<int> l = {1, 2, 3, 4, 5};
    l.remove(2);

    EXPECT_EQ(l.find(3), nullptr);
}

TEST(ListFindStandalone, FindAfterInsert)
{
    List<int> l = {1, 2, 3};
    l.insert(1, 99);

    typename List<int>::Node* n = l.find(99);
    ASSERT_NE(n, nullptr);
    EXPECT_EQ(n->data, 99);
}

class ListContainsTest : public ::testing::Test {
protected:
    List<int> l{100, 200, 300, 400, 500};
};

TEST_F(ListContainsTest, ContainsExistingElement)
{
    EXPECT_TRUE(l.contains(100));
    EXPECT_TRUE(l.contains(300));
    EXPECT_TRUE(l.contains(500));
}

TEST_F(ListContainsTest, DoesNotContainMissingElement)
{
    EXPECT_FALSE(l.contains(0));
    EXPECT_FALSE(l.contains(150));
    EXPECT_FALSE(l.contains(999));
}

TEST(ListContainsStandalone, EmptyListContainsNothing)
{
    List<int> l;

    EXPECT_FALSE(l.contains(0));
    EXPECT_FALSE(l.contains(42));
}

TEST(ListContainsStandalone, ContainsSingleElement)
{
    List<int> l = {7};

    EXPECT_TRUE(l.contains(7));
    EXPECT_FALSE(l.contains(8));
}

TEST(ListContainsStandalone, ContainsAfterPush)
{
    List<int> l;
    l.push_back(42);

    EXPECT_TRUE(l.contains(42));
}

TEST(ListContainsStandalone, ContainsAfterPop)
{
    List<int> l = {1, 2, 3};
    l.pop_back();

    EXPECT_FALSE(l.contains(3));
    EXPECT_TRUE(l.contains(1));
    EXPECT_TRUE(l.contains(2));
}

TEST(ListContainsStandalone, ContainsAfterClear)
{
    List<int> l = {10, 20, 30};
    l.clear();

    EXPECT_FALSE(l.contains(10));
    EXPECT_FALSE(l.contains(20));
    EXPECT_FALSE(l.contains(30));
}

TEST(ListContainsStandalone, ContainsStringType)
{
    List<std::string> l = {"Hello", "World"};

    EXPECT_TRUE(l.contains("Hello"));
    EXPECT_TRUE(l.contains("World"));
    EXPECT_FALSE(l.contains("Foo"));
}

TEST(ListContainsStandalone, ContainsWithDuplicates)
{
    List<int> l = {5, 5, 5};

    EXPECT_TRUE(l.contains(5));
    EXPECT_FALSE(l.contains(6));
}
