#include "Krystal.Booey/Graphics/Path.hpp"
#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Graphics/TestPathStrokeStrategy.hpp"
#include "Krystal.Booey/Graphics/FloatRoundedRect.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
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

  TEST_CASE("Path: contains", "[Graphics][Path]")
  {
    CHECK_FALSE(Path().Contains({0, 0}));

    Path rect;
    rect.AddRect({10, 10, 20, 20});
    CHECK(rect.Contains({15, 15}));
    CHECK_FALSE(rect.Contains({5, 15}));
    CHECK_FALSE(rect.Contains({35, 15}));

    SECTION("an open subpath is filled as if closed")
    {
      Path triangle;
      triangle.MoveTo({0, 0});
      triangle.AddLineTo({40, 0});
      triangle.AddLineTo({0, 40});
      CHECK(triangle.Contains({10, 10}));
      CHECK_FALSE(triangle.Contains({30, 30}));
    }

    SECTION("wind rules")
    {
      // Two rects wound the same way: the inner one is filled with nonzero, a hole with evenodd.
      Path nested;
      nested.AddRect({0, 0, 40, 40});
      nested.AddRect({10, 10, 20, 20});
      CHECK(nested.Contains({20, 20}));
      CHECK_FALSE(nested.Contains({20, 20}, WindRule::EvenOdd));
      CHECK(nested.Contains({5, 5}, WindRule::EvenOdd));
    }

    SECTION("curves")
    {
      Path ellipse;
      ellipse.AddEllipseInRect({0, 0, 40, 20});
      CHECK(ellipse.Contains({20, 10}));
      CHECK(ellipse.Contains({2, 10}));
      CHECK_FALSE(ellipse.Contains({2, 2}));

      Path rounded;
      rounded.AddRoundedRect(FloatRoundedRect({0, 0, 40, 40}, FloatRoundedRect::Radii(10)));
      CHECK(rounded.Contains({20, 1}));
      CHECK_FALSE(rounded.Contains({1, 1}));
    }
  }

  TEST_CASE("Path: strokes", "[Graphics][Path]")
  {
    Path line;
    line.MoveTo({10, 10});
    line.AddLineTo({50, 10});

    auto applier = [](GraphicsContext &context)
    {
      context.SetStrokeThickness(4);
      context.SetLineCap(LineCap::Round);
      context.SetLineJoin(LineJoin::Bevel);
      context.SetMiterLimit(2);
      context.SetLineDash(DashArray {3, 1}, 2);
    };

    SECTION("without the host's stroker, nothing is inside a stroke and its bounds are estimated")
    {
      CHECK_FALSE(line.StrokeContains({30, 10}, applier));
      CHECK(line.StrokeBoundingRect(applier) == FloatRect(8, 8, 44, 4));
      // Without an applier, the stroke is a solid one 1 wide.
      CHECK(line.StrokeBoundingRect() == FloatRect(9.5f, 9.5f, 41, 1));
    }

    SECTION("the host's stroker is asked with the style the applier set")
    {
      PathStrokeStrategyScope scope;
      CHECK(line.StrokeContains({30, 11}, applier));
      CHECK_FALSE(line.StrokeContains({30, 13}, applier));
      const auto &style = scope.Strategy.LastStyle;
      CHECK(style.Thickness == 4);
      CHECK(style.Cap == LineCap::Round);
      CHECK(style.Join == LineJoin::Bevel);
      CHECK(style.MiterLimit == 2);
      CHECK(style.Dashes == DashArray {3, 1});
      CHECK(style.DashOffset == 2);

      CHECK(line.StrokeBoundingRect() == FloatRect(9.5f, 9.5f, 41, 1));
      CHECK(scope.Strategy.LastStyle.Dashes.isEmpty());

      // An empty path has no stroke.
      CHECK_FALSE(Path().StrokeContains({0, 0}, applier));
      CHECK(Path().StrokeBoundingRect(applier) == FloatRect());
    }
  }

  TEST_CASE("Path: transforming segments that can't be transformed themselves", "[Graphics][Path]")
  {
    // A rect under a transform isn't a rect, so the path is rebuilt from its elements.
    Path rect;
    rect.AddRect({10, 8, 20, 48});
    rect.Transform(AffineTransform().Scale(3, 1));
    CHECK(rect.BoundingRect() == FloatRect(30, 8, 60, 48));
    CHECK(ElementTypes(rect)
          == List<ElementType> {ElementType::MoveToPoint, ElementType::AddLineToPoint,
                                ElementType::AddLineToPoint, ElementType::AddLineToPoint,
                                ElementType::CloseSubpath});

    Path ellipse;
    ellipse.MoveTo({0, 0});
    ellipse.AddEllipseInRect({0, 0, 10, 10});
    ellipse.Transform(AffineTransform::MakeTranslation({5, 5}));
    CHECK(ellipse.BoundingRect() == FloatRect(5, 5, 10, 10));
  }

  TEST_CASE("Path: arcs without a platform path", "[Graphics][Path]")
  {
    constexpr float pi = std::numbers::pi_v<float>;

    SECTION("an arc is a move and beziers of at most a quarter turn")
    {
      Path circle;
      circle.AddArc({50, 50}, 10, 0, 2 * pi, RotationDirection::Clockwise);
      CHECK(ElementTypes(circle)
            == List<ElementType> {ElementType::MoveToPoint, ElementType::AddCurveToPoint,
                                  ElementType::AddCurveToPoint, ElementType::AddCurveToPoint,
                                  ElementType::AddCurveToPoint});
      CHECK(circle.Contains({50, 50}));
      CHECK(circle.Contains({59, 50}));
      CHECK_FALSE(circle.Contains({59, 59}));

      // The end point is y-down: a quarter turn clockwise from the right is at the bottom.
      Path quarter;
      quarter.AddArc({0, 0}, 10, 0, pi / 2, RotationDirection::Clockwise);
      CHECK(quarter.CurrentPoint().X() == Approx(0).margin(1e-4));
      CHECK(quarter.CurrentPoint().Y() == Approx(10));
    }

    SECTION("after a current point, an arc starts with a line")
    {
      Path path;
      path.MoveTo({0, 0});
      path.AddArc({20, 0}, 5, pi, 0, RotationDirection::Counterclockwise);
      auto types = ElementTypes(path);
      REQUIRE(types.size() >= 3u);
      CHECK(types[0] == ElementType::MoveToPoint);
      CHECK(types[1] == ElementType::AddLineToPoint);
      CHECK(types[2] == ElementType::AddCurveToPoint);
      // With y down, anticlockwise from the left to the right goes under the bottom.
      CHECK(path.Contains({20, 3}));
      CHECK_FALSE(path.Contains({20, -3}));
    }

    SECTION("arcTo rounds the corner between its lines")
    {
      Path path;
      path.MoveTo({0, 0});
      path.AddLineTo({20, 0});
      path.AddArcTo({40, 0}, {40, 40}, 10);
      CHECK(path.CurrentPoint().X() == Approx(40));
      CHECK(path.CurrentPoint().Y() == Approx(10));
      path.AddLineTo({40, 40});
      path.AddLineTo({0, 40});
      path.CloseSubpath();
      // The corner is cut off.
      CHECK(path.Contains({35, 5}));
      CHECK_FALSE(path.Contains({39.5f, 0.5f}));

      // Parallel lines make a line to the corner.
      Path straight;
      straight.MoveTo({0, 0});
      straight.AddArcTo({10, 0}, {20, 0}, 5);
      CHECK(straight.CurrentPoint() == FloatPoint(10, 0));
    }

    SECTION("rotated ellipses rotate about their centres, in radians")
    {
      Path ellipse;
      ellipse.AddEllipse({50, 50}, 20, 5, pi / 2, 0, 2 * pi, RotationDirection::Clockwise);
      CHECK(ellipse.Contains({50, 65}));
      CHECK_FALSE(ellipse.Contains({65, 50}));
      CHECK(ellipse.CurrentPoint().X() == Approx(50).margin(1e-3));
      CHECK(ellipse.CurrentPoint().Y() == Approx(70));
    }

    SECTION("transforming an arc keeps it")
    {
      Path circle;
      circle.AddArc({0, 0}, 10, 0, 2 * pi, RotationDirection::Clockwise);
      circle.Transform(AffineTransform::MakeTranslation({100, 0}));
      CHECK(circle.Contains({100, 0}));
      CHECK_FALSE(circle.Contains({0, 0}));
    }

    SECTION("adding a path to an empty one")
    {
      Path rect;
      rect.AddRect({0, 0, 10, 10});
      Path path;
      path.AddPath(rect, AffineTransform::MakeTranslation({5, 0}));
      CHECK(path.FastBoundingRect() == FloatRect(5, 0, 10, 10));
    }
  }
}
