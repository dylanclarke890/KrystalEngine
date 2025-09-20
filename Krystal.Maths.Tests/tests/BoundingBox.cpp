#include "pch.hpp"

#include "Krystal.Maths/BoundingBox.hpp"

namespace Krys::Tests
{
  using namespace Krys::Maths;

  TEST_CASE("BoundingBox", "[BoundingBox]")
  {
    constexpr BoundingBox<float> defaultBox;
    REQUIRE(defaultBox.Left == 0.0f);
    REQUIRE(defaultBox.Right == 0.0f);
    REQUIRE(defaultBox.Bottom == 0.0f);
    REQUIRE(defaultBox.Top == 0.0f);
    REQUIRE(defaultBox.GetWidth() == 0.0f);
    REQUIRE(defaultBox.GetHeight() == 0.0f);

    constexpr BoundingBox<float> paramBox(-1.0f, 1.0f, -2.0f, 2.0f);
    REQUIRE(paramBox.Left == -1.0f);
    REQUIRE(paramBox.Right == 1.0f);
    REQUIRE(paramBox.Bottom == -2.0f);
    REQUIRE(paramBox.Top == 2.0f);
    REQUIRE(paramBox.GetWidth() == 2.0f);
    REQUIRE(paramBox.GetHeight() == 4.0f);

    constexpr BoundingBox<float> zeroBox(1.0f, 1.0f, 2.0f, 2.0f);
    REQUIRE(zeroBox.GetWidth() == 0.0f);
    REQUIRE(zeroBox.GetHeight() == 0.0f);

    constexpr BoundingBox<float> negBox(2.0f, -2.0f, 3.0f, -3.0f);
    REQUIRE(negBox.Left == 2.0f);
    REQUIRE(negBox.Right == -2.0f);
    REQUIRE(negBox.Bottom == 3.0f);
    REQUIRE(negBox.Top == -3.0f);
    REQUIRE(negBox.GetWidth() == -4.0f);
    REQUIRE(negBox.GetHeight() == -6.0f);
  }
}
