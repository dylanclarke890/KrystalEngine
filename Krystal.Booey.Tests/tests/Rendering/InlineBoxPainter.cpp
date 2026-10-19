#include "Krystal.Booey/Rendering/InlineBoxPainter.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Rendering/PaintTestUtils.hpp"

namespace krys::boo::tests
{

  TEST_CASE("InlineBoxPainter: paints an inline box's background and border around its text",
            "[Rendering][InlineBoxPainter]")
  {
    CHECK(Painted(u8"<style>body{margin:0;font:16px sans-serif}span{background:yellow;border:2px solid "
                  u8"red}</style>a <span>bc</span> d")
          == Canvas() + u8R"(
(draw-glyphs
  (local-anchor (0,14))
  (font-smoothing-mode auto)
  (length 2))
(fill-composited-rect
  (rect at (13,-2) size 21x21)
  (color #FFFF00)
  (composite-operation source-over)
  (blend-mode normal))
(set-state
  (change-flags [fill-brush, stroke-style, should-antialias])
  (fill-brush 
    (color #FF0000))
  (stroke-style no-stroke)
  (should-antialias 0))
(draw-rect
  (rect at (13,-2) size 21x2)
  (border-thickness 1.00))
(draw-rect
  (rect at (13,17) size 21x2)
  (border-thickness 1.00))
(draw-rect
  (rect at (13,-2) size 2x21)
  (border-thickness 1.00))
(draw-rect
  (rect at (32,-2) size 2x21)
  (border-thickness 1.00))
(set-state
  (change-flags [fill-brush, stroke-style, should-antialias])
  (fill-brush 
    (color #000000))
  (stroke-style solid-stroke)
  (should-antialias 1))
(draw-glyphs
  (local-anchor (15.34,14))
  (font-smoothing-mode auto)
  (length 2))
(draw-glyphs
  (local-anchor (34.24,14))
  (font-smoothing-mode auto)
  (length 2)))");
  }

  TEST_CASE("InlineBoxPainter: paints each line's slice, closed only at the box's ends",
            "[Rendering][InlineBoxPainter]")
  {
    CHECK(
      Painted(u8"<style>body{margin:0;font:16px sans-serif}div{width:60px}span{background:yellow;border:2px "
              u8"solid red;padding:0 3px}</style><div><span>aaa bbb ccc</span></div>")
      == Canvas() + u8R"(
(fill-composited-rect
  (rect at (0,-2) size 32x21)
  (color #FFFF00)
  (composite-operation source-over)
  (blend-mode normal))
(set-state
  (change-flags [fill-brush, stroke-style, should-antialias])
  (fill-brush 
    (color #FF0000))
  (stroke-style no-stroke)
  (should-antialias 0))
(draw-rect
  (rect at (0,-2) size 32x2)
  (border-thickness 1.00))
(draw-rect
  (rect at (0,17) size 32x2)
  (border-thickness 1.00))
(draw-rect
  (rect at (0,-2) size 2x21)
  (border-thickness 1.00))
(set-state
  (change-flags [fill-brush, stroke-style, should-antialias])
  (fill-brush 
    (color #000000))
  (stroke-style solid-stroke)
  (should-antialias 1))
(draw-glyphs
  (local-anchor (5,14))
  (font-smoothing-mode auto)
  (length 3))
(fill-composited-rect
  (rect at (0,16) size 27x21)
  (color #FFFF00)
  (composite-operation source-over)
  (blend-mode normal))
(set-state
  (change-flags [fill-brush, stroke-style, should-antialias])
  (fill-brush 
    (color #FF0000))
  (stroke-style no-stroke)
  (should-antialias 0))
(draw-rect
  (rect at (0,16) size 27x2)
  (border-thickness 1.00))
(draw-rect
  (rect at (0,35) size 27x2)
  (border-thickness 1.00))
(set-state
  (change-flags [fill-brush, stroke-style, should-antialias])
  (fill-brush 
    (color #000000))
  (stroke-style solid-stroke)
  (should-antialias 1))
(draw-glyphs
  (local-anchor (0,32))
  (font-smoothing-mode auto)
  (length 3))
(fill-composited-rect
  (rect at (0,34) size 29x21)
  (color #FFFF00)
  (composite-operation source-over)
  (blend-mode normal))
(set-state
  (change-flags [fill-brush, stroke-style, should-antialias])
  (fill-brush 
    (color #FF0000))
  (stroke-style no-stroke)
  (should-antialias 0))
(draw-rect
  (rect at (0,34) size 29x2)
  (border-thickness 1.00))
(draw-rect
  (rect at (0,53) size 29x2)
  (border-thickness 1.00))
(draw-rect
  (rect at (27,34) size 2x21)
  (border-thickness 1.00))
(set-state
  (change-flags [fill-brush, stroke-style, should-antialias])
  (fill-brush 
    (color #000000))
  (stroke-style solid-stroke)
  (should-antialias 1))
(draw-glyphs
  (local-anchor (0,50))
  (font-smoothing-mode auto)
  (length 3)))");
  }

  TEST_CASE("InlineBoxPainter: clones the decorations onto every slice", "[Rendering][InlineBoxPainter]")
  {
    CHECK(
      Painted(u8"<style>body{margin:0;font:16px sans-serif}div{width:60px}span{background:yellow;border:2px "
              u8"solid red;box-decoration-break:clone}</style><div><span>aaa bbb</span></div>")
      == Canvas() + u8R"(
(fill-composited-rect
  (rect at (0,-2) size 31x21)
  (color #FFFF00)
  (composite-operation source-over)
  (blend-mode normal))
(set-state
  (change-flags [fill-brush, stroke-style, should-antialias])
  (fill-brush 
    (color #FF0000))
  (stroke-style no-stroke)
  (should-antialias 0))
(draw-rect
  (rect at (0,-2) size 31x2)
  (border-thickness 1.00))
(draw-rect
  (rect at (0,17) size 31x2)
  (border-thickness 1.00))
(draw-rect
  (rect at (0,-2) size 2x21)
  (border-thickness 1.00))
(draw-rect
  (rect at (29,-2) size 2x21)
  (border-thickness 1.00))
(set-state
  (change-flags [fill-brush, stroke-style, should-antialias])
  (fill-brush 
    (color #000000))
  (stroke-style solid-stroke)
  (should-antialias 1))
(draw-glyphs
  (local-anchor (2,14))
  (font-smoothing-mode auto)
  (length 3))
(fill-composited-rect
  (rect at (0,16) size 31x21)
  (color #FFFF00)
  (composite-operation source-over)
  (blend-mode normal))
(set-state
  (change-flags [fill-brush, stroke-style, should-antialias])
  (fill-brush 
    (color #FF0000))
  (stroke-style no-stroke)
  (should-antialias 0))
(draw-rect
  (rect at (0,16) size 31x2)
  (border-thickness 1.00))
(draw-rect
  (rect at (0,35) size 31x2)
  (border-thickness 1.00))
(draw-rect
  (rect at (0,16) size 2x21)
  (border-thickness 1.00))
(draw-rect
  (rect at (29,16) size 2x21)
  (border-thickness 1.00))
(set-state
  (change-flags [fill-brush, stroke-style, should-antialias])
  (fill-brush 
    (color #000000))
  (stroke-style solid-stroke)
  (should-antialias 1))
(draw-glyphs
  (local-anchor (2,32))
  (font-smoothing-mode auto)
  (length 3)))");
  }

  TEST_CASE("InlineBoxPainter: paints atomic inlines in line order", "[Rendering][InlineBoxPainter]")
  {
    CHECK(Painted(u8"<style>body{margin:0;font:16px "
                  u8"sans-serif}span{display:inline-block;width:10px;height:10px;background:blue}</"
                  u8"style>a<span></span>b")
          == Canvas() + u8R"(
(draw-glyphs
  (local-anchor (0,14))
  (font-smoothing-mode auto)
  (length 1))
(fill-composited-rect
  (rect at (9,4) size 10x10)
  (color #0000FF)
  (composite-operation source-over)
  (blend-mode normal))
(draw-glyphs
  (local-anchor (18.90,14))
  (font-smoothing-mode auto)
  (length 1)))");
  }

  TEST_CASE("InlineBoxPainter: casts an inline box's shadow under its background",
            "[Rendering][InlineBoxPainter]")
  {
    CHECK(Painted(u8"<style>body{margin:0;font:16px sans-serif}span{box-shadow:2px 2px "
                  u8"red;background:white}</style><span>shadow</span>")
          == Canvas() + u8R"(
(save)
(set-state
  (change-flags [drop-shadow])
  (drop-shadow 
    (offset width=2 height=2)
    (radius 0.00)
    (color #FF0000)
    (shadows-use-legacy-radius 0)
    (opacity 1.00)))
(fill-composited-rect
  (rect at (0,0) size 55x17)
  (color #FFFFFF)
  (composite-operation source-over)
  (blend-mode normal))
(restore)
(draw-glyphs
  (local-anchor (0,14))
  (font-smoothing-mode auto)
  (length 6)))");
  }
}
