#include <gtest/gtest.h>
#include "List.h"
#include <string>


TEST(ListDefaultConstructor, CreatesEmptyList)
{
    List<int> l;

    EXPECT_TRUE(l.empty());
    EXPECT_EQ(l.size(), 0u);
}

TEST(ListDefaultConstructor, WorksWithStringType)
{
    List<std::string> l;

    EXPECT_TRUE(l.empty());
    EXPECT_EQ(l.size(), 0u);
}

class ListInitListTest : public ::testing::Test {
protected:
    List<int> l{10, 20, 30, 40, 50};
};

TEST_F(ListInitListTest, CorrectSize)
{
    EXPECT_EQ(l.size(), 5u);
}

TEST_F(ListInitListTest, CorrectElements)
{
    EXPECT_EQ(l.at(0), 10);
    EXPECT_EQ(l.at(1), 20);
    EXPECT_EQ(l.at(2), 30);
    EXPECT_EQ(l.at(3), 40);
    EXPECT_EQ(l.at(4), 50);
}

TEST_F(ListInitListTest, FrontAndBack)
{
    EXPECT_EQ(l.front(), 10);
    EXPECT_EQ(l.back(), 50);
}

TEST(ListInitListStandalone, EmptyInitializerList)
{
    List<int> l = {};

    EXPECT_TRUE(l.empty());
    EXPECT_EQ(l.size(), 0u);
}

TEST(ListInitListStandalone, SingleElement)
{
    List<int> l = {42};

    EXPECT_EQ(l.size(), 1u);
    EXPECT_EQ(l.front(), 42);
    EXPECT_EQ(l.back(), 42);
}

TEST(ListInitListStandalone, StringInitializerList)
{
    List<std::string> l = {"Hello", "World", "GTest"};

    EXPECT_EQ(l.size(), 3u);
    EXPECT_EQ(l.at(0), "Hello");
    EXPECT_EQ(l.at(1), "World");
    EXPECT_EQ(l.at(2), "GTest");
}

TEST(ListCopyConstructor, CopiesAllElements)
{
    List<int> original = {1, 2, 3, 4, 5};
    List<int> copy(original);

    EXPECT_EQ(copy.size(), original.size());
    for (size_t i = 0; i < original.size(); ++i) {
        EXPECT_EQ(copy.at(i), original.at(i));
    }
}

TEST(ListCopyConstructor, CopyIsIndependent)
{
    List<int> original = {10, 20, 30};
    List<int> copy(original);

    copy.at(0) = 999;

    EXPECT_EQ(original.at(0), 10);
    EXPECT_EQ(copy.at(0), 999);
}

TEST(ListCopyConstructor, CopyEmptyList)
{
    List<int> original;
    List<int> copy(original);

    EXPECT_TRUE(copy.empty());
    EXPECT_EQ(copy.size(), 0u);
}

TEST(ListCopyConstructor, CopyStringList)
{
    List<std::string> original = {"Alpha", "Beta", "Gamma"};
    List<std::string> copy(original);

    EXPECT_EQ(copy.size(), 3u);
    EXPECT_EQ(copy.at(0), "Alpha");
    EXPECT_EQ(copy.at(1), "Beta");
    EXPECT_EQ(copy.at(2), "Gamma");

    copy.at(1) = "Modified";
    EXPECT_EQ(original.at(1), "Beta");
}

TEST(ListCopyAssignment, AssignsAllElements)
{
    List<int> original = {5, 10, 15};
    List<int> assigned;
    assigned = original;

    EXPECT_EQ(assigned.size(), original.size());
    for (size_t i = 0; i < original.size(); ++i) {
        EXPECT_EQ(assigned.at(i), original.at(i));
    }
}

TEST(ListCopyAssignment, AssignedIsIndependent)
{
    List<int> original = {1, 2, 3};
    List<int> assigned;
    assigned = original;

    assigned.at(0) = 100;

    EXPECT_EQ(original.at(0), 1);
    EXPECT_EQ(assigned.at(0), 100);
}

TEST(ListCopyAssignment, SelfAssignment)
{
    List<int> l = {7, 8, 9};
    l = l;

    EXPECT_EQ(l.size(), 3u);
    EXPECT_EQ(l.at(0), 7);
    EXPECT_EQ(l.at(1), 8);
    EXPECT_EQ(l.at(2), 9);
}

TEST(ListCopyAssignment, AssignToNonEmpty)
{
    List<int> l1 = {1, 2, 3, 4, 5};
    List<int> l2 = {10, 20};
    l2 = l1;

    EXPECT_EQ(l2.size(), 5u);
    EXPECT_EQ(l2.at(0), 1);
    EXPECT_EQ(l2.at(4), 5);
}

TEST(ListCopyAssignment, AssignSmallerToLarger)
{
    List<int> l1 = {1, 2};
    List<int> l2 = {10, 20, 30, 40, 50};
    l2 = l1;

    EXPECT_EQ(l2.size(), 2u);
    EXPECT_EQ(l2.at(0), 1);
    EXPECT_EQ(l2.at(1), 2);
}

TEST(ListCopyAssignment, AssignEmptyList)
{
    List<int> l1;
    List<int> l2 = {1, 2, 3};
    l2 = l1;

    EXPECT_TRUE(l2.empty());
    EXPECT_EQ(l2.size(), 0u);
}

TEST(ListCopyAssignment, AssignStringList)
{
    List<std::string> l1 = {"X", "Y", "Z"};
    List<std::string> l2;
    l2 = l1;

    EXPECT_EQ(l2.size(), 3u);
    EXPECT_EQ(l2.at(0), "X");
    EXPECT_EQ(l2.at(2), "Z");
}
