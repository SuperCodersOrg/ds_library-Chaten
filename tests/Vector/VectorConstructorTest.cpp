#include <gtest/gtest.h>
#include "Vector.h"
#include <string>


TEST(VectorDefaultConstructor, CreatesEmptyVector)
{
    Vector<int> v;

    EXPECT_TRUE(v.empty());
    EXPECT_EQ(v.size(), 0u);
}

TEST(VectorDefaultConstructor, HasInitialCapacity)
{
    Vector<int> v;

    EXPECT_GE(v.capacity(), 1u);
}

TEST(VectorDefaultConstructor, WorksWithStringType)
{
    Vector<std::string> v;

    EXPECT_TRUE(v.empty());
    EXPECT_EQ(v.size(), 0u);
}

class VectorInitListTest : public ::testing::Test {
protected:
    Vector<int> v{10, 20, 30, 40, 50};
};

TEST_F(VectorInitListTest, CorrectSize)
{
    EXPECT_EQ(v.size(), 5u);
}

TEST_F(VectorInitListTest, CorrectElements)
{
    EXPECT_EQ(v[0], 10);
    EXPECT_EQ(v[1], 20);
    EXPECT_EQ(v[2], 30);
    EXPECT_EQ(v[3], 40);
    EXPECT_EQ(v[4], 50);
}

TEST_F(VectorInitListTest, CapacityMatchesSize)
{
    EXPECT_EQ(v.capacity(), v.size());
}

TEST(VectorInitListStandalone, EmptyInitializerList)
{
    Vector<int> v = {};

    EXPECT_TRUE(v.empty());
    EXPECT_EQ(v.size(), 0u);
    EXPECT_GE(v.capacity(), 1u);
}

TEST(VectorInitListStandalone, SingleElement)
{
    Vector<int> v = {42};

    EXPECT_EQ(v.size(), 1u);
    EXPECT_EQ(v[0], 42);
}

TEST(VectorInitListStandalone, StringInitializerList)
{
    Vector<std::string> v = {"Hello", "World", "GTest"};

    EXPECT_EQ(v.size(), 3u);
    EXPECT_EQ(v[0], "Hello");
    EXPECT_EQ(v[1], "World");
    EXPECT_EQ(v[2], "GTest");
}

TEST(VectorCopyConstructor, CopiesAllElements)
{
    Vector<int> original = {1, 2, 3, 4, 5};
    Vector<int> copy(original);

    EXPECT_EQ(copy.size(), original.size());
    for (size_t i = 0; i < original.size(); ++i) {
        EXPECT_EQ(copy[i], original[i]);
    }
}

TEST(VectorCopyConstructor, CopyIsIndependent)
{
    Vector<int> original = {10, 20, 30};
    Vector<int> copy(original);

    copy[0] = 999;

    EXPECT_EQ(original[0], 10);
    EXPECT_EQ(copy[0], 999);
}

TEST(VectorCopyConstructor, CopyEmptyVector)
{
    Vector<int> original;
    Vector<int> copy(original);

    EXPECT_TRUE(copy.empty());
    EXPECT_EQ(copy.size(), 0u);
}

TEST(VectorCopyConstructor, CopyStringVector)
{
    Vector<std::string> original = {"Alpha", "Beta", "Gamma"};
    Vector<std::string> copy(original);

    EXPECT_EQ(copy.size(), 3u);
    EXPECT_EQ(copy[0], "Alpha");
    EXPECT_EQ(copy[1], "Beta");
    EXPECT_EQ(copy[2], "Gamma");

    copy[1] = "Modified";
    EXPECT_EQ(original[1], "Beta");
}

TEST(VectorCopyAssignment, AssignsAllElements)
{
    Vector<int> original = {5, 10, 15};
    Vector<int> assigned;
    assigned = original;

    EXPECT_EQ(assigned.size(), original.size());
    for (size_t i = 0; i < original.size(); ++i) {
        EXPECT_EQ(assigned[i], original[i]);
    }
}

TEST(VectorCopyAssignment, AssignedIsIndependent)
{
    Vector<int> original = {1, 2, 3};
    Vector<int> assigned;
    assigned = original;

    assigned[0] = 100;

    EXPECT_EQ(original[0], 1);
    EXPECT_EQ(assigned[0], 100);
}

TEST(VectorCopyAssignment, SelfAssignment)
{
    Vector<int> v = {7, 8, 9};
    v = v;

    EXPECT_EQ(v.size(), 3u);
    EXPECT_EQ(v[0], 7);
    EXPECT_EQ(v[1], 8);
    EXPECT_EQ(v[2], 9);
}

TEST(VectorCopyAssignment, AssignToNonEmpty)
{
    Vector<int> v1 = {1, 2, 3, 4, 5};
    Vector<int> v2 = {10, 20};
    v2 = v1;

    EXPECT_EQ(v2.size(), 5u);
    EXPECT_EQ(v2[0], 1);
    EXPECT_EQ(v2[4], 5);
}

TEST(VectorCopyAssignment, AssignSmallerToLarger)
{
    Vector<int> v1 = {1, 2};
    Vector<int> v2 = {10, 20, 30, 40, 50};
    v2 = v1;

    EXPECT_EQ(v2.size(), 2u);
    EXPECT_EQ(v2[0], 1);
    EXPECT_EQ(v2[1], 2);
}

TEST(VectorCopyAssignment, AssignEmptyVector)
{
    Vector<int> v1;
    Vector<int> v2 = {1, 2, 3};
    v2 = v1;

    EXPECT_TRUE(v2.empty());
    EXPECT_EQ(v2.size(), 0u);
}

TEST(VectorCopyAssignment, AssignStringVector)
{
    Vector<std::string> v1 = {"X", "Y", "Z"};
    Vector<std::string> v2;
    v2 = v1;

    EXPECT_EQ(v2.size(), 3u);
    EXPECT_EQ(v2[0], "X");
    EXPECT_EQ(v2[2], "Z");
}
