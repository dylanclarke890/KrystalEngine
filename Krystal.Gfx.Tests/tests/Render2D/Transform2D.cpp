#include "Krystal.Gfx/Render2D/Transform2D.hpp"
#include "catch2/catch_test_macros.hpp"
#include "catch2/matchers/catch_matchers_floating_point.hpp"
#include <cmath>
#include <numbers>

namespace krys::gfx::tests
{
  namespace
  {
    KRYS_NODISCARD bool Near(Vec2f a, Vec2f b) noexcept
    {
      return std::abs(a.x - b.x) < 1e-5f && std::abs(a.y - b.y) < 1e-5f;
    }
  }

  TEST_CASE("Transform2D: maps points", "[Gfx][Render2D]")
  {
    CHECK(Near(Transform2D::Identity().Apply({3.f, 4.f}), {3.f, 4.f}));
    CHECK(Near(Transform2D::Translation(1.f, 2.f).Apply({3.f, 4.f}), {4.f, 6.f}));
    CHECK(Near(Transform2D::Scale(2.f, 3.f).Apply({3.f, 4.f}), {6.f, 12.f}));
    // y points down, so a quarter turn takes +x to +y: clockwise on screen.
    CHECK(Near(Transform2D::Rotation(std::numbers::pi_v<float> / 2.f).Apply({1.f, 0.f}), {0.f, 1.f}));
  }

  TEST_CASE("Transform2D: composes right to left", "[Gfx][Render2D]")
  {
    // Scale first, then translate.
    const auto transform = Transform2D::Translation(10.f, 0.f) * Transform2D::Scale(2.f, 2.f);
    CHECK(Near(transform.Apply({1.f, 1.f}), {12.f, 2.f}));
    CHECK(Near((Transform2D::Scale(2.f, 2.f) * Transform2D::Translation(10.f, 0.f)).Apply({1.f, 1.f}),
               {22.f, 2.f}));
    CHECK(transform * Transform2D::Identity() == transform);
    CHECK_THAT(transform.Determinant(), Catch::Matchers::WithinAbs(4.f, 1e-6f));
  }

  TEST_CASE("Transform2D: knows when rects stay axis-aligned", "[Gfx][Render2D]")
  {
    CHECK(Transform2D::Translation(1.f, 2.f).PreservesAxisAlignment());
    CHECK(Transform2D::Scale(-1.f, 2.f).PreservesAxisAlignment());
    CHECK(Transform2D {.A = 0.f, .B = 1.f, .C = -1.f, .D = 0.f}.PreservesAxisAlignment());
    CHECK_FALSE(Transform2D::Rotation(0.3f).PreservesAxisAlignment());
  }
}
