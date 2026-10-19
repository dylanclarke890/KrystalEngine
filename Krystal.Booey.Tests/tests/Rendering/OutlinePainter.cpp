#include "Krystal.Booey/Rendering/OutlinePainter.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Rendering/PaintTestUtils.hpp"

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
}
