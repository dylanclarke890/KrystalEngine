#include "Krystal.Maths/Round.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::Maths;

#define ROUND_TEST_CASE(func, value, expected)                                                               \
  REQUIRE(func(value) == (expected));                                                                        \
  REQUIRE(func(Vec4(value)) == (Vec4(expected)));                                                            \
  REQUIRE(func(Mat4x4(value)) == (Mat4x4(expected)));

  TEST_CASE("Ceil", "[Round]")
  {
    ROUND_TEST_CASE(Ceil, 0.0f, 0.0f);
    ROUND_TEST_CASE(Ceil, 0.499999f, 1.0f);
    ROUND_TEST_CASE(Ceil, 0.500001f, 1.0f);
    ROUND_TEST_CASE(Ceil, -0.499999f, 0.0f);
    ROUND_TEST_CASE(Ceil, -0.500001f, 0.0f);
    ROUND_TEST_CASE(Ceil, 1.0f, 1.0f);
    ROUND_TEST_CASE(Ceil, 12345.67f, 12346.0f);
    ROUND_TEST_CASE(Ceil, 16777216.0f, 16777216.0f);
    ROUND_TEST_CASE(Ceil, -16777216.0f, -16777216.0f);
    REQUIRE(Ceil(-16777216.0f) == -16777216.0f);
  }

  TEST_CASE("Floor", "[Round]")
  {
    ROUND_TEST_CASE(Floor, 0.0f, 0.0f);
    ROUND_TEST_CASE(Floor, 0.499999f, 0.0f);
    ROUND_TEST_CASE(Floor, 0.500001f, 0.0f);
    ROUND_TEST_CASE(Floor, -0.499999f, -1.0f);
    ROUND_TEST_CASE(Floor, -0.500001f, -1.0f);
    ROUND_TEST_CASE(Floor, 1.0f, 1.0f);
    ROUND_TEST_CASE(Floor, 12345.67f, 12345.0f);
    ROUND_TEST_CASE(Floor, 16777216.0f, 16777216.0f);
    ROUND_TEST_CASE(Floor, -16777216.0f, -16777216.0f);
  }

  TEST_CASE("Round", "[Round]")
  {
    ROUND_TEST_CASE(Round, 0.0f, 0.0f);
    ROUND_TEST_CASE(Round, -0.0f, -0.0f);
    ROUND_TEST_CASE(Round, 0.499999f, 0.0f);
    ROUND_TEST_CASE(Round, 0.500001f, 1.0f);
    ROUND_TEST_CASE(Round, -0.499999f, 0.0f);
    ROUND_TEST_CASE(Round, -0.500001f, -1.0f);
    ROUND_TEST_CASE(Round, 1.0f, 1.0f);
    ROUND_TEST_CASE(Round, 12345.67f, 12346.0f);
    ROUND_TEST_CASE(Round, 16777216.0f, 16777216.0f);
    ROUND_TEST_CASE(Round, -16777216.0f, -16777216.0f);
  }

  TEST_CASE("Trunc", "[Round]")
  {
    ROUND_TEST_CASE(Trunc, 0.0f, 0.0f);
    ROUND_TEST_CASE(Trunc, -0.0f, -0.0f);
    ROUND_TEST_CASE(Trunc, 0.499999f, 0.0f);
    ROUND_TEST_CASE(Trunc, 0.500001f, 0.0f);
    ROUND_TEST_CASE(Trunc, -0.499999f, 0.0f);
    ROUND_TEST_CASE(Trunc, -0.500001f, 0.0f);
    ROUND_TEST_CASE(Trunc, 1.0f, 1.0f);
    ROUND_TEST_CASE(Trunc, 12345.67f, 12345.0f);
    ROUND_TEST_CASE(Trunc, 16777216.0f, 16777216.0f);
    ROUND_TEST_CASE(Trunc, -16777216.0f, -16777216.0f);
  }
}