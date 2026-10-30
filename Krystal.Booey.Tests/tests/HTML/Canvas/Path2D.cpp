#include "Krystal.Booey/HTML/Canvas/Path2D.hpp"
#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"
#include <numbers>

namespace krys::boo::tests
{
  using Catch::Approx;
  using html::CanvasPath;
  using html::Path2D;

  TEST_CASE("Path2D: lines, curves and rectangles", "[HTML][Canvas][Path2D]")
  {
    auto path = Path2D::Create();
    CHECK(path->GetPath().IsEmpty());

    // A line from an empty path starts the subpath at its point.
    path->LineTo(10, 10);
    CHECK(path->GetPath().CurrentPoint() == FloatPoint(10, 10));
    path->LineTo(30, 10);
    path->LineTo(30, 30);
    path->ClosePath();
    CHECK(path->GetPath().Contains({25, 15}));
    CHECK_FALSE(path->GetPath().Contains({15, 25}));

    // Non-finite values are ignored.
    path->MoveTo(std::numeric_limits<float>::infinity(), 0);
    path->LineTo(std::nanf(""), 3);
    CHECK(path->GetPath().FastBoundingRect() == FloatRect(10, 10, 20, 20));

    // A rectangle with no size is a move.
    auto rects = Path2D::Create();
    rects->Rect(5, 5, 0, 0);
    CHECK(rects->GetPath().CurrentPoint() == FloatPoint(5, 5));
    rects->Rect(0, 0, 10, 20);
    CHECK(rects->GetPath().FastBoundingRect() == FloatRect(0, 0, 10, 20));

    // Curves from an empty path start at their first control point.
    auto curve = Path2D::Create();
    curve->QuadraticCurveTo(0, 0, 10, 10);
    curve->BezierCurveTo(20, 0, 30, 20, 40, 10);
    CHECK(curve->GetPath().CurrentPoint() == FloatPoint(40, 10));
  }

  TEST_CASE("Path2D: arcs and ellipses", "[HTML][Canvas][Path2D]")
  {
    SECTION("arc")
    {
      auto path = Path2D::Create();
      REQUIRE_FALSE(path->Arc(50, 50, 10, 0, 2 * std::numbers::pi_v<float>).HasException());
      CHECK(path->GetPath().Contains({50, 50}));
      CHECK_FALSE(path->GetPath().Contains({61, 50}));
      auto bounds = path->GetPath().BoundingRect();
      CHECK(bounds.X() == Approx(40).margin(0.01));
      CHECK(bounds.Width() == Approx(20).margin(0.01));

      // A negative radius throws; an empty arc draws only the line to its start.
      CHECK(path->Arc(0, 0, -1, 0, 1).GetException().Code() == dom::ExceptionCode::IndexSizeError);
      auto empty = Path2D::Create();
      REQUIRE_FALSE(empty->Arc(20, 20, 5, 1, 1).HasException());
      CHECK(empty->GetPath().CurrentPoint().X() == Approx(20 + 5 * std::cos(1.f)));
    }

    SECTION("arcTo")
    {
      auto path = Path2D::Create();
      // From an empty path, it moves to the first point.
      REQUIRE_FALSE(path->ArcTo(0, 0, 10, 0, 5).HasException());
      CHECK(path->GetPath().CurrentPoint() == FloatPoint(0, 0));
      path->LineTo(20, 0);
      REQUIRE_FALSE(path->ArcTo(40, 0, 40, 20, 10).HasException());
      CHECK(path->GetPath().CurrentPoint().X() == Approx(40));
      CHECK(path->GetPath().CurrentPoint().Y() == Approx(10));
      // A zero radius is a line.
      REQUIRE_FALSE(path->ArcTo(40, 40, 0, 40, 0).HasException());
      CHECK(path->GetPath().CurrentPoint() == FloatPoint(40, 40));
      CHECK(path->ArcTo(0, 0, 1, 1, -1).GetException().Code() == dom::ExceptionCode::IndexSizeError);
    }

    SECTION("ellipse")
    {
      auto path = Path2D::Create();
      REQUIRE_FALSE(path->Ellipse(50, 50, 20, 10, 0, 0, 2 * std::numbers::pi_v<float>).HasException());
      auto bounds = path->GetPath().FastBoundingRect();
      CHECK(bounds.Width() == Approx(40).margin(0.01));
      CHECK(bounds.Height() == Approx(20).margin(0.01));
      CHECK(path->Ellipse(0, 0, -1, 1, 0, 0, 1).GetException().Code() == dom::ExceptionCode::IndexSizeError);

      // A flat ellipse is lines through its quarter points.
      auto flat = Path2D::Create();
      REQUIRE_FALSE(flat->Ellipse(0, 0, 10, 0, 0, 0, std::numbers::pi_v<float>).HasException());
      CHECK(flat->GetPath().CurrentPoint().X() == Approx(-10));
    }
  }

  TEST_CASE("Path2D: rounded rectangles", "[HTML][Canvas][Path2D]")
  {
    auto path = Path2D::Create();
    REQUIRE_FALSE(path->RoundRect(0, 0, 100, 50, CanvasPath::RadiusVariant(10.0)).HasException());
    CHECK(path->GetPath().FastBoundingRect() == FloatRect(0, 0, 100, 50));
    // The corner is cut off; the new subpath starts at (x, y).
    CHECK_FALSE(path->GetPath().Contains({1, 1}));
    CHECK(path->GetPath().Contains({50, 25}));
    CHECK(path->GetPath().CurrentPoint() == FloatPoint(0, 0));

    // One to four radii; more or fewer throw, as do negative ones.
    List<CanvasPath::RadiusVariant> five(5, CanvasPath::RadiusVariant(1.0));
    CHECK(path->RoundRect(0, 0, 10, 10, Span<const CanvasPath::RadiusVariant>(five)).GetException().Code()
          == dom::ExceptionCode::RangeError);
    CHECK(path->RoundRect(0, 0, 10, 10, Span<const CanvasPath::RadiusVariant>()).GetException().Code()
          == dom::ExceptionCode::RangeError);
    CHECK(path->RoundRect(0, 0, 10, 10, CanvasPath::RadiusVariant(-1.0)).GetException().Code()
          == dom::ExceptionCode::RangeError);
    CHECK(path->RoundRect(0, 0, 10, 10, CanvasPath::RadiusVariant(dom::DOMPointInit {.X = 1, .Y = -1}))
            .GetException()
            .Code()
          == dom::ExceptionCode::RangeError);

    // A non-finite radius adds nothing.
    auto unchanged = Path2D::Create();
    REQUIRE_FALSE(
      unchanged->RoundRect(0, 0, 10, 10, CanvasPath::RadiusVariant(std::numeric_limits<double>::infinity()))
        .HasException());
    CHECK(unchanged->GetPath().IsEmpty());

    // Negative sizes go the other way round, so the winding still fills.
    auto flipped = Path2D::Create();
    REQUIRE_FALSE(flipped->RoundRect(100, 50, -100, -50, CanvasPath::RadiusVariant(5.0)).HasException());
    CHECK(flipped->GetPath().FastBoundingRect() == FloatRect(0, 0, 100, 50));
    CHECK(flipped->GetPath().Contains({50, 25}));
  }

  TEST_CASE("Path2D: SVG path data and adding paths", "[HTML][Canvas][Path2D]")
  {
    auto triangle = Path2D::Create(u8"M 0 0 L 10 0 L 10 10 Z");
    CHECK(triangle->GetPath().FastBoundingRect() == FloatRect(0, 0, 10, 10));
    CHECK(Path2D::Create(u8"not a path")->GetPath().IsEmpty());

    auto copy = Path2D::Create(*triangle);
    CHECK(copy->GetPath().FastBoundingRect() == triangle->GetPath().FastBoundingRect());

    auto combined = Path2D::Create();
    REQUIRE_FALSE(combined->AddPath(*triangle, {.E = 100}).HasException());
    CHECK(combined->GetPath().FastBoundingRect() == FloatRect(100, 0, 10, 10));

    // The matrix's long names count too.
    auto scaled = Path2D::Create();
    REQUIRE_FALSE(scaled->AddPath(*triangle, {.M11 = 2, .M22 = 3}).HasException());
    CHECK(scaled->GetPath().FastBoundingRect() == FloatRect(0, 0, 20, 30));

    CHECK(combined->AddPath(*triangle, {.A = 1, .M11 = 2}).GetException().Code()
          == dom::ExceptionCode::TypeError);
  }
}
