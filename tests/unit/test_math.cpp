#include <gtest/gtest.h>
#include <math/math.h>

class MathTest : public ::testing::Test {
protected:
	void SetUp() override
	{
		/* Initialize before each test */
	}

	void TearDown() override
	{
		/* Cleanup after each test */
	}
};

TEST_F(MathTest, AddTwoPositiveNumbers)
{
	int result = math_add(5, 3);
	EXPECT_EQ(8, result);
}

TEST_F(MathTest, AddZeroToNumber)
{
	int result = math_add(42, 0);
	EXPECT_EQ(42, result);
}

TEST_F(MathTest, AddNegativeNumbers)
{
	int result = math_add(-5, -3);
	EXPECT_EQ(-8, result);
}

TEST_F(MathTest, AddPositiveAndNegative)
{
	int result = math_add(10, -5);
	EXPECT_EQ(5, result);
}

TEST_F(MathTest, AddZeroAndZero)
{
	int result = math_add(0, 0);
	EXPECT_EQ(0, result);
}
