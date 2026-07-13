#include <gtest/gtest.h>
#include "Vector.h"
#include <string>

class VectorSubscriptTest : public ::testing::Test {
protected:
    Vector<int> v{10, 20, 30, 40, 50};
};

TEST_F(VectorSubscriptTest, AccessFirstElement)
{
    EXPECT_EQ(v[0], 10);
}

TEST_F(VectorSubscriptTest, AccessLastElement)
{
    EXPECT_EQ(v[4], 50);
}

TEST_F(VectorSubscriptTest, AccessMiddleElement)
{
    EXPECT_EQ(v[2], 30);
}

TEST_F(VectorSubscriptTest, ModifyViaSubscript)
{
    v[2] = 999;
    EXPECT_EQ(v[2], 999);
}

TEST(VectorSubscriptStandalone, ConstAccess)
{
    const Vector<int> v = {5, 10, 15};

    EXPECT_EQ(v[0], 5);
    EXPECT_EQ(v[1], 10);
    EXPECT_EQ(v[2], 15);
}


class VectorAtTest : public ::testing::Test {
protected:
    Vector<int> v{100, 200, 300};
};

TEST_F(VectorAtTest, AccessValidIndex)
{
    EXPECT_EQ(v.at(0), 100);
    EXPECT_EQ(v.at(1), 200);
    EXPECT_EQ(v.at(2), 300);
}

TEST_F(VectorAtTest, ModifyViaAt)
{
    v.at(1) = 999;
    EXPECT_EQ(v.at(1), 999);
}

TEST_F(VectorAtTest, AtOutOfRangeThrows)
{
    EXPECT_THROW(v.at(3), std::out_of_range);
    EXPECT_THROW(v.at(100), std::out_of_range);
}

TEST(VectorAtStandalone, AtOnEmptyVectorThrows)
{
    Vector<int> v;

    EXPECT_THROW(v.at(0), std::out_of_range);
}

TEST(VectorAtStandalone, ConstAtAccess)
{
    const Vector<int> v = {7, 14, 21};

    EXPECT_EQ(v.at(0), 7);
    EXPECT_EQ(v.at(1), 14);
    EXPECT_EQ(v.at(2), 21);
}

TEST(VectorAtStandalone, ConstAtThrows)
{
    const Vector<int> v = {1, 2};

    EXPECT_THROW(v.at(2), std::out_of_range);
}


TEST(VectorFront, FrontReturnsFirstElement)
{
    Vector<int> v = {42, 99, 7};

    EXPECT_EQ(v.front(), 42);
}

TEST(VectorFront, FrontModifiable)
{
    Vector<int> v = {1, 2, 3};
    v.front() = 100;

    EXPECT_EQ(v[0], 100);
}

TEST(VectorFront, FrontOnSingleElement)
{
    Vector<int> v = {77};

    EXPECT_EQ(v.front(), 77);
}

TEST(VectorFront, FrontOnEmptyThrows)
{
    Vector<int> v;

    EXPECT_THROW(v.front(), std::out_of_range);
}

TEST(VectorFront, ConstFront)
{
    const Vector<int> v = {10, 20};

    EXPECT_EQ(v.front(), 10);
}

TEST(VectorFront, ConstFrontOnEmptyThrows)
{
    const Vector<int> v = {};

    EXPECT_THROW(v.front(), std::out_of_range);
}

TEST(VectorFront, FrontStringType)
{
    Vector<std::string> v = {"First", "Second"};

    EXPECT_EQ(v.front(), "First");
}

TEST(VectorBack, BackReturnsLastElement)
{
    Vector<int> v = {10, 20, 30};

    EXPECT_EQ(v.back(), 30);
}

TEST(VectorBack, BackModifiable)
{
    Vector<int> v = {1, 2, 3};
    v.back() = 999;

    EXPECT_EQ(v[2], 999);
}

TEST(VectorBack, BackOnSingleElement)
{
    Vector<int> v = {55};

    EXPECT_EQ(v.back(), 55);
}

TEST(VectorBack, BackOnEmptyThrows)
{
    Vector<int> v;

    EXPECT_THROW(v.back(), std::out_of_range);
}

TEST(VectorBack, ConstBack)
{
    const Vector<int> v = {5, 10, 15};

    EXPECT_EQ(v.back(), 15);
}

TEST(VectorBack, ConstBackOnEmptyThrows)
{
    const Vector<int> v = {};

    EXPECT_THROW(v.back(), std::out_of_range);
}

TEST(VectorBack, FrontEqualsBackOnSingleElement)
{
    Vector<int> v = {42};

    EXPECT_EQ(v.front(), v.back());
}

TEST(VectorBack, BackStringType)
{
    Vector<std::string> v = {"X", "Y", "Z"};

    EXPECT_EQ(v.back(), "Z");
}
