#include "Krystal.Booey/Rendering/SVG/SVGRenderingContext.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Rendering/PaintTestUtils.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGElement.hpp"

namespace krys::boo::tests
{
  namespace
  {
    KRYS_NODISCARD bool Contains(utf8_stringview text, utf8_stringview part) noexcept
    {
      return text.find(part) != utf8_stringview::npos;
    }

    KRYS_NODISCARD utf8_string FilteredRect(utf8_stringview filter) noexcept
    {
      return u8R"html(<!DOCTYPE html><body style="margin: 0"><svg width=200 height=100>)html"
             u8R"html(<rect id=r x=10 y=10 width=30 height=20 fill=blue style="filter: )html"
             + utf8_string(filter) + u8R"html(" /></svg></body>)html";
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "SVGRenderingContext: filter functions on SVG shapes",
                   "[Rendering][SVG][SVGRenderingContext]")
  {
    // Filter functions draw the shape into their source image, whose region is the shape's stroke box
    // grown by the functions' outsets.
    SECTION("blur()")
    {
      PaintFixture fixture(FilteredRect(u8"blur(2px)"), {200, 100}, true);
      auto painted = fixture.Painted();
      CAPTURE(css::tests::ToNarrow(painted));
      CHECK(Contains(painted, u8"(rect at (4,4) size 42x32)"));
      CHECK(Contains(painted, u8"(radius width=4 height=4)"));
      CHECK(Contains(painted, u8"(rect at (10,10) size 30x20)"));

      auto outsets = SVGRenderingContext::CSSFilterOutsets(*fixture.ById(u8"r").Renderer());
      CHECK(outsets == IntOutsets(6));
    }

    SECTION("drop-shadow()")
    {
      PaintFixture fixture(FilteredRect(u8"drop-shadow(2px 2px 0 red)"), {200, 100}, true);
      auto painted = fixture.Painted();
      CAPTURE(css::tests::ToNarrow(painted));
      CHECK(Contains(painted, u8"(rect at (10,10) size 32x22)"));
      CHECK(Contains(painted, u8"(color #FF0000)"));
    }

    SECTION("an identity is drawn unfiltered")
    {
      PaintFixture fixture(FilteredRect(u8"blur(0)"), {200, 100}, true);
      auto painted = fixture.Painted();
      CAPTURE(css::tests::ToNarrow(painted));
      CHECK_FALSE(Contains(painted, u8"begin-transparency-layer"));
      CHECK(Contains(painted, u8"(fill-rect"));
    }

    SECTION("a url() is the filter resource's")
    {
      PaintFixture fixture(u8R"html(<!DOCTYPE html><body style="margin: 0"><svg width=200 height=100>)html"
                           u8R"html(<filter id=f><feGaussianBlur stdDeviation=2 /></filter>)html"
                           u8R"html(<rect id=r x=10 y=10 width=30 height=20 style="filter: url(#f)" />)html"
                           u8R"html(</svg></body>)html",
                           {200, 100}, true);
      auto painted = fixture.Painted();
      CAPTURE(css::tests::ToNarrow(painted));
      CHECK(Contains(painted, u8"(draw-filtered-image-buffer"));
      CHECK(SVGRenderingContext::CSSFilterOutsets(*fixture.ById(u8"r").Renderer()).IsZero());
    }
  }
}
