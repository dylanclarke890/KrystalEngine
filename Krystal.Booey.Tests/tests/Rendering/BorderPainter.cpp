#include "Krystal.Booey/Rendering/BorderPainter.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Rendering/PaintTestUtils.hpp"

namespace krys::boo::tests
{
  TEST_CASE("BorderPainter: paints solid sides as rects, mitred where colours meet",
            "[Rendering][BorderPainter]")
  {
    CHECK(
      Painted(u8"<style>body{margin:0}div{width:20px;height:10px;border:2px solid black}</style><div></div>")
      == Canvas() + u8R"(
(set-state
  (change-flags [stroke-style, should-antialias])
  (stroke-style no-stroke)
  (should-antialias 0))
(draw-rect
  (rect at (0,0) size 24x2)
  (border-thickness 1.00))
(draw-rect
  (rect at (0,12) size 24x2)
  (border-thickness 1.00))
(draw-rect
  (rect at (0,0) size 2x14)
  (border-thickness 1.00))
(draw-rect
  (rect at (22,0) size 2x14)
  (border-thickness 1.00))
(set-state
  (change-flags [stroke-style, should-antialias])
  (stroke-style solid-stroke)
  (should-antialias 1)))");

    CHECK(Painted(u8"<style>body{margin:0}div{width:20px;height:10px;border:4px solid;border-color:red green "
                  u8"blue yellow}</style><div></div>")
          == Canvas() + u8R"(
(set-state
  (change-flags [fill-brush, stroke-style, should-antialias])
  (fill-brush 
    (color #FF0000))
  (stroke-style no-stroke)
  (should-antialias 0))
(draw-rect
  (rect at (0,0) size 28x4)
  (border-thickness 1.00))
(set-inline-fill-color
  (color #0000FF))
(draw-rect
  (rect at (0,14) size 28x4)
  (border-thickness 1.00))
(set-inline-fill-color
  (color #FFFF00))
(fill-path
  (path move to (0,0), add line to (0,18), add line to (4,14), add line to (4,4), close subpath))
(set-inline-fill-color
  (color #008000))
(fill-path
  (path move to (24,4), add line to (24,14), add line to (28,18), add line to (28,0), close subpath))
(set-state
  (change-flags [stroke-style, should-antialias])
  (stroke-style solid-stroke)
  (should-antialias 1)))");
  }

  TEST_CASE("BorderPainter: paints uniform rounded and translucent borders as one even-odd path",
            "[Rendering][BorderPainter]")
  {
    CHECK(Painted(u8"<style>body{margin:0}div{width:20px;height:10px;border:3px solid "
                  u8"black;border-radius:6px}</style><div></div>")
          == Canvas() + u8R"(
(set-state
  (change-flags [fill-rule])
  (fill-rule EVEN-ODD))
(fill-path
  (path add rounded rect at (0,0) size 26x16
    (top-left width=6 height=6)
    (top-right width=6 height=6)
    (bottom-left width=6 height=6)
    (bottom-right width=6 height=6), add rounded rect at (3,3) size 20x10
    (top-left width=3 height=3)
    (top-right width=3 height=3)
    (bottom-left width=3 height=3)
    (bottom-right width=3 height=3))))");

    CHECK(Painted(u8"<style>body{margin:0}div{width:20px;height:10px;border:2px solid "
                  u8"rgba(0,0,0,.5)}</style><div></div>")
          == Canvas() + u8R"(
(set-state
  (change-flags [fill-brush, fill-rule])
  (fill-brush 
    (color #00000080))
  (fill-rule EVEN-ODD))
(fill-path
  (path add rect at (0,0) size 24x14, add rect at (2,2) size 20x10)))");
  }

  TEST_CASE("BorderPainter: paints dashed sides as lines and rounded dotted sides as clipped strokes",
            "[Rendering][BorderPainter]")
  {
    CHECK(
      Painted(u8"<style>body{margin:0}div{width:30px;height:10px;border:2px dashed black}</style><div></div>")
      == Canvas() + u8R"(
(set-state
  (change-flags [stroke-thickness, stroke-style, should-antialias])
  (stroke-thickness 2.00)
  (stroke-style dashed-stroke)
  (should-antialias 0))
(draw-line
  (point-1 (0,0))
  (point-2 (34,2)))
(draw-line
  (point-1 (0,12))
  (point-2 (34,14)))
(draw-line
  (point-1 (0,0))
  (point-2 (2,14)))
(draw-line
  (point-1 (32,0))
  (point-2 (34,14)))
(set-state
  (change-flags [stroke-style, should-antialias])
  (stroke-style solid-stroke)
  (should-antialias 1)))");

    CHECK(Painted(u8"<style>body{margin:0}div{width:30px;height:20px;border:3px dotted "
                  u8"black;border-radius:8px}</style><div></div>")
          == Canvas() + u8R"(
(save)
(clip-rounded-rect
  (rect at (0,0) size 36x26
    (top-left width=8 height=8)
    (top-right width=8 height=8)
    (bottom-left width=8 height=8)
    (bottom-right width=8 height=8)))
(clip-out-rounded-rect
  (rect at (3,3) size 30x20
    (top-left width=5 height=5)
    (top-right width=5 height=5)
    (bottom-left width=5 height=5)
    (bottom-right width=5 height=5)))
(save)
(set-state
  (change-flags [should-antialias])
  (should-antialias 0))
(clip-path
  (path move to (0,0), add line to (15,15), add line to (21,15), add line to (36,0), close subpath)
  (wind-rule NON-ZERO))
(set-line-dash
  (dash-array [3.00, 3.00])
  (dash-offset 3.00))
(set-state
  (change-flags [stroke-thickness, stroke-style, should-antialias])
  (stroke-thickness 6.60)
  (stroke-style dotted-stroke)
  (should-antialias 1))
(stroke-path
  (path add rounded rect at (0,0) size 36x26
    (top-left width=8 height=8)
    (top-right width=8 height=8)
    (bottom-left width=8 height=8)
    (bottom-right width=8 height=8)))
(restore)
(save)
(set-state
  (change-flags [should-antialias])
  (should-antialias 0))
(clip-path
  (path move to (0,26), add line to (15,11), add line to (21,11), add line to (36,26), close subpath)
  (wind-rule NON-ZERO))
(set-line-dash
  (dash-array [3.00, 3.00])
  (dash-offset 3.00))
(set-state
  (change-flags [stroke-thickness, stroke-style, should-antialias])
  (stroke-thickness 6.60)
  (stroke-style dotted-stroke)
  (should-antialias 1))
(stroke-path
  (path add rounded rect at (0,0) size 36x26
    (top-left width=8 height=8)
    (top-right width=8 height=8)
    (bottom-left width=8 height=8)
    (bottom-right width=8 height=8)))
(restore)
(save)
(set-state
  (change-flags [should-antialias])
  (should-antialias 0))
(clip-path
  (path move to (0,0), add line to (15,15), add line to (15,11), add line to (0,26), close subpath)
  (wind-rule NON-ZERO))
(set-line-dash
  (dash-array [3.00, 3.00])
  (dash-offset 3.00))
(set-state
  (change-flags [stroke-thickness, stroke-style, should-antialias])
  (stroke-thickness 6.60)
  (stroke-style dotted-stroke)
  (should-antialias 1))
(stroke-path
  (path add rounded rect at (0,0) size 36x26
    (top-left width=8 height=8)
    (top-right width=8 height=8)
    (bottom-left width=8 height=8)
    (bottom-right width=8 height=8)))
(restore)
(save)
(set-state
  (change-flags [should-antialias])
  (should-antialias 0))
(clip-path
  (path move to (36,0), add line to (21,15), add line to (21,11), add line to (36,26), close subpath)
  (wind-rule NON-ZERO))
(set-line-dash
  (dash-array [3.00, 3.00])
  (dash-offset 3.00))
(set-state
  (change-flags [stroke-thickness, stroke-style, should-antialias])
  (stroke-thickness 6.60)
  (stroke-style dotted-stroke)
  (should-antialias 1))
(stroke-path
  (path add rounded rect at (0,0) size 36x26
    (top-left width=8 height=8)
    (top-right width=8 height=8)
    (bottom-left width=8 height=8)
    (bottom-right width=8 height=8)))
(restore)
(restore))");
  }

  TEST_CASE("BorderPainter: paints double, groove and inset sides in stripes and shades",
            "[Rendering][BorderPainter]")
  {
    CHECK(
      Painted(u8"<style>body{margin:0}div{width:20px;height:10px;border:6px double black}</style><div></div>")
      == Canvas() + u8R"(
(set-state
  (change-flags [stroke-style, should-antialias])
  (stroke-style no-stroke)
  (should-antialias 0))
(fill-path
  (path move to (0,0), add line to (2,2), add line to (30,2), add line to (32,0), close subpath))
(fill-path
  (path move to (4,4), add line to (6,6), add line to (26,6), add line to (28,4), close subpath))
(fill-path
  (path move to (6,16), add line to (4,18), add line to (28,18), add line to (26,16), close subpath))
(fill-path
  (path move to (2,20), add line to (0,22), add line to (32,22), add line to (30,20), close subpath))
(fill-path
  (path move to (0,0), add line to (0,22), add line to (2,20), add line to (2,2), close subpath))
(fill-path
  (path move to (4,4), add line to (4,18), add line to (6,16), add line to (6,6), close subpath))
(fill-path
  (path move to (26,6), add line to (26,16), add line to (28,18), add line to (28,4), close subpath))
(fill-path
  (path move to (30,2), add line to (30,20), add line to (32,22), add line to (32,0), close subpath))
(set-state
  (change-flags [stroke-style, should-antialias])
  (stroke-style solid-stroke)
  (should-antialias 1)))");

    CHECK(
      Painted(u8"<style>body{margin:0}div{width:20px;height:10px;border:4px groove #888}</style><div></div>")
      == Canvas() + u8R"(
(set-state
  (change-flags [fill-brush, stroke-style, should-antialias])
  (fill-brush 
    (color #343434))
  (stroke-style no-stroke)
  (should-antialias 0))
(draw-rect
  (rect at (0,0) size 28x2)
  (border-thickness 1.00))
(set-inline-fill-color
  (color #DCDCDC))
(draw-rect
  (rect at (0,2) size 28x2)
  (border-thickness 1.00))
(set-inline-fill-color
  (color #343434))
(draw-rect
  (rect at (0,14) size 28x2)
  (border-thickness 1.00))
(set-inline-fill-color
  (color #DCDCDC))
(draw-rect
  (rect at (0,16) size 28x2)
  (border-thickness 1.00))
(set-inline-fill-color
  (color #343434))
(fill-path
  (path move to (0,0), add line to (0,18), add line to (2,16), add line to (2,2), close subpath))
(set-inline-fill-color
  (color #DCDCDC))
(fill-path
  (path move to (2,2), add line to (2,16), add line to (4,14), add line to (4,4), close subpath))
(set-inline-fill-color
  (color #343434))
(fill-path
  (path move to (24,4), add line to (24,14), add line to (26,16), add line to (26,2), close subpath))
(set-inline-fill-color
  (color #DCDCDC))
(fill-path
  (path move to (26,2), add line to (26,16), add line to (28,18), add line to (28,0), close subpath))
(set-state
  (change-flags [stroke-style, should-antialias])
  (stroke-style solid-stroke)
  (should-antialias 1)))");

    CHECK(
      Painted(u8"<style>body{margin:0}div{width:20px;height:10px;border:4px inset #888}</style><div></div>")
      == Canvas() + u8R"(
(set-state
  (change-flags [fill-brush, stroke-style, should-antialias])
  (fill-brush 
    (color #343434))
  (stroke-style no-stroke)
  (should-antialias 0))
(draw-rect
  (rect at (0,0) size 28x4)
  (border-thickness 1.00))
(set-inline-fill-color
  (color #DCDCDC))
(draw-rect
  (rect at (0,14) size 28x4)
  (border-thickness 1.00))
(set-inline-fill-color
  (color #343434))
(fill-path
  (path move to (0,0), add line to (0,18), add line to (4,14), add line to (4,0), close subpath))
(set-inline-fill-color
  (color #DCDCDC))
(fill-path
  (path move to (24,4), add line to (24,18), add line to (28,18), add line to (28,0), close subpath))
(set-state
  (change-flags [stroke-style, should-antialias])
  (stroke-style solid-stroke)
  (should-antialias 1)))");
  }

  TEST_CASE("BorderPainter: paints border images piece by piece", "[Rendering][BorderPainter]")
  {
    CHECK(Painted(u8"<style>body{margin:0}div{width:20px;height:10px;border:5px "
                  u8"solid;border-image:linear-gradient(red,blue) 5 / 5px stretch}</style><div></div>")
          == Canvas() + u8R"(
(save)
(clip
  (rect at (0,0) size 5x5))
(fill-rect-with-gradient
  (rect at (0,0) size 30x20)
  (gradient 
    (p0 (0,0))
    (p1 (0,20))
    (color-interpolation-method sRGB premultiplied)
    (spread-method pad)
    (stops [#FF0000 at 0.00, #0000FF at 1.00])))
(restore)
(save)
(clip
  (rect at (0,5) size 5x10))
(translate
  (x 0.00)
  (y 5.00))
(translate
  (x 0.00)
  (y -5.00))
(fill-rect-with-gradient
  (rect at (0,0) size 30x20)
  (gradient 
    (p0 (0,0))
    (p1 (0,20))
    (color-interpolation-method sRGB premultiplied)
    (spread-method pad)
    (stops [#FF0000 at 0.00, #0000FF at 1.00])))
(restore)
(save)
(clip
  (rect at (0,15) size 5x5))
(translate
  (x 0.00)
  (y 15.00))
(translate
  (x 0.00)
  (y -15.00))
(fill-rect-with-gradient
  (rect at (0,0) size 30x20)
  (gradient 
    (p0 (0,0))
    (p1 (0,20))
    (color-interpolation-method sRGB premultiplied)
    (spread-method pad)
    (stops [#FF0000 at 0.00, #0000FF at 1.00])))
(restore)
(save)
(clip
  (rect at (25,0) size 5x5))
(translate
  (x 25.00)
  (y 0.00))
(translate
  (x -25.00)
  (y 0.00))
(fill-rect-with-gradient
  (rect at (0,0) size 30x20)
  (gradient 
    (p0 (0,0))
    (p1 (0,20))
    (color-interpolation-method sRGB premultiplied)
    (spread-method pad)
    (stops [#FF0000 at 0.00, #0000FF at 1.00])))
(restore)
(save)
(clip
  (rect at (25,5) size 5x10))
(translate
  (x 25.00)
  (y 5.00))
(translate
  (x -25.00)
  (y -5.00))
(fill-rect-with-gradient
  (rect at (0,0) size 30x20)
  (gradient 
    (p0 (0,0))
    (p1 (0,20))
    (color-interpolation-method sRGB premultiplied)
    (spread-method pad)
    (stops [#FF0000 at 0.00, #0000FF at 1.00])))
(restore)
(save)
(clip
  (rect at (25,15) size 5x5))
(translate
  (x 25.00)
  (y 15.00))
(translate
  (x -25.00)
  (y -15.00))
(fill-rect-with-gradient
  (rect at (0,0) size 30x20)
  (gradient 
    (p0 (0,0))
    (p1 (0,20))
    (color-interpolation-method sRGB premultiplied)
    (spread-method pad)
    (stops [#FF0000 at 0.00, #0000FF at 1.00])))
(restore)
(save)
(clip
  (rect at (5,0) size 20x5))
(translate
  (x 5.00)
  (y 0.00))
(translate
  (x -5.00)
  (y 0.00))
(fill-rect-with-gradient
  (rect at (0,0) size 30x20)
  (gradient 
    (p0 (0,0))
    (p1 (0,20))
    (color-interpolation-method sRGB premultiplied)
    (spread-method pad)
    (stops [#FF0000 at 0.00, #0000FF at 1.00])))
(restore)
(save)
(clip
  (rect at (5,15) size 20x5))
(translate
  (x 5.00)
  (y 15.00))
(translate
  (x -5.00)
  (y -15.00))
(fill-rect-with-gradient
  (rect at (0,0) size 30x20)
  (gradient 
    (p0 (0,0))
    (p1 (0,20))
    (color-interpolation-method sRGB premultiplied)
    (spread-method pad)
    (stops [#FF0000 at 0.00, #0000FF at 1.00])))
(restore))");

    CHECK(Painted(u8"<style>body{margin:0}div{width:20px;height:10px;border:5px "
                  u8"solid;border-image:linear-gradient(red,blue) 30% / 5px round}</style><div></div>")
          == Canvas() + u8R"(
(save)
(clip
  (rect at (0,0) size 5x5))
(scale
  (size width=0.56 height=0.83))
(fill-rect-with-gradient
  (rect at (0,0) size 30x20)
  (gradient 
    (p0 (0,0))
    (p1 (0,20))
    (color-interpolation-method sRGB premultiplied)
    (spread-method pad)
    (stops [#FF0000 at 0.00, #0000FF at 1.00])))
(restore)
(save)
(clip
  (rect at (0,5) size 5x10))
(save)
(clip
  (rect at (0,5) size 5x5))
(translate
  (x 0.00)
  (y 5.00))
(scale
  (size width=0.56 height=0.63))
(translate
  (x 0.00)
  (y -6.00))
(fill-rect-with-gradient
  (rect at (0,0) size 30x20)
  (gradient 
    (p0 (0,0))
    (p1 (0,20))
    (color-interpolation-method sRGB premultiplied)
    (spread-method pad)
    (stops [#FF0000 at 0.00, #0000FF at 1.00])))
(restore)
(save)
(clip
  (rect at (0,10) size 5x5))
(translate
  (x 0.00)
  (y 10.00))
(scale
  (size width=0.56 height=0.63))
(translate
  (x 0.00)
  (y -6.00))
(fill-rect-with-gradient
  (rect at (0,0) size 30x20)
  (gradient 
    (p0 (0,0))
    (p1 (0,20))
    (color-interpolation-method sRGB premultiplied)
    (spread-method pad)
    (stops [#FF0000 at 0.00, #0000FF at 1.00])))
(restore)
(restore)
(save)
(clip
  (rect at (0,15) size 5x5))
(translate
  (x 0.00)
  (y 15.00))
(scale
  (size width=0.56 height=0.83))
(translate
  (x 0.00)
  (y -14.00))
(fill-rect-with-gradient
  (rect at (0,0) size 30x20)
  (gradient 
    (p0 (0,0))
    (p1 (0,20))
    (color-interpolation-method sRGB premultiplied)
    (spread-method pad)
    (stops [#FF0000 at 0.00, #0000FF at 1.00])))
(restore)
(save)
(clip
  (rect at (25,0) size 5x5))
(translate
  (x 25.00)
  (y 0.00))
(scale
  (size width=0.56 height=0.83))
(translate
  (x -21.00)
  (y 0.00))
(fill-rect-with-gradient
  (rect at (0,0) size 30x20)
  (gradient 
    (p0 (0,0))
    (p1 (0,20))
    (color-interpolation-method sRGB premultiplied)
    (spread-method pad)
    (stops [#FF0000 at 0.00, #0000FF at 1.00])))
(restore)
(save)
(clip
  (rect at (25,5) size 5x10))
(save)
(clip
  (rect at (25,5) size 5x5))
(translate
  (x 25.00)
  (y 5.00))
(scale
  (size width=0.56 height=0.63))
(translate
  (x -21.00)
  (y -6.00))
(fill-rect-with-gradient
  (rect at (0,0) size 30x20)
  (gradient 
    (p0 (0,0))
    (p1 (0,20))
    (color-interpolation-method sRGB premultiplied)
    (spread-method pad)
    (stops [#FF0000 at 0.00, #0000FF at 1.00])))
(restore)
(save)
(clip
  (rect at (25,10) size 5x5))
(translate
  (x 25.00)
  (y 10.00))
(scale
  (size width=0.56 height=0.63))
(translate
  (x -21.00)
  (y -6.00))
(fill-rect-with-gradient
  (rect at (0,0) size 30x20)
  (gradient 
    (p0 (0,0))
    (p1 (0,20))
    (color-interpolation-method sRGB premultiplied)
    (spread-method pad)
    (stops [#FF0000 at 0.00, #0000FF at 1.00])))
(restore)
(restore)
(save)
(clip
  (rect at (25,15) size 5x5))
(translate
  (x 25.00)
  (y 15.00))
(scale
  (size width=0.56 height=0.83))
(translate
  (x -21.00)
  (y -14.00))
(fill-rect-with-gradient
  (rect at (0,0) size 30x20)
  (gradient 
    (p0 (0,0))
    (p1 (0,20))
    (color-interpolation-method sRGB premultiplied)
    (spread-method pad)
    (stops [#FF0000 at 0.00, #0000FF at 1.00])))
(restore)
(save)
(clip
  (rect at (5,0) size 20x5))
(save)
(clip
  (rect at (5,0) size 10x5))
(translate
  (x 5.00)
  (y 0.00))
(scale
  (size width=0.83 height=0.83))
(translate
  (x -9.00)
  (y 0.00))
(fill-rect-with-gradient
  (rect at (0,0) size 30x20)
  (gradient 
    (p0 (0,0))
    (p1 (0,20))
    (color-interpolation-method sRGB premultiplied)
    (spread-method pad)
    (stops [#FF0000 at 0.00, #0000FF at 1.00])))
(restore)
(save)
(clip
  (rect at (15,0) size 10x5))
(translate
  (x 15.00)
  (y 0.00))
(scale
  (size width=0.83 height=0.83))
(translate
  (x -9.00)
  (y 0.00))
(fill-rect-with-gradient
  (rect at (0,0) size 30x20)
  (gradient 
    (p0 (0,0))
    (p1 (0,20))
    (color-interpolation-method sRGB premultiplied)
    (spread-method pad)
    (stops [#FF0000 at 0.00, #0000FF at 1.00])))
(restore)
(restore)
(save)
(clip
  (rect at (5,15) size 20x5))
(save)
(clip
  (rect at (5,15) size 10x5))
(translate
  (x 5.00)
  (y 15.00))
(scale
  (size width=0.83 height=0.83))
(translate
  (x -9.00)
  (y -14.00))
(fill-rect-with-gradient
  (rect at (0,0) size 30x20)
  (gradient 
    (p0 (0,0))
    (p1 (0,20))
    (color-interpolation-method sRGB premultiplied)
    (spread-method pad)
    (stops [#FF0000 at 0.00, #0000FF at 1.00])))
(restore)
(save)
(clip
  (rect at (15,15) size 10x5))
(translate
  (x 15.00)
  (y 15.00))
(scale
  (size width=0.83 height=0.83))
(translate
  (x -9.00)
  (y -14.00))
(fill-rect-with-gradient
  (rect at (0,0) size 30x20)
  (gradient 
    (p0 (0,0))
    (p1 (0,20))
    (color-interpolation-method sRGB premultiplied)
    (spread-method pad)
    (stops [#FF0000 at 0.00, #0000FF at 1.00])))
(restore)
(restore))");
  }
}
