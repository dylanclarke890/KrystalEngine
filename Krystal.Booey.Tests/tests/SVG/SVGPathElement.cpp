#include "Krystal.Booey/SVG/SVGPathElement.hpp"
#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/SVG/Internals/SVGPathUtilities.hpp"
#include "Krystal.Booey/SVG/SVGPoint.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo::svg::tests
{
  using Catch::Approx;
  using style::tests::ResolverFixture;

  namespace
  {
    KRYS_NODISCARD SVGPathElement &PathById(dom::Document &document, dom::DOMStringView id) noexcept
    {
      auto element = document.GetElementById(id);
      krys_assert(element);
      return Downcast<SVGPathElement>(Downcast<SVGElement>(*element));
    }

    KRYS_NODISCARD dom::DOMString AsString(const SVGPathByteStream &stream) noexcept
    {
      dom::DOMString result;
      (void)BuildStringFromByteStream(stream, result, PathParsingMode::UnalteredParsing);
      return result;
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "SVGPathElement: the path data", "[SVG][SVGPathElement]")
  {
    SECTION("from the d attribute, without a renderer")
    {
      auto document = ResolverFixture::Parse(u8R"html(<svg><path id=p d="M 10 10 h 20 v 20" /></svg>)html");
      auto &path = PathById(*document, u8"p");
      CHECK(AsString(path.PathByteStream()) == u8"M 10 10 h 20 v 20");
      CHECK(path.GetPath().BoundingRect() == FloatRect(10, 10, 20, 20));

      // Path data up to an error is kept.
      REQUIRE_FALSE(path.SetAttribute(u8"d", u8"M 0 0 L 5 5 X 1").HasException());
      CHECK(AsString(path.PathByteStream()) == u8"M 0 0 L 5 5");
      REQUIRE_FALSE(path.SetAttribute(u8"d", u8"").HasException());
      CHECK(path.PathByteStream().IsEmpty());
    }

    SECTION("from the d property, once rendered")
    {
      ResolverFixture fixture {
        u8R"html(<!DOCTYPE html><body><svg width=200 height=100><path id=p d="M 10 10 h 20 v 20" />)html"
        u8R"html(<path id=q d="M 0 0 h 5" style='d: path("M 1 1 L 2 2")' /></svg></body>)html"};
      (void)fixture.Document->UpdateLayout();

      // The attribute is a presentation attribute for the d property.
      auto &p = PathById(*fixture.Document, u8"p");
      REQUIRE(p.Renderer());
      CHECK(AsString(p.PathByteStream()) == u8"M 10 10 h 20 v 20");

      // The property overrides it.
      auto &q = PathById(*fixture.Document, u8"q");
      CHECK(AsString(q.PathByteStream()) == u8"M 1 1 L 2 2");
      CHECK(q.GetBBox() == FloatRect(1, 1, 1, 1));

      // Changing the attribute changes the shape.
      REQUIRE_FALSE(p.SetAttribute(u8"d", u8"M 0 0 L 40 30").HasException());
      CHECK(p.GetBBox() == FloatRect(0, 0, 40, 30));
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "SVGPathElement: lengths and points", "[SVG][SVGPathElement]")
  {
    auto document =
      ResolverFixture::Parse(u8R"html(<svg><path id=p d="M 0 0 l 30 40 h 10" /><path id=empty /></svg>)html");

    // Measured on the path data, rendered or not.
    auto &path = PathById(*document, u8"p");
    CHECK(path.GetTotalLength() == Approx(60));
    auto point = path.GetPointAtLength(55);
    REQUIRE_FALSE(point.HasException());
    CHECK(point.Value()->Value() == FloatPoint(35, 40));

    // An empty path has no length, and asking for a point throws.
    auto &empty = PathById(*document, u8"empty");
    CHECK(empty.GetTotalLength() == 0);
    CHECK(empty.GetPointAtLength(0).HasException());
  }
}
