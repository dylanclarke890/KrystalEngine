#include "Krystal.Gfx/Render2D/PathStroker.hpp"
#include "catch2/catch_test_macros.hpp"
#include "catch2/matchers/catch_matchers_floating_point.hpp"
#include "Krystal.Gfx/Render2D/PathRasteriser.hpp"
#include <numbers>

namespace krys::gfx::tests
{
  namespace
  {
    constexpr float Pi = std::numbers::pi_v<float>;
    constexpr float Tolerance = PathRasteriser::FlatteningTolerance;

    /// @brief The area the stroke covers, in square target pixels.
    KRYS_NODISCARD float StrokedArea(const Path2D &path, const StrokeStyle &style,
                                     const Transform2D &transform = {})
    {
      PathStroker stroker;
      PathRasteriser rasteriser;
      const auto outline = stroker.Stroke(path, style, transform);
      const auto mask = rasteriser.Rasterise(outline, transform, FillRule::NonZero,
                                             {.X = -1'000, .Y = -1'000, .Width = 2'000u, .Height = 2'000u});
      float sum = 0.f;
      for (const auto value : mask.Coverage)
      {
        sum += static_cast<float>(value);
      }
      return sum / 255.f;
    }

    KRYS_NODISCARD Path2D Line(Vec2f from, Vec2f to)
    {
      Path2D path;
      path.MoveTo(from);
      path.LineTo(to);
      return path;
    }

    KRYS_NODISCARD Path2D Square()
    {
      Path2D path;
      path.AddRect({0.f, 0.f, 10.f, 10.f});
      return path;
    }

    /// @brief Flattening cuts inside curves by up to the tolerance, so an area can fall short by up to the
    /// curved length times it.
    KRYS_NODISCARD auto WithinFlattening(float area, float curvedLength)
    {
      return Catch::Matchers::WithinAbs(area, curvedLength * Tolerance + 0.01f);
    }
  }

  TEST_CASE("PathStroker: caps open ends", "[Gfx][Render2D]")
  {
    const auto line = Line({0.f, 5.f}, {10.f, 5.f});
    CHECK(StrokedArea(line, {.Width = 2.f, .Cap = LineCap::Butt}) == 20.f);
    CHECK(StrokedArea(line, {.Width = 2.f, .Cap = LineCap::Square}) == 24.f);
    // Two half circles of radius 1.
    CHECK_THAT(StrokedArea(line, {.Width = 2.f, .Cap = LineCap::Round}),
               WithinFlattening(20.f + Pi, 2.f * Pi));
  }

  TEST_CASE("PathStroker: joins corners", "[Gfx][Render2D]")
  {
    // The 12-by-12 square outside less the 8-by-8 inside, less what each join cuts off the outer corners.
    const auto square = Square();
    CHECK(StrokedArea(square, {.Width = 2.f, .Join = LineJoin::Miter}) == 80.f);
    // Half-covered pixels store as 128 of 255, so diagonal cuts come out a touch over.
    CHECK_THAT(StrokedArea(square, {.Width = 2.f, .Join = LineJoin::Bevel}),
               Catch::Matchers::WithinAbs(78.f, 0.05f));
    CHECK_THAT(StrokedArea(square, {.Width = 2.f, .Join = LineJoin::Round}),
               WithinFlattening(80.f - 4.f + Pi, 2.f * Pi));

    // A right angle's miter is the square root of 2 times the width.
    CHECK_THAT(StrokedArea(square, {.Width = 2.f, .Join = LineJoin::Miter, .MiterLimit = 1.4f}),
               Catch::Matchers::WithinAbs(78.f, 0.05f));
    CHECK(StrokedArea(square, {.Width = 2.f, .Join = LineJoin::Miter, .MiterLimit = 1.5f}) == 80.f);

    // An open corner: two 10-by-2 strokes overlapping by one pixel, and the miter's one-pixel square.
    Path2D corner;
    corner.MoveTo({0.f, 0.f});
    corner.LineTo({10.f, 0.f});
    corner.LineTo({10.f, 10.f});
    CHECK(StrokedArea(corner, {.Width = 2.f}) == 40.f);
  }

  TEST_CASE("PathStroker: dashes", "[Gfx][Render2D]")
  {
    const auto line = Line({0.f, 5.f}, {10.f, 5.f});

    // Dashes over [0, 2), [4, 6) and [8, 10).
    CHECK(StrokedArea(line, {.Width = 2.f, .Dashes = {2.f, 2.f}}) == 12.f);
    // An odd pattern repeats to make it even: [2] is [2, 2].
    CHECK(StrokedArea(line, {.Width = 2.f, .Dashes = {2.f}}) == 12.f);
    // [3, 1, 1] is [3, 1, 1, 3, 1, 1]: dashes over [0, 3), [4, 5) and [8, 9).
    CHECK(StrokedArea(line, {.Width = 2.f, .Dashes = {3.f, 1.f, 1.f}}) == 10.f);

    // Offsets move into the pattern, either way: [0, 1), [3, 5), [7, 9); then [1, 3), [5, 7), [9, 10).
    CHECK(StrokedArea(line, {.Width = 2.f, .Dashes = {2.f, 2.f}, .DashOffset = 1.f}) == 10.f);
    CHECK(StrokedArea(line, {.Width = 2.f, .Dashes = {2.f, 2.f}, .DashOffset = -1.f}) == 10.f);

    // Zero-length dashes are dots under round and square caps, and nothing under butt caps.
    CHECK_THAT(StrokedArea(line, {.Width = 2.f, .Cap = LineCap::Round, .Dashes = {0.f, 4.f}}),
               WithinFlattening(3.f * Pi, 3.f * 2.f * Pi));
    CHECK(StrokedArea(line, {.Width = 2.f, .Cap = LineCap::Square, .Dashes = {0.f, 4.f}}) == 12.f);
    CHECK(StrokedArea(line, {.Width = 2.f, .Dashes = {0.f, 4.f}}) == 0.f);

    // Invalid patterns stroke solid.
    CHECK(StrokedArea(line, {.Width = 2.f, .Dashes = {-1.f, 2.f}}) == 20.f);
    CHECK(StrokedArea(line, {.Width = 2.f, .Dashes = {0.f, 0.f}}) == 20.f);

    // Closed contours dash across their closing segment: here the first and third sides.
    CHECK(StrokedArea(Square(), {.Width = 2.f, .Dashes = {10.f, 10.f}}) == 40.f);

    // A closed contour's last dash runs on into its first and is joined at the start, as at any other
    // corner: dashes over [30, 40) and [0, 5) of the perimeter meet at the start's corner. Each mitred
    // corner adds its outer square and overlaps its inner one, so the area is the dashes' alone.
    CHECK(StrokedArea(Square(), {.Width = 2.f, .Dashes = {15.f, 5.f}, .DashOffset = 10.f}) == 60.f);
    // A dash all the way round strokes the whole closed contour.
    CHECK(StrokedArea(Square(), {.Width = 2.f, .Dashes = {100.f, 5.f}})
          == StrokedArea(Square(), {.Width = 2.f}));
    // A first dash the last doesn't reach is stroked alone.
    CHECK(StrokedArea(Square(), {.Width = 2.f, .Dashes = {5.f, 15.f}}) == 20.f);
  }

  TEST_CASE("PathStroker: strokes curves", "[Gfx][Render2D]")
  {
    Path2D circle;
    circle.AddEllipse({20.f, 20.f}, {10.f, 10.f});
    CHECK_THAT(StrokedArea(circle, {.Width = 2.f}), WithinFlattening(Pi * (121.f - 81.f), 2.f * Pi * 20.f));

    // A stroke much wider than the curve is tight keeps its outer edge on the circle too.
    Path2D small;
    small.AddEllipse({20.f, 20.f}, {5.f, 5.f});
    CHECK_THAT(StrokedArea(small, {.Width = 8.f}), WithinFlattening(Pi * (81.f - 1.f), 2.f * Pi * 10.f));
  }

  TEST_CASE("PathStroker: strokes in the path's coordinates", "[Gfx][Render2D]")
  {
    const auto scale = Transform2D::Scale(10.f, 10.f);
    CHECK(StrokedArea(Line({0.f, 0.5f}, {1.f, 0.5f}), {.Width = 0.2f}, scale) == 20.f);

    // A hairline is one target pixel wide, whatever the scale.
    // Centred on a pixel boundary, it half covers two rows.
    CHECK_THAT(StrokedArea(Line({0.f, 5.f}, {10.f, 5.f}), {.Width = 0.f}),
               Catch::Matchers::WithinAbs(10.f, 0.1f));
    CHECK_THAT(StrokedArea(Line({0.f, 1.f}, {2.5f, 1.f}), {.Width = 0.f}, Transform2D::Scale(4.f, 4.f)),
               Catch::Matchers::WithinAbs(10.f, 0.1f));
  }

  TEST_CASE("PathStroker: strokes zero-length contours as dots", "[Gfx][Render2D]")
  {
    const auto point = Line({5.f, 5.f}, {5.f, 5.f});
    CHECK_THAT(StrokedArea(point, {.Width = 4.f, .Cap = LineCap::Round}),
               WithinFlattening(4.f * Pi, 4.f * Pi));
    CHECK(StrokedArea(point, {.Width = 4.f, .Cap = LineCap::Square}) == 16.f);
    CHECK(StrokedArea(point, {.Width = 4.f}) == 0.f);

    // A move alone draws nothing.
    Path2D move;
    move.MoveTo({5.f, 5.f});
    PathStroker stroker;
    CHECK(stroker.Stroke(move, {.Width = 4.f, .Cap = LineCap::Round}, {}).IsEmpty());
  }
}
