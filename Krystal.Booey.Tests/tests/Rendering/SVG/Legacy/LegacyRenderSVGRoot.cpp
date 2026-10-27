#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGRoot.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/Rendering/RenderTreeAsText.hpp"
#include "Krystal.Booey/SVG/SVGSvgElement.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo::tests
{
  using style::tests::ResolverFixture;

  namespace
  {
    KRYS_NODISCARD svg::SVGSvgElement &SvgById(dom::Document &document, dom::DOMStringView id) noexcept
    {
      auto element = document.GetElementById(id);
      krys_assert(element);
      return Downcast<svg::SVGSvgElement>(Downcast<svg::SVGElement>(*element));
    }

    KRYS_NODISCARD LegacyRenderSVGRoot &RootById(dom::Document &document, dom::DOMStringView id) noexcept
    {
      auto *renderer = SvgById(document, id).Renderer();
      krys_assert(renderer);
      return Downcast<LegacyRenderSVGRoot>(*renderer);
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "LegacyRenderSVGRoot: the render tree",
                   "[Rendering][SVG][LegacyRenderSVGRoot]")
  {
    ResolverFixture fixture {
      u8R"html(<!DOCTYPE html><body style="margin: 0"><svg width=200 height=100 viewBox="0 0 100 50">)html"
      u8R"html(<g transform="translate(10 20)"><g/></g><defs><g/></defs><symbol><g/></symbol>)html"
      u8R"html(<switch><g systemLanguage=xx-YY /><g id=first /><g/></switch><g style="display: none"><g/></g>)html"
      u8R"html(<foreignObject><div>html</div></foreignObject><x><g/></x>)html"
      u8R"html(<svg x=5 y=6 width=50 height=40 viewBox="0 0 10 10"><g/></svg></svg></body>)html"};

    // The outermost <svg> is a replaced box (dumped as `RenderSVGRoot`), and its
    // descendants are SVG containers: hidden ones for <defs>, <symbol> and <g> with `display: none`, which
    // still get renderers, and viewport containers for inner <svg>s. A <switch> renders only its first child
    // whose conditional processing attributes pass, a <foreignObject> holds HTML blocks, and elements SVG
    // doesn't know get no renderers. Without shapes, nothing has a repaint rect.
    CHECK(ExternalRepresentation(fixture.Frame.get()) == u8R"(layer at (0,0) size 800x600
  RenderView at (0,0) size 800x600
layer at (0,0) size 800x104
  RenderBlock {HTML} at (0,0) size 800x104
    RenderBody {BODY} at (0,0) size 800x104
      RenderSVGRoot {svg} at (0,0) size 0x0
        RenderSVGContainer {g} at (0,0) size 0x0 [transform={m=((1.00,0.00)(0.00,1.00)) t=(10.00,20.00)}]
          RenderSVGContainer {g} at (0,0) size 0x0
        RenderSVGHiddenContainer {defs} at (0,0) size 0x0
          RenderSVGContainer {g} at (0,0) size 0x0
        RenderSVGHiddenContainer {symbol} at (0,0) size 0x0
          RenderSVGContainer {g} at (0,0) size 0x0
        RenderSVGContainer {switch} at (0,0) size 0x0
          RenderSVGContainer {g} at (0,0) size 0x0
        RenderSVGHiddenContainer {g} at (0,0) size 0x0
          RenderSVGContainer {g} at (0,0) size 0x0
        RenderSVGForeignObject {foreignObject} at (0,0) size 0x0
          RenderBlock {DIV} at (0,0) size 0x18
            RenderText {#text} at (0,0) size 31x17
              text run at (0,0) width 31: "html"
        RenderSVGViewportContainer {svg} at (0,0) size 0x0
          RenderSVGContainer {g} at (0,0) size 0x0
)");
    auto first = fixture.Document->GetElementById(u8"first");
    REQUIRE(first);
    CHECK(first->Renderer() != nullptr);
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "LegacyRenderSVGRoot: size and view box",
                   "[Rendering][SVG][LegacyRenderSVGRoot]")
  {
    ResolverFixture fixture {
      u8R"html(<!DOCTYPE html><body style="margin: 0">)html"
      u8R"html(<svg id=a width=200 height=100 viewBox="0 0 100 50"></svg>)html"
      u8R"html(<svg id=b></svg>)html"
      u8R"html(<svg id=c width=200 height=100 viewBox="0 0 100 100" style="border: 2px solid; padding: 3px"></svg>)html"
      u8R"html(<svg id=d viewBox="0 0 40 20" style="width: 100px"></svg>)html"
      u8R"html(</body>)html"};
    (void)fixture.Document->UpdateLayout();

    // The width and height attributes size the box; the view box maps into its content box.
    auto &a = RootById(*fixture.Document, u8"a");
    CHECK(a.Size() == LayoutSize(200, 100));
    CHECK(a.LocalToBorderBoxTransform() == AffineTransform(2, 0, 0, 2, 0, 0));

    // Without them, the box takes the default size of replaced elements.
    auto &b = RootById(*fixture.Document, u8"b");
    CHECK(b.Size() == LayoutSize(300, 150));
    CHECK(b.LocalToBorderBoxTransform().IsIdentity());

    // The view box is centred (xMidYMid meet) inside the border and padding.
    auto &c = RootById(*fixture.Document, u8"c");
    CHECK(c.Size() == LayoutSize(210, 110));
    CHECK(c.LocalToBorderBoxTransform() == AffineTransform(1, 0, 0, 1, 55, 5));

    // A view box gives an aspect ratio: with only a width, the height follows it.
    auto &d = RootById(*fixture.Document, u8"d");
    CHECK(d.Size() == LayoutSize(100, 50));
    CHECK(d.LocalToBorderBoxTransform() == AffineTransform(2.5, 0, 0, 2.5, 0, 0));

    // Changing the view box lays the root out again.
    REQUIRE_FALSE(SvgById(*fixture.Document, u8"a").SetAttribute(u8"viewBox", u8"0 0 50 25").HasException());
    (void)fixture.Document->UpdateLayout();
    CHECK(a.LocalToBorderBoxTransform() == AffineTransform(4, 0, 0, 4, 0, 0));

    // So does panning.
    SvgById(*fixture.Document, u8"b").SetCurrentTranslate(FloatPoint(7, 8));
    (void)fixture.Document->UpdateLayout();
    CHECK(b.LocalToBorderBoxTransform() == AffineTransform(1, 0, 0, 1, 7, 8));
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "LegacyRenderSVGRoot: hit testing",
                   "[Rendering][SVG][LegacyRenderSVGRoot]")
  {
    ResolverFixture fixture {
      u8R"html(<!DOCTYPE html><body style="margin: 0"><svg id=s width=200 height=100><g/></svg>)html"
      u8R"html(<svg id=n width=200 height=100 style="pointer-events: none"></svg></body>)html"};
    (void)fixture.Document->UpdateLayout();

    // Unlike SVG 1.1, SVG 2 lets the <svg> itself be hit, when no graphics are under the point.
    auto &s = SvgById(*fixture.Document, u8"s");
    CHECK(fixture.Document->ElementFromPoint(50, 50).get() == &s);
    CHECK(fixture.Document->ElementFromPoint(250, 50).get() != &s);
    // Not with `pointer-events: none`.
    CHECK(fixture.Document->ElementFromPoint(250, 50).get() != &SvgById(*fixture.Document, u8"n"));
  }
}
