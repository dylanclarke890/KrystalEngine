#include "Krystal.Booey/Rendering/SVG/RenderSVGText.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"
#include "Krystal.Booey/Rendering/RenderTreeAsText.hpp"
#include "Krystal.Booey/Rendering/SVG/RenderSVGInlineText.hpp"
#include "Krystal.Booey/Rendering/SVG/RenderSVGTextPath.hpp"
#include "Krystal.Booey/Rendering/SVG/RenderSVGTSpan.hpp"
#include "Krystal.Booey/SVG/SVGElement.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo::tests
{
  using style::tests::ResolverFixture;

  namespace
  {
    // The render tree below the body, which every dump here shares.
    KRYS_NODISCARD utf8_string SVGDump(ResolverFixture &fixture) noexcept
    {
      auto dump = ExternalRepresentation(fixture.Frame.get());
      auto start = dump.find(u8"      RenderSVGRoot");
      krys_assert(start != utf8_string::npos);
      return dump.substr(start);
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "RenderSVGText: the render tree",
                   "[Rendering][SVG][RenderSVGText]")
  {
    ResolverFixture fixture {u8R"html(<!DOCTYPE html><body style="margin: 0"><svg width=200 height=100>)html"
                             u8R"html(<text x=10 y=20>Hello <tspan fill=red>world</tspan></text>)html"
                             u8R"html(<text x="10 20 30" y=50 dx=1 text-anchor=middle>abc</text>)html"
                             u8R"html(</svg></body>)html"};

    // <text> is a block, <tspan> an inline and the text nodes inline text. Each run is a fragment of a text
    // box, at its position in user space. Each absolutely positioned character starts a text chunk of its
    // own, so the middle anchor centres each character on its x (dx shifting the first).
    CHECK(SVGDump(fixture) == u8R"(      RenderSVGRoot {svg} at (6,5) size 84x49
        RenderSVGText {text} at (10,5) size 80x19 contains 1 chunk(s)
          RenderSVGInlineText {#text} at (0,0) size 41x18
            chunk 1 text run 1 at (10.00,20.00) startOffset 0 endOffset 6 width 40.91: "Hello "
          RenderSVGTSpan {tspan} at (40,0) size 40x18
            RenderSVGInlineText {#text} at (40,0) size 39x18
              chunk 1 text run 1 at (50.91,20.00) startOffset 0 endOffset 5 width 38.23: "world"
        RenderSVGText {text} at (6,35) size 28x19 contains 1 chunk(s)
          RenderSVGInlineText {#text} at (0,0) size 28x18
            chunk 1 (middle anchor) text run 1 at (6.55,50.00) startOffset 0 endOffset 1 width 8.90: "a"
            chunk 1 (middle anchor) text run 1 at (15.55,50.00) startOffset 0 endOffset 1 width 8.90: "b"
            chunk 1 (middle anchor) text run 1 at (26.00,50.00) startOffset 0 endOffset 1 width 8.00: "c"
)");

    auto *text = fixture.Document->QuerySelector(u8"text").ReleaseValue()->Renderer();
    REQUIRE(Is<RenderSVGText>(text));
    CHECK(Is<RenderSVGInlineText>(text->FirstChildSlow()));
    CHECK(Is<RenderSVGTSpan>(text->LastChildSlow()));
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "RenderSVGText: positioning", "[Rendering][SVG][RenderSVGText]")
  {
    ResolverFixture fixture {
      u8R"html(<!DOCTYPE html><body style="margin: 0"><svg width=200 height=100>)html"
      u8R"html(<path id=p d="M 10 80 L 190 80" />)html"
      u8R"html(<text><textPath href="#p" startOffset=20>abc</textPath></text>)html"
      u8R"html(<text x=10 y=20 xml:space=preserve>a  b</text>)html"
      u8R"html(<text x=10 y=40 textLength=100 lengthAdjust=spacing>abc</text>)html"
      u8R"html(<text x=10 y=60><tspan dy=5 dx=3>a</tspan><tspan rotate=90>b</tspan></text>)html"
      u8R"html(</svg></body>)html"};

    // Text on a path starts startOffset along it, each glyph placed at its midpoint. xml:space=preserve keeps
    // both spaces. textLength with lengthAdjust=spacing spreads the glyphs so the text is 100 long. dx and dy
    // shift a tspan's first character and the ones after it; rotate turns its glyph.
    CHECK(SVGDump(fixture) == u8R"(      RenderSVGRoot {svg} at (10,5) size 100x79
        RenderSVGPath {path} at (0,0) size 0x0 [fill={[type=SOLID] [color=#000000]}] [data="M 10 80 L 190 80"]
        RenderSVGText {text} at (30,65) size 26x19 contains 1 chunk(s)
          RenderSVGTextPath {textPath} at (0,0) size 26x18
            RenderSVGInlineText {#text} at (0,0) size 26x18
              chunk 1 text run 1 at (34.45,80.00) startOffset 0 endOffset 1 width 8.90: "a"
              chunk 1 text run 2 at (43.35,80.00) startOffset 1 endOffset 2 width 8.90: "b"
              chunk 1 text run 3 at (51.80,80.00) startOffset 2 endOffset 3 width 8.00: "c"
        RenderSVGText {text} at (10,5) size 27x19 contains 1 chunk(s)
          RenderSVGInlineText {#text} at (0,0) size 27x18
            chunk 1 text run 1 at (10.00,20.00) startOffset 0 endOffset 4 width 26.69: "a  b"
        RenderSVGText {text} at (10,25) size 100x19 contains 1 chunk(s)
          RenderSVGInlineText {#text} at (0,0) size 100x18
            chunk 1 text run 1 at (10.00,40.00) startOffset 0 endOffset 1 width 8.90: "a"
            chunk 1 text run 2 at (56.00,40.00) startOffset 1 endOffset 2 width 8.90: "b"
            chunk 1 text run 3 at (102.00,40.00) startOffset 2 endOffset 3 width 8.00: "c"
        RenderSVGText {text} at (13,50) size 24x24 contains 1 chunk(s)
          RenderSVGTSpan {tspan} at (0,0) size 9x18
            RenderSVGInlineText {#text} at (0,0) size 9x18
              chunk 1 text run 1 at (13.00,65.00) startOffset 0 endOffset 1 width 8.90: "a"
          RenderSVGTSpan {tspan} at (5,14) size 19x10
            RenderSVGInlineText {#text} at (5,14) size 18x9
              chunk 1 text run 1 at (21.90,65.00) startOffset 0 endOffset 1 width 8.90: "b"
)");
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "RenderSVGText: changes", "[Rendering][SVG][RenderSVGText]")
  {
    ResolverFixture fixture {
      u8R"html(<!DOCTYPE html><body style="margin: 0"><svg width=200 height=100>)html"
      u8R"html(<text><textPath id=tp href="#later">abc</textPath></text></svg></body>)html"};

    // A text path whose path doesn't exist yet lays out nothing.
    CHECK(SVGDump(fixture) == u8R"(      RenderSVGRoot {svg} at (0,0) size 0x0
        RenderSVGText {text} at (0,0) size 0x0 contains 1 chunk(s)
          RenderSVGTextPath {textPath} at (0,0) size 0x0
            RenderSVGInlineText {#text} at (0,0) size 0x0
)");

    // Once an element takes the id, the text path finds it.
    auto *svg = fixture.Document->QuerySelector(u8"svg").ReleaseValue().get();
    auto path = fixture.Document
                  ->CreateElementNS(dom::DOMStringAtom(infra::Namespaces::SVG), dom::DOMStringAtom(u8"path"))
                  .ReleaseValue();
    REQUIRE(!path->SetAttribute(dom::DOMStringAtom(u8"id"), u8"later").HasException());
    REQUIRE(!path->SetAttribute(dom::DOMStringAtom(u8"d"), u8"M 0 50 L 200 50").HasException());
    REQUIRE(!svg->AppendChild(*path).HasException());
    CHECK(SVGDump(fixture) == u8R"(      RenderSVGRoot {svg} at (0,35) size 26x19
        RenderSVGText {text} at (0,35) size 26x19 contains 1 chunk(s)
          RenderSVGTextPath {textPath} at (0,0) size 26x18
            RenderSVGInlineText {#text} at (0,0) size 26x18
              chunk 1 text run 1 at (4.45,50.00) startOffset 0 endOffset 1 width 8.90: "a"
              chunk 1 text run 2 at (13.35,50.00) startOffset 1 endOffset 2 width 8.90: "b"
              chunk 1 text run 3 at (21.80,50.00) startOffset 2 endOffset 3 width 8.00: "c"
        RenderSVGPath {path} at (0,0) size 0x0 [fill={[type=SOLID] [color=#000000]}] [data="M 0 50 L 200 50"]
)");

    // Changing the text lays it out again.
    auto &textNode = Downcast<dom::Text>(*fixture.Document->GetElementById(u8"tp")->FirstChild());
    REQUIRE(!textNode.AppendData(u8"de").HasException());
    CHECK(SVGDump(fixture) == u8R"(      RenderSVGRoot {svg} at (0,35) size 44x19
        RenderSVGText {text} at (0,35) size 44x19 contains 1 chunk(s)
          RenderSVGTextPath {textPath} at (0,0) size 44x18
            RenderSVGInlineText {#text} at (0,0) size 44x18
              chunk 1 text run 1 at (4.45,50.00) startOffset 0 endOffset 1 width 8.90: "a"
              chunk 1 text run 2 at (13.35,50.00) startOffset 1 endOffset 2 width 8.90: "b"
              chunk 1 text run 3 at (21.80,50.00) startOffset 2 endOffset 3 width 8.00: "c"
              chunk 1 text run 4 at (30.25,50.00) startOffset 3 endOffset 4 width 8.90: "d"
              chunk 1 text run 5 at (39.14,50.00) startOffset 4 endOffset 5 width 8.90: "e"
        RenderSVGPath {path} at (0,0) size 0x0 [fill={[type=SOLID] [color=#000000]}] [data="M 0 50 L 200 50"]
)");

    // Removing the path leaves the text path without one again.
    REQUIRE(!path->Remove().HasException());
    CHECK(SVGDump(fixture) == u8R"(      RenderSVGRoot {svg} at (0,0) size 0x0
        RenderSVGText {text} at (0,0) size 0x0 contains 1 chunk(s)
          RenderSVGTextPath {textPath} at (0,0) size 0x0
            RenderSVGInlineText {#text} at (0,0) size 0x0
)");
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "RenderSVGText: hit testing", "[Rendering][SVG][RenderSVGText]")
  {
    ResolverFixture fixture {u8R"html(<!DOCTYPE html><body style="margin: 0"><svg width=200 height=100>)html"
                             u8R"html(<text id=t x=10 y=20>ab<tspan id=s>cd</tspan></text>)html"
                             u8R"html(<text x=10 y=60 pointer-events=none>ab</text></svg></body>)html"};

    // A point in a glyph's cell hits the element whose text it is.
    auto *text = fixture.Document->GetElementById(u8"t").get();
    auto *tspan = fixture.Document->GetElementById(u8"s").get();
    CHECK(fixture.Document->ElementFromPoint(12, 15).get() == text);
    CHECK(fixture.Document->ElementFromPoint(40, 15).get() == tspan);

    // Points beside the text, and text that doesn't take pointer events, hit the <svg>.
    auto *svg = fixture.Document->QuerySelector(u8"svg").ReleaseValue().get();
    CHECK(fixture.Document->ElementFromPoint(150, 15).get() == svg);
    CHECK(fixture.Document->ElementFromPoint(12, 55).get() == svg);
  }
}
