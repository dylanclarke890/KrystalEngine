#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGResourcePattern.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Rendering/PaintTestUtils.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/Rendering/RenderTreeAsText.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/SVGResources.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/SVGResourcesCache.hpp"
#include "Krystal.Booey/SVG/PatternAttributes.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo::tests
{
  using style::tests::ResolverFixture;

  namespace
  {
    KRYS_NODISCARD dom::Element &ElementById(dom::Document &document, dom::DOMStringView id) noexcept
    {
      auto element = document.GetElementById(id);
      krys_assert(element);
      return *element;
    }

    KRYS_NODISCARD bool Contains(utf8_stringview text, utf8_stringview part) noexcept
    {
      return text.find(part) != utf8_stringview::npos;
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "LegacyRenderSVGResourcePattern: the render tree",
                   "[Rendering][SVG][LegacyRenderSVGResourcePattern]")
  {
    ResolverFixture fixture {
      u8R"html(<!DOCTYPE html><body style="margin: 0"><svg width=200 height=100>)html"
      u8R"html(<pattern id=p width=10 height=10 patternUnits=userSpaceOnUse><rect width=5 height=5 fill=red /></pattern>)html"
      u8R"html(<pattern id=q href="#p" patternTransform="scale(2)" patternContentUnits=objectBoundingBox />)html"
      u8R"html(<rect width=40 height=40 fill="url(#q)" /></svg></body>)html"};

    // A pattern dumps its id and the units and transform it paints with, collected from the patterns it
    // references; its content dumps as a hidden container's would.
    auto text = ExternalRepresentation(fixture.Frame.get());
    CAPTURE(css::tests::ToNarrow(text));
    CHECK(Contains(
      text,
      u8R"(        RenderSVGResourcePattern {pattern} [id="p"] [patternUnits=userSpaceOnUse] [patternContentUnits=userSpaceOnUse]
          RenderSVGRect {rect} at (0,0) size 5x5 [fill={[type=SOLID] [color=#FF0000]}] [x=0.00] [y=0.00] [width=5.00] [height=5.00]
        RenderSVGResourcePattern {pattern} [id="q"] [patternUnits=userSpaceOnUse] [patternContentUnits=objectBoundingBox] [patternTransform={m=((2.00,0.00)(0.00,2.00)) t=(0.00,0.00)}]
)"));
    CHECK(Contains(text, u8R"([fill={[type=PATTERN] [id="q"]}])"));

    // The attributes, with the content, come from the referenced pattern where the pattern doesn't have them.
    svg::PatternAttributes attributes;
    Downcast<LegacyRenderSVGResourcePattern>(*ElementById(*fixture.Document, u8"q").Renderer())
      .CollectPatternAttributes(attributes);
    CHECK(attributes.PatternContentElement() == &ElementById(*fixture.Document, u8"p"));
    CHECK(attributes.Width().ValueAsString() == u8"10");
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "LegacyRenderSVGResourcePattern: painting",
                   "[Rendering][SVG][LegacyRenderSVGResourcePattern]")
  {
    // The tile is drawn into an image buffer at the scale it's painted at, and fills the shape as a pattern
    // brush: the tile placed at the pattern's origin, scaled from the buffer back to the tile's size.
    auto painted = Painted(
      u8R"html(<body style="margin: 0"><svg width=100 height=50>)html"
      u8R"html(<pattern id=p x=5 width=10 height=20 patternUnits=userSpaceOnUse><rect width=5 height=5 fill=red /></pattern>)html"
      u8R"html(<rect width=40 height=40 fill="url(#p)" /></svg></body>)html");
    CAPTURE(css::tests::ToNarrow(painted));
    CHECK(Contains(painted, u8"(tile-size width=10 height=20)"));
    CHECK(Contains(painted, u8"(pattern-space-transform {m=((1.00,0.00)(0.00,1.00)) t=(5.00,0.00)})"));
    CHECK(Contains(painted, u8"(fill-rect\n  (rect at (0,0) size 40x40)"));

    // A pattern without content, or with an empty view box, paints nothing.
    auto empty = Painted(
      u8R"html(<body style="margin: 0"><svg width=100 height=50>)html"
      u8R"html(<pattern id=p width=10 height=10 patternUnits=userSpaceOnUse />)html"
      u8R"html(<pattern id=v width=10 height=10 patternUnits=userSpaceOnUse viewBox="0 0 0 10"><rect width=5 height=5 /></pattern>)html"
      u8R"html(<rect width=40 height=40 fill="url(#p)" /><rect width=40 height=40 fill="url(#v)" /></svg></body>)html");
    CAPTURE(css::tests::ToNarrow(empty));
    CHECK_FALSE(Contains(empty, u8"pattern"));
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "LegacyRenderSVGResourcePattern: cycles",
                   "[Rendering][SVG][LegacyRenderSVGResourcePattern]")
  {
    // A pattern whose content fills with the pattern itself is a cycle: that fill is dropped.
    ResolverFixture fixture {
      u8R"html(<!DOCTYPE html><body style="margin: 0"><svg width=200 height=100>)html"
      u8R"html(<pattern id=p width=10 height=10 patternUnits=userSpaceOnUse><rect id=inner width=5 height=5 fill="url(#p)" /></pattern>)html"
      u8R"html(<rect id=outer width=40 height=40 fill="url(#p)" /></svg></body>)html"};
    (void)fixture.Document->UpdateLayout();

    auto *inner =
      SVGResourcesCache::CachedResourcesForRenderer(*ElementById(*fixture.Document, u8"inner").Renderer());
    CHECK((!inner || !inner->Fill()));
    auto *outer =
      SVGResourcesCache::CachedResourcesForRenderer(*ElementById(*fixture.Document, u8"outer").Renderer());
    REQUIRE(outer);
    CHECK(outer->Fill() == ElementById(*fixture.Document, u8"p").Renderer());

    // Painting doesn't recurse.
    PaintFixture paint(
      u8R"html(<!DOCTYPE html><body style="margin: 0"><svg width=200 height=100>)html"
      u8R"html(<pattern id=p width=10 height=10 patternUnits=userSpaceOnUse><rect width=5 height=5 fill="url(#p)" /></pattern>)html"
      u8R"html(<rect width=40 height=40 fill="url(#p)" /></svg></body>)html");
    CHECK(Contains(paint.Painted(), u8"(pattern"));
  }
}
