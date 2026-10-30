#include "Krystal.Gfx/Render2D/PathRasteriser.hpp"
#include "catch2/catch_test_macros.hpp"
#include "catch2/matchers/catch_matchers_floating_point.hpp"
#include <cmath>
#include <limits>
#include <numbers>

namespace krys::gfx::tests
{
  namespace
  {
    constexpr PixelRect2D Everywhere {.X = -1'000, .Y = -1'000, .Width = 2'000u, .Height = 2'000u};
    constexpr float Pi = std::numbers::pi_v<float>;

    /// @brief The area the mask covers, in square pixels.
    KRYS_NODISCARD float Area(const CoverageMask &mask)
    {
      float sum = 0.f;
      for (const auto value : mask.Coverage)
      {
        sum += static_cast<float>(value);
      }
      return sum / 255.f;
    }

    /// @brief Flattening cuts inside convex curves by up to the tolerance, so a curved shape's area can fall
    /// short by up to its curved length times the tolerance.
    KRYS_NODISCARD auto WithinFlattening(float area, float curvedLength)
    {
      return Catch::Matchers::WithinAbs(area, curvedLength * PathRasteriser::FlatteningTolerance);
    }

    KRYS_NODISCARD Path2D Rect(float x, float y, float width, float height)
    {
      Path2D path;
      path.AddRect({x, y, width, height});
      return path;
    }

    KRYS_NODISCARD CoverageMask Rasterise(const Path2D &path, FillRule rule = FillRule::NonZero,
                                          const Transform2D &transform = {},
                                          const PixelRect2D &clip = Everywhere)
    {
      PathRasteriser rasteriser;
      return rasteriser.Rasterise(path, transform, rule, clip);
    }
  }

  TEST_CASE("PathRasteriser: covers whole pixels exactly", "[Gfx][Render2D]")
  {
    auto mask = Rasterise(Rect(1.f, 1.f, 2.f, 3.f));
    CHECK(mask.Bounds == PixelRect2D {.X = 1, .Y = 1, .Width = 2u, .Height = 3u});
    for (const auto value : mask.Coverage)
    {
      CHECK(value == 255u);
    }
  }

  TEST_CASE("PathRasteriser: covers partial pixels by area", "[Gfx][Render2D]")
  {
    SECTION("Exactly across a row")
    {
      auto mask = Rasterise(Rect(0.5f, 0.f, 2.f, 1.f));
      CHECK(mask.Bounds == PixelRect2D {.X = 0, .Y = 0, .Width = 3u, .Height = 1u});
      CHECK(mask.At(0, 0) == 128u);
      CHECK(mask.At(1, 0) == 255u);
      CHECK(mask.At(2, 0) == 128u);
    }

    SECTION("In sixteenths down a column")
    {
      // Twelve of the sixteen sub-scanlines are inside.
      auto mask = Rasterise(Rect(0.f, 0.25f, 1.f, 0.75f));
      CHECK(mask.At(0, 0) == 191u);
    }

    SECTION("Diagonal edges")
    {
      Path2D triangle;
      triangle.MoveTo({0.f, 0.f});
      triangle.LineTo({8.f, 0.f});
      triangle.LineTo({0.f, 8.f});
      CHECK_THAT(Area(Rasterise(triangle)), Catch::Matchers::WithinAbs(32.f, 0.05f));
    }
  }

  TEST_CASE("PathRasteriser: follows the fill rule", "[Gfx][Render2D]")
  {
    // Two squares wound the same way, one inside the other.
    Path2D nested;
    nested.AddRect({0.f, 0.f, 4.f, 4.f});
    nested.AddRect({1.f, 1.f, 2.f, 2.f});
    CHECK(Rasterise(nested, FillRule::NonZero).At(2, 2) == 255u);
    CHECK(Rasterise(nested, FillRule::EvenOdd).At(2, 2) == 0u);
    CHECK(Rasterise(nested, FillRule::EvenOdd).At(0, 0) == 255u);

    // The inner square wound the other way cancels the outer one under both rules.
    Path2D reversed;
    reversed.AddRect({0.f, 0.f, 4.f, 4.f});
    reversed.MoveTo({1.f, 1.f});
    reversed.LineTo({1.f, 3.f});
    reversed.LineTo({3.f, 3.f});
    reversed.LineTo({3.f, 1.f});
    reversed.Close();
    CHECK(Rasterise(reversed, FillRule::NonZero).At(2, 2) == 0u);
    CHECK(Rasterise(reversed, FillRule::NonZero).At(0, 0) == 255u);
  }

  TEST_CASE("PathRasteriser: flattens curves", "[Gfx][Render2D]")
  {
    SECTION("Quadratic")
    {
      // The area between a parabola and its chord is two thirds of the base times the height.
      Path2D path;
      path.MoveTo({0.f, 0.f});
      path.QuadTo({4.f, 8.f}, {8.f, 0.f});
      path.Close();
      CHECK_THAT(Area(Rasterise(path)), Catch::Matchers::WithinAbs(2.f / 3.f * 8.f * 4.f, 0.1f));
    }

    SECTION("Ellipses")
    {
      Path2D circle;
      circle.AddEllipse({25.f, 25.f}, {20.f, 20.f});
      CHECK_THAT(Area(Rasterise(circle)), WithinFlattening(Pi * 400.f, 2.f * Pi * 20.f));

      Path2D ellipse;
      ellipse.AddEllipse({30.f, 20.f}, {25.f, 10.f});
      // Ramanujan's approximation of the perimeter.
      const float perimeter = Pi * (3.f * 35.f - std::sqrt((3.f * 25.f + 10.f) * (25.f + 3.f * 10.f)));
      CHECK_THAT(Area(Rasterise(ellipse)), WithinFlattening(Pi * 250.f, perimeter));
    }

    SECTION("Rounded rects with elliptical corners")
    {
      // Each corner removes a square of its radii less a quarter of its ellipse.
      Path2D path;
      path.AddRoundedRect({0.f, 0.f, 40.f, 30.f},
                          {Vec2f {10.f, 6.f}, Vec2f {10.f, 6.f}, Vec2f {10.f, 6.f}, Vec2f {10.f, 6.f}});
      const float expected = 40.f * 30.f - 4.f * (1.f - Pi / 4.f) * 10.f * 6.f;
      // The four corners together are one ellipse's perimeter.
      const float corners = Pi * (3.f * 16.f - std::sqrt((3.f * 10.f + 6.f) * (10.f + 3.f * 6.f)));
      CHECK_THAT(Area(Rasterise(path)), WithinFlattening(expected, corners));
    }

    SECTION("Arcs")
    {
      Path2D half;
      half.AddArc({20.f, 20.f}, {10.f, 10.f}, 0.f, Pi);
      half.Close();
      CHECK_THAT(Area(Rasterise(half)), WithinFlattening(Pi * 100.f / 2.f, Pi * 10.f));

      // A sweep of a full turn or more is the whole circle, in either direction.
      Path2D full;
      full.AddArc({20.f, 20.f}, {10.f, 10.f}, 1.f, 1.f - 3.f * Pi, true);
      CHECK_THAT(Area(Rasterise(full)), WithinFlattening(Pi * 100.f, 2.f * Pi * 10.f));
    }
  }

  TEST_CASE("PathRasteriser: maps the path into pixels and clips it", "[Gfx][Render2D]")
  {
    auto transformed = Rasterise(Rect(0.f, 0.f, 1.f, 1.f), FillRule::NonZero,
                                 Transform2D::Translation(1.f, 1.f) * Transform2D::Scale(3.f, 2.f));
    CHECK(transformed.Bounds == PixelRect2D {.X = 1, .Y = 1, .Width = 3u, .Height = 2u});
    CHECK(Area(transformed) == 6.f);

    // Edges outside the clip still decide what's inside it.
    constexpr PixelRect2D clip {.X = 2, .Y = 2, .Width = 3u, .Height = 3u};
    auto clipped = Rasterise(Rect(0.f, 0.f, 8.f, 8.f), FillRule::NonZero, {}, clip);
    CHECK(clipped.Bounds == clip);
    CHECK(Area(clipped) == 9.f);

    CHECK(
      Rasterise(Rect(0.f, 0.f, 8.f, 8.f), FillRule::NonZero, {}, {.X = 20, .Y = 0, .Width = 4u, .Height = 4u})
        .Bounds.IsEmpty());
  }

  TEST_CASE("PathRasteriser: covers nothing for empty and invalid paths", "[Gfx][Render2D]")
  {
    CHECK(Rasterise(Path2D {}).Bounds.IsEmpty());

    Path2D line;
    line.MoveTo({1.f, 1.f});
    line.LineTo({5.f, 5.f});
    CHECK(Area(Rasterise(line)) == 0.f);

    Path2D invalid;
    invalid.MoveTo({0.f, 0.f});
    invalid.LineTo({std::numeric_limits<float>::quiet_NaN(), 4.f});
    invalid.LineTo({0.f, 4.f});
    CHECK(Rasterise(invalid).Bounds.IsEmpty());
  }
}
