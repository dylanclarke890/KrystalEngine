#include "Krystal.Booey/SVG/SVGGeometryElement.hpp"
#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Graphics/TestPathStrokeStrategy.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/SVG/SVGCircleElement.hpp"
#include "Krystal.Booey/SVG/SVGEllipseElement.hpp"
#include "Krystal.Booey/SVG/SVGLineElement.hpp"
#include "Krystal.Booey/SVG/SVGNames.hpp"
#include "Krystal.Booey/SVG/SVGPoint.hpp"
#include "Krystal.Booey/SVG/SVGPointList.hpp"
#include "Krystal.Booey/SVG/SVGPolygonElement.hpp"
#include "Krystal.Booey/SVG/SVGPolylineElement.hpp"
#include "Krystal.Booey/SVG/SVGRect.hpp"
#include "Krystal.Booey/SVG/SVGRectElement.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include <numbers>

namespace krys::boo::svg::tests
{
  using boo::tests::PathStrokeStrategyScope;
  using Catch::Approx;
  using style::tests::ResolverFixture;

  namespace
  {
    template <typename T>
    KRYS_NODISCARD T &ById(dom::Document &document, dom::DOMStringView id) noexcept
    {
      auto element = document.GetElementById(id);
      krys_assert(element);
      return Downcast<T>(Downcast<SVGElement>(*element));
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "SVGGeometryElement: the shapes' attributes",
                   "[SVG][SVGGeometryElement]")
  {
    auto document = ResolverFixture::Parse(
      u8R"html(<svg><rect id=rect x=1 y=2 width=30% height=4 rx=5 ry=-6 /><circle id=circle cx=1 cy=2 r=3 />)html"
      u8R"html(<ellipse id=ellipse cx=1 cy=2 rx=3 ry=4 /><line id=line x1=1 y1=2 x2=3 y2=4em />)html"
      u8R"html(<polygon id=polygon points="0,0 10,0 10 10" /><polyline id=polyline points="1 2 3" /></svg>)html");

    auto &rect = ById<SVGRectElement>(*document, u8"rect");
    CHECK(rect.X().ValueAsString() == u8"1");
    CHECK(rect.Width().ValueAsString() == u8"30%");
    CHECK(rect.Rx().ValueAsString() == u8"5");
    // Negative radii are reported as errors, but kept (the ry property ignores them).
    CHECK(rect.Ry().ValueAsString() == u8"-6");
    CHECK(rect.PropertyForAttribute(SVGNames::WidthAttr()) == &rect.WidthAnimated());
    CHECK(rect.PropertyForAttribute(SVGNames::CxAttr()) == nullptr);

    auto &circle = ById<SVGCircleElement>(*document, u8"circle");
    CHECK(circle.R().ValueAsString() == u8"3");
    CHECK(circle.PropertyForAttribute(SVGNames::RAttr()) == &circle.RAnimated());

    auto &ellipse = ById<SVGEllipseElement>(*document, u8"ellipse");
    CHECK(ellipse.Ry().ValueAsString() == u8"4");

    // A line has relative lengths only when its coordinates do.
    auto &line = ById<SVGLineElement>(*document, u8"line");
    CHECK(line.Y2().ValueAsString() == u8"4em");
    CHECK(line.HasRelativeLengths());
    REQUIRE_FALSE(line.SetAttribute(u8"y2", u8"4").HasException());
    CHECK_FALSE(line.HasRelativeLengths());

    // An odd number of coordinates drops the last; the points are kept up to an error.
    auto &polygon = ById<SVGPolygonElement>(*document, u8"polygon");
    CHECK(polygon.Points().NumberOfItems() == 3);
    CHECK(polygon.Points().ValueAsString() == u8"0 0 10 0 10 10");
    auto &polyline = ById<SVGPolylineElement>(*document, u8"polyline");
    CHECK(polyline.Points().ValueAsString() == u8"1 2");
    REQUIRE_FALSE(polyline.SetAttribute(u8"points", u8"5,6 7,8").HasException());
    CHECK(polyline.Points().NumberOfItems() == 2);

    auto &pathLength = rect.PathLengthAnimated();
    CHECK(pathLength.BaseVal() == 0);
    REQUIRE_FALSE(rect.SetAttribute(u8"pathLength", u8" 12.5px").HasException());
    CHECK(rect.PathLength() == 12.5f);
    REQUIRE_FALSE(rect.SetAttribute(u8"pathLength", u8"x").HasException());
    CHECK(rect.PathLength() == 0);
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "SVGGeometryElement: lengths and points",
                   "[SVG][SVGGeometryElement]")
  {
    ResolverFixture fixture {
      u8R"html(<!DOCTYPE html><body><svg width=200 height=100><rect id=rect x=10 y=10 width=30 height=40 />)html"
      u8R"html(<circle id=circle cx=50 cy=50 r=10 /><line id=line x1=0 y1=0 x2=30 y2=40 stroke=black />)html"
      u8R"html(<polyline id=polyline points="0,0 10,0 10,10" fill=none stroke=black stroke-width=2 />)html"
      u8R"html(</svg><svg style="display: none"><rect id=unrendered width=10 height=10 /></svg></body>)html"};

    // The perimeter of the shape's path.
    CHECK(ById<SVGRectElement>(*fixture.Document, u8"rect").GetTotalLength() == Approx(140));
    CHECK(ById<SVGCircleElement>(*fixture.Document, u8"circle").GetTotalLength()
          == Approx(20 * std::numbers::pi_v<float>).epsilon(0.01));
    auto &line = ById<SVGLineElement>(*fixture.Document, u8"line");
    CHECK(line.GetTotalLength() == Approx(50));

    // Points along it, clamped to its length.
    auto point = line.GetPointAtLength(25);
    REQUIRE_FALSE(point.HasException());
    CHECK(point.Value()->Value().X() == Approx(15));
    CHECK(point.Value()->Value().Y() == Approx(20));
    auto end = line.GetPointAtLength(100);
    REQUIRE_FALSE(end.HasException());
    CHECK(end.Value()->Value() == FloatPoint(30, 40));
    auto start = line.GetPointAtLength(-5);
    REQUIRE_FALSE(start.HasException());
    CHECK(start.Value()->Value() == FloatPoint(0, 0));

    // An unrendered shape has no length, and asking for a point throws.
    auto &unrendered = ById<SVGRectElement>(*fixture.Document, u8"unrendered");
    CHECK(unrendered.GetTotalLength() == 0);
    CHECK(unrendered.GetPointAtLength(1).HasException());

    // Hit testing in the shape's own coordinates, whatever its fill and stroke.
    auto &rect = ById<SVGRectElement>(*fixture.Document, u8"rect");
    CHECK(rect.IsPointInFill({20, 20}));
    CHECK_FALSE(rect.IsPointInFill({5, 20}));
    CHECK_FALSE(rect.IsPointInStroke({10, 20}));
    CHECK_FALSE(unrendered.IsPointInFill({5, 5}));

    auto &polyline = ById<SVGPolylineElement>(*fixture.Document, u8"polyline");
    CHECK(polyline.IsPointInFill({8, 2}));
    {
      PathStrokeStrategyScope scope;
      CHECK(polyline.IsPointInStroke({5, 0.5f}));
      CHECK_FALSE(polyline.IsPointInStroke({5, 2}));
    }

    // getBBox is the object bounding box, in the element's coordinates.
    auto bbox = rect.GetBBox();
    CHECK(bbox == FloatRect(10, 10, 30, 40));
    CHECK(unrendered.GetBBox() == FloatRect());
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "SVGGeometryElement: changes", "[SVG][SVGGeometryElement]")
  {
    ResolverFixture fixture {
      u8R"html(<!DOCTYPE html><body><svg width=200 height=100><rect id=rect x=10 y=10 width=30 height=40 />)html"
      u8R"html(<line id=line x1=0 y1=0 x2=30 y2=40 stroke=black />)html"
      u8R"html(<polygon id=polygon points="0,0 10,0 10,10" />)html"
      u8R"html(<path id=path d="M 0 0 L 100 0" stroke=black stroke-dasharray="10 5" pathLength=10 /></svg></body>)html"};

    // Presentation attributes change the style, and the shape follows.
    auto &rect = ById<SVGRectElement>(*fixture.Document, u8"rect");
    REQUIRE_FALSE(rect.SetAttribute(u8"width", u8"50").HasException());
    CHECK(rect.GetBBox() == FloatRect(10, 10, 50, 40));
    REQUIRE_FALSE(rect.SetAttribute(u8"style", u8"x: 20px").HasException());
    CHECK(rect.GetBBox() == FloatRect(20, 10, 50, 40));

    // Other attributes update the shape directly.
    auto &line = ById<SVGLineElement>(*fixture.Document, u8"line");
    REQUIRE_FALSE(line.SetAttribute(u8"x2", u8"60").HasException());
    CHECK(line.GetBBox() == FloatRect(0, 0, 60, 40));
    auto &polygon = ById<SVGPolygonElement>(*fixture.Document, u8"polygon");
    REQUIRE_FALSE(polygon.SetAttribute(u8"points", u8"0,0 20,0 20,30").HasException());
    CHECK(polygon.GetBBox() == FloatRect(0, 0, 20, 30));

    // pathLength scales the dashes by the path's length over it.
    auto &path = ById<SVGGeometryElement>(*fixture.Document, u8"path");
    PathStrokeStrategyScope scope;
    (void)path.IsPointInStroke({50, 0});
    CHECK(scope.Strategy.LastStyle.Dashes == DashArray {100, 50});
  }
}
