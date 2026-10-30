#include "Krystal.Booey/Rendering/TextDecorationPainter.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Rendering/PaintTestUtils.hpp"

namespace krys::boo::tests
{

  TEST_CASE(
    "TextDecorationPainter: draws underlines, overlines and line-throughs in their colour and thickness",
    "[Rendering][TextDecorationPainter]")
  {
    CHECK(
      Painted(
        u8"<style>body{margin:0;font:16px "
        u8"sans-serif}span{text-decoration:underline;text-decoration-skip-ink:none}</style><span>Hi</span>")
      == Canvas() + u8R"(
(set-inline-fill-color
  (color #000000))
(set-inline-stroke
  (color #000000))
(draw-lines-for-text
  (point (0,15))
  (thickness 1.00)
  (double 0)
  (lineSegments [
    (
      (begin 0.00)
      (end 15.00))])
  (is-printing 0)
  (double 0))
(draw-glyphs
  (local-anchor (0,14))
  (font-smoothing-mode auto)
  (length 2)))");

    CHECK(Painted(u8"<style>body{margin:0;font:16px sans-serif}span{text-decoration:overline line-through "
                  u8"blue;text-decoration-thickness:3px}</style><span>Hi</span>")
          == Canvas() + u8R"(
(set-inline-fill-color
  (color #000000))
(set-inline-stroke
  (color #0000FF))
(draw-lines-for-text
  (point (0,-2))
  (thickness 3.00)
  (double 0)
  (lineSegments [
    (
      (begin 0.00)
      (end 15.00))])
  (is-printing 0)
  (double 0))
(draw-glyphs
  (local-anchor (0,14))
  (font-smoothing-mode auto)
  (length 2))
(draw-lines-for-text
  (point (0,8.66))
  (thickness 3.00)
  (double 0)
  (lineSegments [
    (
      (begin 0.00)
      (end 15.00))])
  (is-printing 0)
  (double 0)))");
  }

  TEST_CASE("TextDecorationPainter: skips the ink of descenders", "[Rendering][TextDecorationPainter]")
  {
    CHECK(Painted(u8"<style>body{margin:0;font:16px sans-serif}span{text-decoration:underline "
                  u8"red}</style><span>gypsy</span>")
          == Canvas() + u8R"(
(set-inline-fill-color
  (color #000000))
(set-inline-stroke
  (color #FF0000))
(draw-lines-for-text
  (point (0,15))
  (thickness 1.00)
  (double 0)
  (lineSegments [
    (
      (begin 7.59)
      (end 9.32)), 
    (
      (begin 14.13)
      (end 16.98)), 
    (
      (begin 20.38)
      (end 34.22)), 
    (
      (begin 39.03)
      (end 42.00))])
  (is-printing 0)
  (double 0))
(draw-glyphs
  (local-anchor (0,14))
  (font-smoothing-mode auto)
  (length 5)))");
  }

  TEST_CASE("TextDecorationPainter: draws double and wavy lines", "[Rendering][TextDecorationPainter]")
  {
    CHECK(Painted(u8"<style>body{margin:0;font:16px sans-serif}span{text-decoration:underline "
                  u8"double;text-decoration-skip-ink:none}</style><span>Hi</span>")
          == Canvas() + u8R"(
(set-inline-fill-color
  (color #000000))
(set-inline-stroke
  (color #000000))
(draw-lines-for-text
  (point (0,15))
  (thickness 1.00)
  (double 1)
  (lineSegments [
    (
      (begin 0.00)
      (end 15.00))])
  (is-printing 0)
  (double 1))
(draw-glyphs
  (local-anchor (0,14))
  (font-smoothing-mode auto)
  (length 2)))");

    CHECK(Painted(u8"<style>body{margin:0;font:16px sans-serif}span{text-decoration:underline wavy "
                  u8"green}</style><span>Hi</span>")
          == Canvas() + u8R"(
(set-inline-fill-color
  (color #000000))
(set-inline-stroke
  (color #008000))
(save)
(clip
  (rect at (0,14.50) size 15x4))
(set-inline-stroke
  (thickness 1.00))
(stroke-path
  (path move to (-7.11,16.50), add curve to (-3.56,18) (-3.56,15) (0,16.50), add curve to (3.56,18) (3.56,15) (7.11,16.50), add curve to (10.67,18) (10.67,15) (14.22,16.50), add curve to (17.78,18) (17.78,15) (21.33,16.50)))
(restore)
(draw-glyphs
  (local-anchor (0,14))
  (font-smoothing-mode auto)
  (length 2)))");
  }

  TEST_CASE("TextDecorationPainter: casts the text's shadow from its decorations",
            "[Rendering][TextDecorationPainter]")
  {
    CHECK(Painted(u8"<style>body{margin:0;font:16px sans-serif}span{text-decoration:underline "
                  u8"dotted;text-decoration-skip-ink:none;text-shadow:2px 2px red}</style><span>Hi</span>")
          == Canvas() + u8R"(
(set-state
  (change-flags [fill-brush, stroke-brush, drop-shadow])
  (fill-brush 
    (color #000000))
  (stroke-brush 
    (color #000000))
  (drop-shadow 
    (offset width=2 height=2)
    (radius 0.00)
    (color #FF0000)
    (shadows-use-legacy-radius 0)
    (opacity 1.00)))
(draw-lines-for-text
  (point (0,15))
  (thickness 1.00)
  (double 0)
  (lineSegments [
    (
      (begin 0.00)
      (end 15.00))])
  (is-printing 0)
  (double 0))
(draw-glyphs
  (local-anchor (0,14))
  (font-smoothing-mode auto)
  (length 2))
(set-state
  (change-flags [drop-shadow])
  (drop-shadow nullopt)))");
  }
}
