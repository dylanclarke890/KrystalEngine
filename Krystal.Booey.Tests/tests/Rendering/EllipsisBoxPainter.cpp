#include "Krystal.Booey/Rendering/EllipsisBoxPainter.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Rendering/PaintTestUtils.hpp"

namespace krys::boo::tests
{

  TEST_CASE("EllipsisBoxPainter: paints the ellipsis where the line is truncated",
            "[Rendering][EllipsisBoxPainter]")
  {
    CHECK(Painted(u8"<style>body{margin:0;font:16px "
                  u8"sans-serif}div{width:60px;overflow:hidden;white-space:nowrap;text-overflow:ellipsis}</"
                  u8"style><div>a long line of text</div>")
          == Canvas() + u8R"(
(save)
(clip
  (rect at (0,0) size 60x18))
(restore)
(save)
(clip
  (rect at (0,0) size 60x18))
(restore)
(save)
(clip
  (rect at (0,0) size 60x18))
(set-inline-fill-color
  (color #000000))
(draw-glyphs
  (local-anchor (0,14))
  (font-smoothing-mode auto)
  (length 6))
(draw-glyphs
  (local-anchor (43.59,14))
  (font-smoothing-mode auto)
  (length 1))
(restore)
(save)
(clip
  (rect at (0,0) size 60x18))
(restore))");
  }
}
