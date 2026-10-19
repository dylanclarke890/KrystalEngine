#include "Krystal.Booey/Rendering/BackgroundPainter.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Rendering/PaintTestUtils.hpp"

namespace krys::boo::tests
{
  TEST_CASE("BackgroundPainter: the canvas takes the root's background, or the body's",
            "[Rendering][BackgroundPainter]")
  {
    CHECK(Painted(u8"<style>body{margin:0;background:red}</style>") == u8R"(
(set-state
  (change-flags [composite-mode])
  (composite-mode 
    (composite-operation copy)
    (blend-mode normal)))
(fill-rect-with-color
  (rect at (0,0) size 200x100)
  (color #FFFFFF))
(set-state
  (change-flags [composite-mode])
  (composite-mode 
    (composite-operation source-over)
    (blend-mode normal)))
(fill-composited-rect
  (rect at (0,0) size 200x100)
  (color #FF0000)
  (composite-operation copy)
  (blend-mode normal)))");

    CHECK(Painted(u8"<style>html{background:#eee}body{margin:10px;height:20px;background:red}</style>")
          == u8R"(
(set-state
  (change-flags [composite-mode])
  (composite-mode 
    (composite-operation copy)
    (blend-mode normal)))
(fill-rect-with-color
  (rect at (0,0) size 200x100)
  (color #FFFFFF))
(set-state
  (change-flags [composite-mode])
  (composite-mode 
    (composite-operation source-over)
    (blend-mode normal)))
(fill-composited-rect
  (rect at (0,0) size 200x100)
  (color #EEEEEE)
  (composite-operation copy)
  (blend-mode normal))
(fill-composited-rect
  (rect at (10,10) size 180x20)
  (color #FF0000)
  (composite-operation source-over)
  (blend-mode normal)))");
  }

  TEST_CASE("BackgroundPainter: clips to the content box", "[Rendering][BackgroundPainter]")
  {
    CHECK(Painted(u8"<style>body{margin:0}div{width:40px;height:20px;padding:5px;border:2px solid "
                  u8"transparent;background:green content-box}</style><div></div>")
          == u8R"(
(set-state
  (change-flags [composite-mode])
  (composite-mode 
    (composite-operation copy)
    (blend-mode normal)))
(fill-rect-with-color
  (rect at (0,0) size 200x100)
  (color #FFFFFF))
(set-state
  (change-flags [composite-mode])
  (composite-mode 
    (composite-operation source-over)
    (blend-mode normal)))
(fill-composited-rect
  (rect at (0,0) size 200x100)
  (color #FFFFFF)
  (composite-operation copy)
  (blend-mode normal))
(save)
(clip
  (rect at (7,7) size 40x20))
(fill-composited-rect
  (rect at (0,0) size 54x34)
  (color #008000)
  (composite-operation source-over)
  (blend-mode normal))
(restore))");
  }

  TEST_CASE("BackgroundPainter: tiles and positions gradient images over the colour",
            "[Rendering][BackgroundPainter]")
  {
    CHECK(Painted(u8"<style>body{margin:0}div{width:40px;height:20px;background:linear-gradient(red,blue) 0 "
                  u8"0/20px 10px}</style><div></div>")
          == u8R"(
(set-state
  (change-flags [composite-mode])
  (composite-mode 
    (composite-operation copy)
    (blend-mode normal)))
(fill-rect-with-color
  (rect at (0,0) size 200x100)
  (color #FFFFFF))
(set-state
  (change-flags [composite-mode])
  (composite-mode 
    (composite-operation source-over)
    (blend-mode normal)))
(fill-composited-rect
  (rect at (0,0) size 200x100)
  (color #FFFFFF)
  (composite-operation copy)
  (blend-mode normal))
(save)
(clip
  (rect at (0,0) size 40x20))
(save)
(clip
  (rect at (0,0) size 20x10))
(fill-rect-with-gradient
  (rect at (0,0) size 20x10)
  (gradient 
    (p0 (0,0))
    (p1 (0,10))
    (color-interpolation-method sRGB premultiplied)
    (spread-method pad)
    (stops [#FF0000 at 0.00, #0000FF at 1.00])))
(restore)
(save)
(clip
  (rect at (20,0) size 20x10))
(translate
  (x 20.00)
  (y 0.00))
(fill-rect-with-gradient
  (rect at (0,0) size 20x10)
  (gradient 
    (p0 (0,0))
    (p1 (0,10))
    (color-interpolation-method sRGB premultiplied)
    (spread-method pad)
    (stops [#FF0000 at 0.00, #0000FF at 1.00])))
(restore)
(save)
(clip
  (rect at (0,10) size 20x10))
(translate
  (x 0.00)
  (y 10.00))
(fill-rect-with-gradient
  (rect at (0,0) size 20x10)
  (gradient 
    (p0 (0,0))
    (p1 (0,10))
    (color-interpolation-method sRGB premultiplied)
    (spread-method pad)
    (stops [#FF0000 at 0.00, #0000FF at 1.00])))
(restore)
(save)
(clip
  (rect at (20,10) size 20x10))
(translate
  (x 20.00)
  (y 10.00))
(fill-rect-with-gradient
  (rect at (0,0) size 20x10)
  (gradient 
    (p0 (0,0))
    (p1 (0,10))
    (color-interpolation-method sRGB premultiplied)
    (spread-method pad)
    (stops [#FF0000 at 0.00, #0000FF at 1.00])))
(restore)
(restore))");

    CHECK(Painted(u8"<style>body{margin:0}div{width:40px;height:20px;background:linear-gradient(red,blue) "
                  u8"5px 5px/10px 10px no-repeat,yellow}</style><div></div>")
          == u8R"(
(set-state
  (change-flags [composite-mode])
  (composite-mode 
    (composite-operation copy)
    (blend-mode normal)))
(fill-rect-with-color
  (rect at (0,0) size 200x100)
  (color #FFFFFF))
(set-state
  (change-flags [composite-mode])
  (composite-mode 
    (composite-operation source-over)
    (blend-mode normal)))
(fill-composited-rect
  (rect at (0,0) size 200x100)
  (color #FFFFFF)
  (composite-operation copy)
  (blend-mode normal))
(fill-composited-rect
  (rect at (0,0) size 40x20)
  (color #FFFF00)
  (composite-operation source-over)
  (blend-mode normal))
(save)
(clip
  (rect at (5,5) size 10x10))
(translate
  (x 5.00)
  (y 5.00))
(fill-rect-with-gradient
  (rect at (0,0) size 10x10)
  (gradient 
    (p0 (0,0))
    (p1 (0,10))
    (color-interpolation-method sRGB premultiplied)
    (spread-method pad)
    (stops [#FF0000 at 0.00, #0000FF at 1.00])))
(restore))");
  }

  TEST_CASE("BackgroundPainter: avoids bleeding under rounded borders", "[Rendering][BackgroundPainter]")
  {
    CHECK(Painted(u8"<style>body{margin:0}div{width:40px;height:20px;border:4px solid "
                  u8"black;border-radius:8px;background:yellow}</style><div></div>")
          == Canvas() + u8R"(
(fill-rounded-rect
  (rect at (1,1) size 46x26
    (top-left width=8 height=8)
    (top-right width=8 height=8)
    (bottom-left width=8 height=8)
    (bottom-right width=8 height=8))
  (color #FFFF00)
  (blend-mode normal))
(set-state
  (change-flags [fill-rule])
  (fill-rule EVEN-ODD))
(fill-path
  (path add rounded rect at (0,0) size 48x28
    (top-left width=8 height=8)
    (top-right width=8 height=8)
    (bottom-left width=8 height=8)
    (bottom-right width=8 height=8), add rounded rect at (4,4) size 40x20
    (top-left width=4 height=4)
    (top-right width=4 height=4)
    (bottom-left width=4 height=4)
    (bottom-right width=4 height=4))))");

    CHECK(Painted(u8"<style>body{margin:0}div{width:40px;height:20px;border:1px solid "
                  u8"black;border-radius:8px;background:rgba(255,255,0,.5)}</style><div></div>")
          == Canvas() + u8R"(
(save)
(clip-rounded-rect
  (rect at (0,0) size 42x22
    (top-left width=8 height=8)
    (top-right width=8 height=8)
    (bottom-left width=8 height=8)
    (bottom-right width=8 height=8)))
(begin-transparency-layer
  (opacity 1.00))
(fill-composited-rect
  (rect at (0,0) size 42x22)
  (color #FFFF0080)
  (composite-operation source-over)
  (blend-mode normal))
(set-state
  (change-flags [fill-rule])
  (fill-rule EVEN-ODD))
(fill-path
  (path add rounded rect at (0,0) size 42x22
    (top-left width=8 height=8)
    (top-right width=8 height=8)
    (bottom-left width=8 height=8)
    (bottom-right width=8 height=8), add rounded rect at (1,1) size 40x20
    (top-left width=7 height=7)
    (top-right width=7 height=7)
    (bottom-left width=7 height=7)
    (bottom-right width=7 height=7)))
(end-transparency-layer)
(restore))");
  }

  TEST_CASE("BackgroundPainter: casts outer box shadows from outside the clip, spread shadows rounded",
            "[Rendering][BackgroundPainter]")
  {
    CHECK(Painted(u8"<style>body{margin:0}div{margin:10px;width:20px;height:10px;box-shadow:2px 3px 4px "
                  u8"rgba(0,0,0,.5)}</style><div></div>")
          == Canvas() + u8R"(
(save)
(clip
  (rect at (6,7) size 32x22))
(set-state
  (change-flags [drop-shadow])
  (drop-shadow 
    (offset width=-27 height=3)
    (radius 4.00)
    (color #00000080)
    (shadows-use-legacy-radius 0)
    (opacity 1.00)))
(clip-out
  (rect at (10,10) size 20x10))
(fill-rect-with-color
  (rect at (39,10) size 20x10)
  (color #000000))
(restore))");

    CHECK(Painted(u8"<style>body{margin:0}div{margin:10px;width:20px;height:10px;background:white;border-"
                  u8"radius:4px;box-shadow:0 0 0 3px blue}</style><div></div>")
          == Canvas() + u8R"(
(save)
(clip
  (rect at (7,7) size 26x16))
(set-state
  (change-flags [drop-shadow])
  (drop-shadow 
    (offset width=-27 height=0)
    (radius 0.00)
    (color #0000FF)
    (shadows-use-legacy-radius 0)
    (opacity 1.00)))
(clip-out-rounded-rect
  (rect at (10,10) size 20x10
    (top-left width=4 height=4)
    (top-right width=4 height=4)
    (bottom-left width=4 height=4)
    (bottom-right width=4 height=4)))
(fill-rounded-rect
  (rect at (34,7) size 26x16
    (top-left width=7 height=7)
    (top-right width=7 height=7)
    (bottom-left width=7 height=7)
    (bottom-right width=7 height=7))
  (color #000000)
  (blend-mode normal))
(restore)
(fill-rounded-rect
  (rect at (10,10) size 20x10
    (top-left width=4 height=4)
    (top-right width=4 height=4)
    (bottom-left width=4 height=4)
    (bottom-right width=4 height=4))
  (color #FFFFFF)
  (blend-mode normal)))");
  }

  TEST_CASE("BackgroundPainter: casts inset box shadows from a rect with the box as its hole",
            "[Rendering][BackgroundPainter]")
  {
    CHECK(Painted(u8"<style>body{margin:0}div{margin:10px;width:20px;height:10px;box-shadow:inset 2px 2px "
                  u8"4px black}</style><div></div>")
          == Canvas() + u8R"(
(save)
(clip
  (rect at (10,10) size 20x10))
(translate
  (x 49.00)
  (y 0.00))
(set-state
  (change-flags [drop-shadow])
  (drop-shadow 
    (offset width=-47 height=2)
    (radius 4.00)
    (color #000000)
    (shadows-use-legacy-radius 0)
    (opacity 1.00)))
(fill-rect-with-rounded-hole
  (rect at (2,2) size 34x24)
  (rounded-hole-rect at (10,10) size 20x10
    (top-left width=0 height=0)
    (top-right width=0 height=0)
    (bottom-left width=0 height=0)
    (bottom-right width=0 height=0))
  (color #000000))
(restore))");
  }
}
