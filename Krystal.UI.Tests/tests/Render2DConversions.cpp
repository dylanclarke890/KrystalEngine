#include "Krystal.UI/Render2DConversions.hpp"
#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/Graphics/Gradient.hpp"
#include "Krystal.Booey/Graphics/Path.hpp"
#include <numbers>

namespace krys::ui::tests
{
  namespace
  {
    KRYS_NODISCARD bool Near(Vec2f a, Vec2f b, float tolerance = 1e-3f)
    {
      return std::abs(a.x - b.x) <= tolerance && std::abs(a.y - b.y) <= tolerance;
    }
  }

  TEST_CASE("Render2D conversions: colours", "[UI]")
  {
    CHECK(ToColour(boo::Color(boo::Color::red)) == SRGBA<uint8> {255, 0, 0, 255});
    CHECK(ToColour(boo::Color(SRGBA<uint8> {0, 0, 255, 200}), 0.5f) == SRGBA<uint8> {0, 0, 255, 100});
    // An invalid colour is transparent.
    CHECK(ToColour(boo::Color()) == SRGBA<uint8> {});
    // Other spaces are converted to sRGB.
    CHECK(ToColour(boo::Color(SRGBA<float> {1.f, 0.5f, 0.f, 1.f})) == SRGBA<uint8> {255, 128, 0, 255});
  }

  TEST_CASE("Render2D conversions: rounded rects", "[UI]")
  {
    const boo::FloatRoundedRect circular({0, 0, 20, 10}, {2, 2}, {3, 3}, {4, 4}, {5, 5});
    auto rect = ToCircularRoundedRect(circular);
    REQUIRE(rect.has_value());
    CHECK(rect->Rect == gfx::Rect2D {0, 0, 20, 10});
    // Booey's radii are top-left, top-right, bottom-left, bottom-right.
    CHECK(rect->Radii == gfx::CornerRadii2D {.TopLeft = 2, .TopRight = 3, .BottomRight = 5, .BottomLeft = 4});

    const boo::FloatRoundedRect elliptical({0, 0, 20, 10}, {2, 4}, {3, 3}, {4, 4}, {5, 5});
    CHECK_FALSE(ToCircularRoundedRect(elliptical).has_value());
  }

  TEST_CASE("Render2D conversions: paths", "[UI]")
  {
    SECTION("Lines and curves keep their points")
    {
      boo::Path path;
      path.MoveTo({1, 2});
      path.AddLineTo({10, 2});
      path.AddQuadCurveTo({10, 10}, {1, 10});
      path.AddBezierCurveTo({0, 8}, {0, 4}, {1, 2});
      path.CloseSubpath();
      const auto converted = ToPath2D(path);
      CHECK(converted.Verbs().size() == 5u);
      CHECK(converted.Verbs()[2] == gfx::PathVerb::QuadTo);
      CHECK(converted.Verbs()[4] == gfx::PathVerb::Close);
      CHECK(converted.Points()[0] == Vec2f {1, 2});
      CHECK(converted.Points()[1] == Vec2f {10, 2});
    }

    SECTION("An arc to a corner turns at the corner, tangent to both lines")
    {
      boo::Path path;
      path.MoveTo({0, 0});
      path.AddArcTo({10, 0}, {10, 10}, 4);
      const auto converted = ToPath2D(path);
      // A line to the first tangent point, then the quarter circle to the second.
      REQUIRE(converted.Verbs()[1] == gfx::PathVerb::LineTo);
      CHECK(Near(converted.Points()[1], {6, 0}));
      CHECK(Near(converted.Points().back(), {10, 4}));
    }

    SECTION("An arc to a straight line is a line to the corner")
    {
      boo::Path path;
      path.MoveTo({0, 0});
      path.AddArcTo({10, 0}, {20, 0}, 4);
      const auto converted = ToPath2D(path);
      REQUIRE(converted.Verbs().size() == 2u);
      CHECK(converted.Points()[1] == Vec2f {10, 0});
    }

    SECTION("Arcs run clockwise or anticlockwise from their start angle")
    {
      boo::Path clockwise;
      clockwise.AddArc({10, 10}, 5, 0, std::numbers::pi_v<float> / 2, boo::RotationDirection::Clockwise);
      auto converted = ToPath2D(clockwise);
      CHECK(Near(converted.Points().front(), {15, 10}));
      CHECK(Near(converted.Points().back(), {10, 15}));

      boo::Path anticlockwise;
      anticlockwise.AddArc({10, 10}, 5, 0, std::numbers::pi_v<float> / 2,
                           boo::RotationDirection::Counterclockwise);
      converted = ToPath2D(anticlockwise);
      CHECK(Near(converted.Points().back(), {10, 15}));
      // The long way round passes through the top.
      bool passesTop = false;
      for (const auto point : converted.Points())
      {
        passesTop = passesTop || point.y < 6.f;
      }
      CHECK(passesTop);
    }

    SECTION("A rotated ellipse is rotated about its centre")
    {
      boo::Path path;
      path.AddEllipse({10, 10}, 8, 2, std::numbers::pi_v<float> / 2, 0, 2 * std::numbers::pi_v<float>,
                      boo::RotationDirection::Clockwise);
      const auto converted = ToPath2D(path);
      // The long axis is now vertical: it starts 8 below the centre.
      CHECK(Near(converted.Points().front(), {10, 18}));
    }

    SECTION("Rects and rounded rects become closed contours")
    {
      boo::Path path;
      path.AddRect({0, 0, 10, 10});
      path.AddRoundedRect(boo::FloatRoundedRect({20, 0, 10, 10}, {2, 4}, {2, 4}, {2, 4}, {2, 4}));
      const auto converted = ToPath2D(path);
      size_t closes = 0;
      for (const auto verb : converted.Verbs())
      {
        closes += verb == gfx::PathVerb::Close ? 1u : 0u;
      }
      CHECK(closes == 2u);
    }
  }

  TEST_CASE("Render2D conversions: gradients", "[UI]")
  {
    auto gradient =
      boo::Gradient::Create(boo::Gradient::RadialData {{1, 2}, {3, 4}, 5, 6, 0.5f},
                            {ColourInterpolationMethod::OKLab {}, AlphaPremultiplication::Premultiplied},
                            GradientSpreadMethod::Reflect);
    gradient->AddColorStop({0.25f, boo::Color::red});
    gradient->AddColorStop({1, boo::Color(SRGBA<uint8> {0, 0, 255, 128})});

    const auto converted = ToGradient2D(*gradient, 0.5f);
    const auto &radial = std::get<gfx::RadialGradient2D>(converted.Shape);
    CHECK(radial.Point0 == Vec2f {1, 2});
    CHECK(radial.Point1 == Vec2f {3, 4});
    CHECK(radial.StartRadius == 5);
    CHECK(radial.EndRadius == 6);
    CHECK(radial.AspectRatio == 0.5f);
    CHECK(converted.Spread == GradientSpreadMethod::Reflect);
    CHECK(std::holds_alternative<ColourInterpolationMethod::OKLab>(converted.Interpolation.ColourSpace));
    REQUIRE(converted.Stops.size() == 2u);
    CHECK(converted.Stops[0].Offset == 0.25f);
    CHECK(converted.Stops[0].Colour.Resolved().Red == Catch::Approx(1.f));
    // The context's alpha multiplies each stop's.
    CHECK(converted.Stops[0].Colour.Resolved().Alpha == Catch::Approx(0.5f));
    CHECK(converted.Stops[1].Colour.Resolved().Alpha == Catch::Approx(0.25f).margin(0.01f));
  }
}
