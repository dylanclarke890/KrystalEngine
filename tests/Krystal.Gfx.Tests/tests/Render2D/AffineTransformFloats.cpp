#include "Krystal.Gfx/Render2D/AffineTransformFloats.hpp"
#include "catch2/catch_test_macros.hpp"
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

  TEST_CASE("AffineTransformFloats: maps points", "[Gfx][Render2D]")
  {
    CHECK(Near(MapPoint(AffineTransform(), {3.f, 4.f}), {3.f, 4.f}));
    CHECK(Near(MapPoint(AffineTransform::CreateTranslation(SizeF(1.f, 2.f)), {3.f, 4.f}), {4.f, 6.f}));
    CHECK(Near(MapPoint(AffineTransform::CreateScale(SizeF(2.f, 3.f)), {3.f, 4.f}), {6.f, 12.f}));
    // y points down, so a quarter turn takes +x to +y: clockwise on screen.
    CHECK(Near(MapPoint(AffineTransform().RotateRadians(std::numbers::pi / 2), {1.f, 0.f}), {0.f, 1.f}));

    // Composition applies the right-hand transform first: scale, then translate.
    const auto transform =
      AffineTransform::CreateTranslation(SizeF(10.f, 0.f)) * AffineTransform::CreateScale(SizeF(2.f, 2.f));
    CHECK(Near(MapPoint(transform, {1.f, 1.f}), {12.f, 2.f}));
  }

  TEST_CASE("AffineTransformFloats: the parts the GPU takes", "[Gfx][Render2D]")
  {
    const AffineTransform transform(1, 2, 3, 4, 5, 6);
    CHECK(LinearPartF(transform) == Array<float, 4> {1.f, 2.f, 3.f, 4.f});
    CHECK(TranslationF(transform) == Array<float, 2> {5.f, 6.f});
    CHECK(transform.Determinant() == -2);
  }
}
