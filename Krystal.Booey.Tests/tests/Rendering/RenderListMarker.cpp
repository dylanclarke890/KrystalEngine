#include "Krystal.Booey/Rendering/RenderListMarker.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Rendering/PaintTestUtils.hpp"
#include "Krystal.Booey/Graphics/FontCascade.hpp"
#include "Krystal.Booey/Rendering/RenderListItem.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

namespace krys::boo::tests
{
  using style::tests::ResolverFixture;

  namespace
  {
    KRYS_NODISCARD RenderListMarker &MarkerOf(ResolverFixture &fixture, dom::DOMStringView id) noexcept
    {
      auto *marker = Downcast<RenderListItem>(*fixture.ById(id).Renderer()).MarkerRenderer();
      REQUIRE(marker);
      return *marker;
    }
  }

  // FIXME: The markers' positions, once there's inline layout.
  TEST_CASE_METHOD(css::tests::ValueFixture, "RenderListMarker: layout", "[Rendering][RenderListMarker]")
  {
    ResolverFixture fixture {u8R"html(<!DOCTYPE html><style>
      ul, ol { margin: 0 }
      #in { list-style-position: inside }
      #str { list-style-type: "-> " }
      #sq { list-style-type: square }
      #lh { line-height: 30px }
    </style><body><ul><li id=disc>a</li></ul><ol start=9><li id=nine>b</li><li id=ten>c</li></ol>
    <ul id=in><li id=inside>d</li></ul><ul id=str><li id=string>e</li></ul><ul id=sq><li id=square>f</li></ul>
    <ul id=lh><li id=tall>g</li></ul></body>)html"};
    (void)fixture.Document->UpdateLayout();

    // The values below are worked out from these metrics.
    const auto &metrics = MarkerOf(fixture, u8"disc").Style().MetricsOfPrimaryFont();
    REQUIRE(metrics.IntAscent() == 14);
    REQUIRE(metrics.IntDescent() == 3);
    REQUIRE(metrics.LineSpacing() == 18.f);
    REQUIRE(metrics.Ascent() > 14.f);
    REQUIRE(metrics.Ascent() < 14.5f);
    REQUIRE(metrics.Descent() > 3.f);
    REQUIRE(metrics.Descent() < 3.5f);

    SECTION("An outside bullet")
    {
      // Its width is (ascent * 2 / 3 + 1) / 2 + 2, and its margins put it (ascent * 2 / 3) + 7 before the
      // line.
      auto &marker = MarkerOf(fixture, u8"disc");
      CHECK(marker.TextWithSuffix() == u8"• ");
      CHECK(marker.LogicalWidth() == 7);
      CHECK(marker.LogicalHeight() == 17);
      CHECK(marker.MarginStart() == -17);
      CHECK(marker.MarginEnd() == 10);
      // The font's height rounds to 17 within the line spacing of 18, so the half leading is 0.5.
      CHECK(marker.LayoutBounds() == Pair<float, float> {14.f, 4.f});
      CHECK(marker.LineLogicalOffsetForListItem() == 0);

      CHECK(MarkerOf(fixture, u8"square").LogicalWidth() == 7);
      CHECK(MarkerOf(fixture, u8"square").MarginStart() == -17);
    }

    SECTION("An inside bullet")
    {
      auto &marker = MarkerOf(fixture, u8"inside");
      CHECK(marker.LogicalWidth() == 7);
      CHECK(marker.MarginStart() == -1);
      CHECK(marker.MarginEnd() == 8);
    }

    SECTION("Counters and strings are measured")
    {
      const auto &font = MarkerOf(fixture, u8"nine").Style().GetFontCascade();

      // A counter's outside margins add half the ascent's two thirds after it.
      auto &nine = MarkerOf(fixture, u8"nine");
      CHECK(nine.TextWithSuffix() == u8"9. ");
      CHECK(nine.LogicalWidth() == LayoutUnit(font.Width(u8"9. ")));
      CHECK(nine.MarginStart() == -nine.LogicalWidth() - 4);
      CHECK(nine.MarginEnd() == 4);

      auto &ten = MarkerOf(fixture, u8"ten");
      CHECK(ten.TextWithSuffix() == u8"10. ");
      CHECK(ten.LogicalWidth() == LayoutUnit(font.Width(u8"10. ")));
      CHECK(ten.LogicalWidth() > nine.LogicalWidth());

      // A string's outside margins only move it before the line.
      auto &string = MarkerOf(fixture, u8"string");
      CHECK(string.LogicalWidth() == LayoutUnit(font.Width(u8"-> ")));
      CHECK(string.MarginStart() == -string.LogicalWidth());
      CHECK(string.MarginEnd() == 0);
    }

    SECTION("A fixed line height spreads its leading around the text")
    {
      // (30 - 17) / 2 = 6.5, floored into the ascent and ceiled into the descent.
      CHECK(MarkerOf(fixture, u8"tall").LayoutBounds() == Pair<float, float> {20.f, 10.f});
      CHECK(MarkerOf(fixture, u8"tall").LogicalHeight() == 17);
    }
  }

  TEST_CASE("RenderListMarker: paints bullets as shapes and counters as text",
            "[Rendering][RenderListMarker]")
  {
    CHECK(
      Painted(u8"<style>body{margin:0;font:16px "
              u8"sans-serif}ul{margin:0;padding-left:30px}</style><ul><li>disc</li><li "
              u8"style=list-style-type:circle>circle</li><li style=list-style-type:square>square</li></ul>")
      == Canvas() + u8R"(
(set-inline-stroke
  (thickness 1.00))
(fill-ellipse
  (rect at (14,7) size 5.17x5.17))
(draw-glyphs
  (local-anchor (30,14))
  (font-smoothing-mode auto)
  (length 4))
(stroke-ellipse
  (rect at (14,25) size 5.17x5.17))
(draw-glyphs
  (local-anchor (30,32))
  (font-smoothing-mode auto)
  (length 6))
(fill-rect
  (rect at (14,43) size 5.17x5.17)
  (requiresClipToRect 1))
(draw-glyphs
  (local-anchor (30,50))
  (font-smoothing-mode auto)
  (length 6)))");

    CHECK(Painted(u8"<style>body{margin:0;font:16px "
                  u8"sans-serif}ol{margin:0;padding-left:30px}</style><ol><li>one</li><li>two</li></ol>")
          == Canvas() + u8R"(
(set-inline-stroke
  (thickness 1.00))
(draw-glyphs
  (local-anchor (8,14))
  (font-smoothing-mode auto)
  (length 3))
(draw-glyphs
  (local-anchor (30,14))
  (font-smoothing-mode auto)
  (length 3))
(draw-glyphs
  (local-anchor (8,32))
  (font-smoothing-mode auto)
  (length 3))
(draw-glyphs
  (local-anchor (30,32))
  (font-smoothing-mode auto)
  (length 3)))");
  }
}
