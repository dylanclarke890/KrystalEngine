#include "Krystal.Booey/Rendering/TextBoxPainter.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Rendering/PaintTestUtils.hpp"

namespace krys::boo::tests
{

  TEST_CASE("TextBoxPainter: draws text runs on the baseline in the text colour",
            "[Rendering][TextBoxPainter]")
  {
    CHECK(Painted(u8"<style>body{margin:0;font:16px sans-serif}</style>Hi there") == Canvas() + u8R"(
(set-inline-fill-color
  (color #000000))
(draw-glyphs
  (local-anchor (0,14))
  (font-smoothing-mode auto)
  (length 8)))");

    CHECK(Painted(u8"<style>body{margin:0;font:16px sans-serif;color:#118ab2}</style>Blue")
          == Canvas() + u8R"(
(set-inline-fill-color
  (color #118AB2))
(draw-glyphs
  (local-anchor (0,14))
  (font-smoothing-mode auto)
  (length 4)))");

    CHECK(Painted(u8"<style>body{margin:0;font:16px sans-serif}</style>a <b>bold</b> <span "
                  u8"style=font-size:24px>big</span>")
          == Canvas() + u8R"(
(set-inline-fill-color
  (color #000000))
(draw-glyphs
  (local-anchor (0,22))
  (font-smoothing-mode auto)
  (length 2))
(draw-glyphs
  (local-anchor (13.34,22))
  (font-smoothing-mode auto)
  (length 4))
(draw-glyphs
  (local-anchor (47.59,22))
  (font-smoothing-mode auto)
  (length 1))
(draw-glyphs
  (local-anchor (52.04,22))
  (font-smoothing-mode auto)
  (length 3)))");
  }

  TEST_CASE("TextBoxPainter: draws each text shadow, the last with the text", "[Rendering][TextBoxPainter]")
  {
    CHECK(Painted(u8"<style>body{margin:0;font:20px sans-serif;text-shadow:2px 3px 4px red, 1px 1px "
                  u8"green}</style>Shadow")
          == Canvas() + u8R"(
(set-inline-fill-color
  (color #000000))
(save)
(clip
  (rect at (-65,-65) size 204.28x154))
(set-state
  (change-flags [drop-shadow])
  (drop-shadow 
    (offset width=1 height=-308)
    (radius 0.00)
    (color #008000)
    (shadows-use-legacy-radius 0)
    (opacity 1.00)))
(draw-glyphs
  (local-anchor (0,327))
  (font-smoothing-mode auto)
  (length 6))
(restore)
(set-state
  (change-flags [drop-shadow])
  (drop-shadow 
    (offset width=2 height=3)
    (radius 4.00)
    (color #FF0000)
    (shadows-use-legacy-radius 0)
    (opacity 1.00)))
(draw-glyphs
  (local-anchor (0,18))
  (font-smoothing-mode auto)
  (length 6))
(set-state
  (change-flags [drop-shadow])
  (drop-shadow nullopt)))");
  }

  TEST_CASE("TextBoxPainter: places right-to-left text and skips hidden text", "[Rendering][TextBoxPainter]")
  {
    CHECK(Painted(u8"<style>body{margin:0;font:16px sans-serif}</style><div dir=rtl>abc</div>")
          == Canvas() + u8R"(
(set-inline-fill-color
  (color #000000))
(draw-glyphs
  (local-anchor (174.20,14))
  (font-smoothing-mode auto)
  (length 3)))");

    CHECK(
      Painted(u8"<style>body{margin:0;font:16px sans-serif}</style><div style=visibility:hidden>gone</div>")
      == Canvas() + u8R"()");
  }

  TEST_CASE("TextBoxPainter: draws emphasis marks over the text", "[Rendering][TextBoxPainter]")
  {
    CHECK(Painted(u8"<style>body{margin:0;font:16px sans-serif;text-emphasis:dot}</style>Dots")
          == Canvas() + u8R"(
(set-inline-fill-color
  (color #000000))
(draw-glyphs
  (local-anchor (0,23))
  (font-smoothing-mode auto)
  (length 4))
(draw-glyphs
  (local-anchor (4.65,7))
  (font-smoothing-mode auto)
  (length 4)))");
  }
}
