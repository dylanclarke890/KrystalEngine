#include "pch.hpp"

#include "Krystal.Maths/Quaternion.hpp"

#include <catch_all.hpp>
#include <utility>

namespace Krys::Tests
{
  using namespace Krys::Maths;

  TEST_CASE("Quaternion::new", "[Quaternion]")
  {
    constexpr Quaternion<float> empty {};
    REQUIRE(empty == Quaternion<float>(1, 0, 0, 0));

    constexpr Quaternion<float> copy(empty);
    REQUIRE(copy == empty);

    constexpr Quaternion<float> move_src(empty);
    constexpr Quaternion<float> move_dst(std::move(move_src));
    REQUIRE(move_dst == empty);
  }

  TEST_CASE("Quaternion::Equality", "[Quaternion]")
  {
    constexpr Quaternion<float> a(1, 2, 3, 4);
    constexpr Quaternion<float> b(1, 2, 3, 4);
    constexpr Quaternion<float> c(5, 6, 7, 8);

    REQUIRE(a == b);
    REQUIRE_FALSE(a == c);
    REQUIRE(a != c);
  }

  TEST_CASE("Quaternion::Addition", "[Quaternion]")
  {
    constexpr Quaternion<float> a(1, 2, 3, 4);
    constexpr Quaternion<float> b(5, 6, 7, 8);

    REQUIRE(a + b == Quaternion<float>(6, 8, 10, 12));
  }

  TEST_CASE("Quaternion::Subtraction", "[Quaternion]")
  {
    constexpr Quaternion<float> a(1, 2, 3, 4);
    constexpr Quaternion<float> b(5, 6, 7, 8);

    REQUIRE(a - b == Quaternion<float>(-4, -4, -4, -4));
  }

  TEST_CASE("Quaternion::Multiplication", "[Quaternion]")
  {
    constexpr Quaternion<float> a(1, 2, 3, 4);
    constexpr Quaternion<float> b(5, 6, 7, 8);
    constexpr float scalar = 10;

    REQUIRE(a * b == Quaternion<float>(-60, 12, 30, 24));
    REQUIRE(a * scalar == Quaternion<float>(10, 20, 30, 40));
  }

  TEST_CASE("Quaternion::Division", "[Quaternion]")
  {
    constexpr Quaternion<float> a(1, 2, 3, 4);
    constexpr float scalar = 10;

    REQUIRE(a / scalar == Quaternion<float>(0.1f, 0.2f, 0.3f, 0.4f));
  }
}