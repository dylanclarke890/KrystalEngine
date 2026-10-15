#include "Krystal.Booey/Graphics/Path.hpp"
#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/Graphics/FloatRoundedRect.hpp"
#include "Krystal.Booey/Graphics/PathTraversalState.hpp"
#include "Krystal.Booey/Graphics/Transforms/AffineTransform.hpp"
#include <cmath>
#include <numbers>

namespace krys::boo::tests
{
  using Catch::Approx;
  using ElementType = PathElement::Type;

  namespace
  {
    KRYS_NODISCARD List<ElementType> ElementTypes(const Path &path) noexcept
    {
      List<ElementType> types;
      path.ApplyElements([&](const PathElement &element) { types.push_back(element.ElementType); });
      return types;
    }
  }

  TEST_CASE("Path", "[Graphics][Path]")
  {
    Path empty;
    CHECK(empty.IsEmpty());
    CHECK(empty.BoundingRect() == FloatRect {});
    CHECK(empty.CurrentPoint() == FloatPoint {0, 0});
    CHECK_FALSE(empty.IsClosed());

    SECTION("a single line")
    {
      Path line;
      line.MoveTo({0, 0});
      line.AddLineTo({30, 40});
      CHECK_FALSE(line.IsEmpty());
      CHECK(line.DefinitelySingleLine());
      REQUIRE(line.SingleDataLine());
      CHECK(line.SingleDataLine()->Start() == FloatPoint {0, 0});
      CHECK(line.SingleDataLine()->End() == FloatPoint {30, 40});
      CHECK(line.CurrentPoint() == FloatPoint {30, 40});
      CHECK(line.Length() == Approx(50));
      CHECK(line.PointAtLength(25).X() == Approx(15));
      CHECK(line.PointAtLength(25).Y() == Approx(20));
      CHECK(line.BoundingRect() == FloatRect {0, 0, 30, 40});
      CHECK(ElementTypes(line) == List<ElementType> {ElementType::MoveToPoint, ElementType::AddLineToPoint});
    }

    SECTION("a closed polyline")
    {
      Path triangle;
      triangle.MoveTo({0, 0});
      triangle.AddLineTo({10, 0});
      triangle.AddLineTo({10, 10});
      triangle.CloseSubpath();
      CHECK(triangle.IsClosed());
      CHECK_FALSE(triangle.DefinitelySingleLine());
      CHECK(triangle.BoundingRect() == FloatRect {0, 0, 10, 10});
      CHECK(triangle.FastBoundingRect() == FloatRect {0, 0, 10, 10});
      CHECK(ElementTypes(triangle)
            == List<ElementType> {ElementType::MoveToPoint, ElementType::AddLineToPoint,
                                  ElementType::AddLineToPoint, ElementType::CloseSubpath});
      // Around the three sides: 10 + 10 + sqrt(200).
      CHECK(triangle.Length() == Approx(20 + std::sqrt(200.0)));

      // The same polygon from points.
      Path polygon {List<FloatPoint> {{0, 0}, {10, 0}, {10, 10}}};
      CHECK(polygon.IsClosed());
      CHECK(polygon.BoundingRect() == FloatRect {0, 0, 10, 10});
      CHECK(polygon.DefinitelyEqual(triangle));
    }

    SECTION("curves")
    {
      Path quad;
      quad.MoveTo({0, 0});
      quad.AddQuadCurveTo({50, 100}, {100, 0});
      // The curve peaks at half its control point's height.
      CHECK(quad.BoundingRect().Height() == Approx(50));
      // The fast bounding rect includes the control point.
      CHECK(quad.FastBoundingRect().Height() == Approx(100));

      Path cubic;
      cubic.MoveTo({0, 0});
      cubic.AddBezierCurveTo({0, 100}, {100, 100}, {100, 0});
      CHECK(cubic.BoundingRect().Height() == Approx(75));
      CHECK(ElementTypes(cubic)
            == List<ElementType> {ElementType::MoveToPoint, ElementType::AddCurveToPoint});
    }

    SECTION("shapes")
    {
      Path rect;
      rect.AddRect({5, 5, 20, 10});
      CHECK(rect.BoundingRect() == FloatRect {5, 5, 20, 10});

      Path ellipse;
      ellipse.AddEllipseInRect({0, 0, 20, 10});
      CHECK(ellipse.BoundingRect() == FloatRect {0, 0, 20, 10});

      Path rounded;
      rounded.AddRoundedRect(FloatRoundedRect {FloatRect {0, 0, 40, 20}, FloatRoundedRect::Radii {5}},
                             PathRoundedRect::Strategy::PreferBezier);
      CHECK(rounded.BoundingRect() == FloatRect {0, 0, 40, 20});
      CHECK(FloatRoundedRect {FloatRect {0, 0, 40, 20}, FloatRoundedRect::Radii {5}}.GetPath().BoundingRect()
            == FloatRect {0, 0, 40, 20});

      // Radii that don't fit make a plain rect.
      Path oversized;
      oversized.AddRoundedRect(FloatRoundedRect {FloatRect {0, 0, 10, 10}, FloatRoundedRect::Radii {20}});
      REQUIRE(oversized.SingleSegment());
      CHECK(std::holds_alternative<PathRect>(oversized.SingleSegment()->GetData()));

      Path arc;
      arc.AddArc({0, 0}, 10, 0, std::numbers::pi_v<float>, RotationDirection::Clockwise);
      CHECK(arc.BoundingRect().Width() == Approx(20));
    }

    SECTION("transforms")
    {
      Path path;
      path.MoveTo({0, 0});
      path.AddLineTo({10, 10});
      path.AddLineTo({20, 0});

      auto copy = path;
      path.Translate({5, 5});
      CHECK(path.BoundingRect() == FloatRect {5, 5, 20, 10});
      // Copies don't see each other's changes.
      CHECK(copy.BoundingRect() == FloatRect {0, 0, 20, 10});

      path.Transform(AffineTransform().Scale(2));
      CHECK(path.BoundingRect() == FloatRect {10, 10, 40, 20});

      Path combined;
      combined.AddRect({0, 0, 1, 1});
      combined.AddPath(copy, AffineTransform::MakeTranslation({100, 0}));
      CHECK(combined.BoundingRect() == FloatRect {0, 0, 120, 10});

      combined.Clear();
      CHECK(combined.IsEmpty());
    }
  }
}
