#include "Krystal.Booey/Rendering/Painting/OutlinePainter.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Rendering/PaintTestUtils.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLSelectElement.hpp"
#include "Krystal.Booey/Page/EventHandler.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Platform/Events/PlatformKeyboardEvent.hpp"
#include "Krystal.Booey/Platform/Events/WindowsKeyboardCodes.hpp"

namespace krys::boo::tests
{
  TEST_CASE("OutlinePainter: paints outlines outside the border box", "[Rendering][OutlinePainter]")
  {
    CHECK(Painted(u8"<style>body{margin:0}div{margin:10px;width:20px;height:10px;outline:2px solid "
                  u8"red;outline-offset:2px}</style><div></div>")
          == Canvas() + u8R"(
(set-state
  (change-flags [fill-brush, stroke-style, should-antialias])
  (fill-brush 
    (color #FF0000))
  (stroke-style no-stroke)
  (should-antialias 0))
(draw-rect
  (rect at (6,6) size 28x2)
  (border-thickness 1.00))
(draw-rect
  (rect at (6,22) size 28x2)
  (border-thickness 1.00))
(draw-rect
  (rect at (6,6) size 2x18)
  (border-thickness 1.00))
(draw-rect
  (rect at (32,6) size 2x18)
  (border-thickness 1.00))
(set-state
  (change-flags [stroke-style, should-antialias])
  (stroke-style solid-stroke)
  (should-antialias 1)))");
  }

  TEST_CASE("OutlinePainter: paints auto outlines as focus rings", "[Rendering][OutlinePainter]")
  {
    CHECK(Painted(u8"<style>body{margin:0}div{margin:10px;width:20px;height:10px;outline:3px auto "
                  u8"blue}</style><div></div>")
          == Canvas() + u8R"(
(draw-focus-ring-rects
  (rects [at (10,10) size 20x10])
  (outline-offset 0.00)
  (outline-width 3.00)
  (color #0000FF)))");
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "OutlinePainter: paints focus rings around SVG graphics",
                   "[Rendering][OutlinePainter]")
  {
    // A shape's ring is its bounding box, in the SVG's coordinates; a container's unites what its children
    // paint.
    auto painted = Painted(u8"<style>body{margin:0}</style><svg width=100 height=50>"
                           u8"<rect x=10 y=5 width=20 height=10 style='fill:none;outline:3px auto blue'/>"
                           u8"<g style='outline:3px auto red'><rect x=40 y=5 width=10 height=10 />"
                           u8"<rect x=60 y=20 width=10 height=10 /></g></svg>");
    CHECK(painted.contains(u8R"(
(draw-focus-ring-rects
  (rects [at (10,5) size 20x10])
  (outline-offset 0.00)
  (outline-width 3.00)
  (color #0000FF)))"));
    CHECK(painted.contains(u8R"(
(draw-focus-ring-rects
  (rects [at (40,5) size 30x25])
  (outline-offset 0.00)
  (outline-width 3.00)
  (color #FF0000)))"));
  }

  TEST_CASE("OutlinePainter: rings a list box's active item while the control key moves it",
            "[Rendering][OutlinePainter]")
  {
    PaintFixture fixture(
      u8"<!DOCTYPE html><style>body{margin:0}select{margin:0;outline:3px auto blue}</style>"
      u8"<select id=s multiple size=3><option>a</option><option>b</option><option>c</option>"
      u8"</select>");
    auto &select = Downcast<html::HTMLSelectElement>(fixture.ById(u8"s"));
    select.Focus();
    // Otherwise the ring is around the whole list box.
    CHECK(fixture.Painted().contains(u8"(rects [at (2,2) size 32x51])"));

    // Moving with the control key held moves the active item without selecting it, and rings the item.
    (void)fixture.Frame->GetEventHandler().KeyEvent(PlatformKeyboardEvent(
      PlatformEvent::Type::RawKeyDown, u8"", u8"", u8"ArrowDown", u8"ArrowDown", u8"Down", vk::Down, false,
      false, false, {PlatformEvent::Modifier::ControlKey}, {}));
    CHECK(select.ActiveSelectionEndListIndex() == 0);
    CHECK(select.SelectedIndex() == -1);
    CHECK(fixture.Painted().contains(u8"(rects [at (3,3) size 9x14])"));
  }

  TEST_CASE("OutlinePainter: outlines inline boxes around their line boxes", "[Rendering][OutlinePainter]")
  {
    CHECK(
      Painted(
        u8"<style>body{margin:0;font:16px sans-serif}span{outline:2px solid red}</style>a <span>bc</span> d")
      == Canvas() + u8R"(
(set-inline-fill-color
  (color #000000))
(draw-glyphs
  (local-anchor (0,14))
  (font-smoothing-mode auto)
  (length 2))
(draw-glyphs
  (local-anchor (13.34,14))
  (font-smoothing-mode auto)
  (length 2))
(draw-glyphs
  (local-anchor (30.24,14))
  (font-smoothing-mode auto)
  (length 2))
(set-state
  (change-flags [fill-brush, stroke-style, should-antialias])
  (fill-brush 
    (color #FF0000))
  (stroke-style no-stroke)
  (should-antialias 0))
(draw-rect
  (rect at (11,-2) size 21x2)
  (border-thickness 1.00))
(draw-rect
  (rect at (11,17) size 21x2)
  (border-thickness 1.00))
(draw-rect
  (rect at (11,-2) size 2x21)
  (border-thickness 1.00))
(draw-rect
  (rect at (30,-2) size 2x21)
  (border-thickness 1.00))
(set-state
  (change-flags [stroke-style, should-antialias])
  (stroke-style solid-stroke)
  (should-antialias 1)))");

    CHECK(Painted(u8"<style>body{margin:0;font:16px sans-serif}div{width:60px}span{outline:2px solid "
                  u8"red}</style><div>aa <span>bbb ccc</span></div>")
          == Canvas() + u8R"(
(set-inline-fill-color
  (color #000000))
(draw-glyphs
  (local-anchor (0,14))
  (font-smoothing-mode auto)
  (length 3))
(draw-glyphs
  (local-anchor (22.24,14))
  (font-smoothing-mode auto)
  (length 3))
(draw-glyphs
  (local-anchor (0,32))
  (font-smoothing-mode auto)
  (length 3))
(set-inline-stroke
  (color #FF0000)
  (thickness 2.00))
(stroke-path
  (path move to (21,-1), add curve to (21,-1) (21,-1) (21,-1), add line to (50,-1), add line to (50,18), add line to (25,18), add line to (25,36), add curve to (25,36) (25,36) (25,36), add line to (-1,36), add line to (-1,17), add line to (21,17), close subpath)))");

    CHECK(
      Painted(
        u8"<style>body{margin:0;font:16px sans-serif}span{outline:2px auto blue}</style><span>focus</span>")
      == Canvas() + u8R"(
(set-inline-fill-color
  (color #000000))
(draw-glyphs
  (local-anchor (0,14))
  (font-smoothing-mode auto)
  (length 5))
(draw-focus-ring-rects
  (rects [at (0,0) size 38x17])
  (outline-offset 0.00)
  (outline-width 3.00)
  (color #0000FF)))");
  }
}
