#include "Krystal.Booey/Graphics/Transforms/Quaternion.hpp"
#include "catch2/catch_test_macros.hpp"
#include "catch2/matchers/catch_matchers_floating_point.hpp"
#include <cmath>
#include <numbers>

namespace krys::boo::tests
{
  using Catch::Matchers::WithinAbs;

  TEST_CASE("Quaternion", "[Graphics][Transforms][Quaternion]")
  {
    const double halfSqrt2 = std::sqrt(0.5);
    constexpr Quaternion identity {0, 0, 0, 1};
    // A 90 degree rotation about z.
    const Quaternion rotateZ {0, 0, halfSqrt2, halfSqrt2};

    SECTION("Slerp")
    {
      auto start = identity.Slerp(rotateZ, 0);
      CHECK_THAT(start.Z, WithinAbs(0, 1e-12));
      CHECK_THAT(start.W, WithinAbs(1, 1e-12));

      // Halfway is a 45 degree rotation about z.
      auto half = identity.Slerp(rotateZ, 0.5);
      CHECK_THAT(half.X, WithinAbs(0, 1e-12));
      CHECK_THAT(half.Z, WithinAbs(std::sin(std::numbers::pi / 8), 1e-12));
      CHECK_THAT(half.W, WithinAbs(std::cos(std::numbers::pi / 8), 1e-12));

      auto end = identity.Slerp(rotateZ, 1);
      CHECK_THAT(end.Z, WithinAbs(halfSqrt2, 1e-12));
      CHECK_THAT(end.W, WithinAbs(halfSqrt2, 1e-12));

      // The same rotation returns itself.
      CHECK(rotateZ.Slerp(rotateZ, 0.3) == rotateZ);

      // A negated quaternion is the same rotation: the start is negated onto the shorter arc, so halfway is
      // the negated 45 degree rotation.
      auto flipped = identity.Slerp(Quaternion {0, 0, -halfSqrt2, -halfSqrt2}, 0.5);
      CHECK_THAT(flipped.Z, WithinAbs(-std::sin(std::numbers::pi / 8), 1e-12));
      CHECK_THAT(flipped.W, WithinAbs(-std::cos(std::numbers::pi / 8), 1e-12));
    }

    SECTION("Accumulate")
    {
      CHECK(identity.Accumulate(rotateZ) == rotateZ);

      // Two 90 degree rotations are a 180 degree rotation.
      auto twice = rotateZ.Accumulate(rotateZ);
      CHECK_THAT(twice.Z, WithinAbs(1, 1e-12));
      CHECK_THAT(twice.W, WithinAbs(0, 1e-12));
    }

    SECTION("Interpolate")
    {
      CHECK(rotateZ.Interpolate(rotateZ, 0.5, CompositeOperation::Accumulate) == rotateZ.Accumulate(rotateZ));
      CHECK(identity.Interpolate(rotateZ, 0.5, CompositeOperation::Replace) == identity.Slerp(rotateZ, 0.5));
      CHECK(identity.Interpolate(rotateZ, 0.5, CompositeOperation::Add) == identity.Slerp(rotateZ, 0.5));
    }
  }
}
