#include "Krystal.Booey/Rendering/RenderTreeAsText.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"

namespace krys::boo::tests
{
  using style::tests::ResolverFixture;

  namespace
  {
    // FIXME: Every box is 0x0 until block layout sizes them; the dumps' geometry changes then.
    constexpr const char8 *Markup = u8R"html(<!DOCTYPE html><style>
      #a { color: red; background-color: blue; border: 2px solid green; border-left: 3px dashed green }
      #rel { position: relative; z-index: 2 }
      #neg { position: absolute; z-index: -1 }
      #normal { overflow: hidden }
    </style><body><div id=a class="x y">hi</div><div id=rel></div><div id=neg></div><div id=normal></div>
    <ul><li>one</li></ul></body>)html";
  }

  TEST_CASE("RenderTreeAsText: QuoteAndEscapeNonPrintables", "[Rendering][RenderTreeAsText]")
  {
    CHECK(QuoteAndEscapeNonPrintables(u8"") == u8R"("")");
    CHECK(QuoteAndEscapeNonPrintables(u8"a\"b\\c") == u8R"("a\"b\\c")");
    // Newlines and no-break spaces print as spaces.
    CHECK(QuoteAndEscapeNonPrintables(u8"a\nb c") == u8R"("a b c")");
    // Everything else outside printable ASCII is escaped per UTF-16 code unit.
    CHECK(QuoteAndEscapeNonPrintables(u8"\té•\U0001F600") == u8R"("\x{9}\x{E9}\x{2022}\x{D83D}\x{DE00}")");
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "RenderTreeAsText: ExternalRepresentation",
                   "[Rendering][RenderTreeAsText]")
  {
    ResolverFixture fixture {Markup};

    SECTION("The layers that paint")
    {
      // Nothing intersects the empty view, so only the root layers dump.
      CHECK(ExternalRepresentation(fixture.Frame.get()) == u8R"(layer at (0,0) size 0x0
  RenderView at (0,0) size 0x0
layer at (0,0) size 0x0 layerType: background only
layer at (0,0) size 0x0 layerType: foreground only
  RenderBlock {HTML} at (0,0) size 0x0
    RenderBody {BODY} at (0,0) size 0x0
      RenderBlock {DIV} at (0,0) size 0x0 [color=#FF0000] [bgcolor=#0000FF] [border: (2px solid #008000) (3px dashed #008000)]
        RenderText {#text} at (0,0) size 0x0
      RenderBlock {UL} at (0,0) size 0x0
        RenderListItem {LI} at (0,0) size 0x0
          RenderListMarker at (0,0) size 0x0
          RenderText {#text} at (0,0) size 0x0
)");
    }

    SECTION("All layers, nested, with ids and classes")
    {
      CHECK(ExternalRepresentation(fixture.Frame.get(),
                                   {RenderAsTextFlag::ShowAllLayers, RenderAsTextFlag::ShowLayerNesting,
                                    RenderAsTextFlag::ShowIDAndClass, RenderAsTextFlag::ShowLayoutState,
                                    RenderAsTextFlag::ShowOverflow})
            == u8R"(layer at (0,0) size 0x0
  RenderView at (0,0) size 0x0
 positive z-order list (1)
  layer at (0,0) size 0x0 layerType: background only
   negative z-order list (1)
    layer at (0,0) size 0x0
      RenderBlock (positioned) zI: -1 {DIV} at (0,0) size 0x0 id="neg"
  layer at (0,0) size 0x0 layerType: foreground only
    RenderBlock {HTML} at (0,0) size 0x0
      RenderBody {BODY} at (0,0) size 0x0
        RenderBlock {DIV} at (0,0) size 0x0 [color=#FF0000] [bgcolor=#0000FF] [border: (2px solid #008000) (3px dashed #008000)] id="a" class="x y"
          RenderText {#text} at (0,0) size 0x0
        RenderBlock {UL} at (0,0) size 0x0
          RenderListItem {LI} at (0,0) size 0x0
            RenderListMarker at (0,0) size 0x0
            RenderText {#text} at (0,0) size 0x0
   normal flow list (1)
    layer at (0,0) size 0x0
      RenderBlock {DIV} at (0,0) size 0x0 id="normal"
   positive z-order list (1)
    layer at (0,0) size 0x0
      RenderBlock (relative positioned) zI: 2 {DIV} at (0,0) size 0x0 id="rel"
)");
    }

    SECTION("The layout state")
    {
      auto &renderer = *fixture.ById(u8"a").Renderer();
      (void)fixture.Document->UpdateLayout();
      renderer.SetNeedsLayout();

      TextStream ts;
      WriteDebugInfo(ts, renderer, {RenderAsTextFlag::ShowLayoutState, RenderAsTextFlag::ShowIDAndClass});
      CHECK(ts.Release() == u8R"( id="a" class="x y" (needs layout: self))");
      WriteDebugInfo(ts, *renderer.Parent(), {RenderAsTextFlag::ShowLayoutState});
      CHECK(ts.Release() == u8" (needs layout: child)");
      WriteDebugInfo(ts, renderer, {});
      CHECK(ts.Release() == u8"");
    }

    SECTION("An element's layers")
    {
      // Only boxes with a layer dump.
      CHECK(ExternalRepresentation(&fixture.ById(u8"a")) == u8"");
      CHECK(ExternalRepresentation(&fixture.ById(u8"rel")) == u8R"(layer at (0,0) size 0x0
  RenderBlock (relative positioned) zI: 2 {DIV} at (0,0) size 0x0
)");
    }
  }
}
