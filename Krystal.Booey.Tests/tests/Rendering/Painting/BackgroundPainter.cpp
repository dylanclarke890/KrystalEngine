#include "Krystal.Booey/Rendering/Painting/BackgroundPainter.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Loader/PageInFrameFixture.hpp"
#include "Krystal.Booey.Tests/Loader/TestLoader.hpp"
#include "Krystal.Booey.Tests/Rendering/PaintTestUtils.hpp"
#include "Krystal.Booey/Platform/PlatformStrategies.hpp"
#include "Krystal.Core/Threading/ManualWorkQueue.hpp"

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
          == Canvas() + u8R"(
(draw-pattern-imagebuffer
  (pattern-transform identity)
  (tile-rect at (0,0) size 1x10)
  (dest-rect at (0,0) size 40x20)
  (phase (0,0))
  (spacing width=0 height=0)))");

    CHECK(Painted(u8"<style>body{margin:0}div{width:40px;height:20px;background:linear-gradient(red,blue) "
                  u8"5px 5px/10px 10px no-repeat,yellow}</style><div></div>")
          == Canvas() + u8R"(
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

  TEST_CASE("BackgroundPainter: a large background image decodes on the work queue when first painted",
            "[Rendering][BackgroundPainter][css-backgrounds:background-image]")
  {
    // 400x400 decodes to 640KB, over the 500KB that makes a still image large for decoding.
    static constexpr uint8 LargePNG[] = {
      137, 80,  78,  71,  13,  10,  26,  10,  0,   0,   0,   13,  73,  72,  68,  82,  0,   0,   1,   144, 0,
      0,   1,   144, 8,   6,   0,   0,   0,   128, 191, 54,  204, 0,   0,   4,   130, 73,  68,  65,  84,  120,
      218, 237, 213, 49,  1,   0,   0,   8,   195, 176, 73,  199, 249, 208, 192, 77,  142, 24,  232, 211, 116,
      82,  0,   184, 138, 8,   0,   24,  8,   0,   6,   2,   128, 129, 0,   96,  32,  0,   96,  32,  0,   24,
      8,   0,   6,   2,   128, 129, 0,   96,  32,  0,   96,  32,  0,   24,  8,   0,   6,   2,   128, 129, 0,
      96,  32,  0,   96,  32,  0,   24,  8,   0,   6,   2,   128, 129, 0,   96,  32,  0,   96,  32,  0,   24,
      8,   0,   6,   2,   128, 129, 0,   96,  32,  0,   96,  32,  0,   24,  8,   0,   6,   2,   128, 129, 0,
      96,  32,  0,   96,  32,  0,   24,  8,   0,   6,   2,   128, 129, 0,   96,  32,  0,   96,  32,  0,   24,
      8,   0,   6,   2,   128, 129, 0,   128, 129, 0,   96,  32,  0,   24,  8,   0,   6,   2,   128, 129, 0,
      128, 129, 0,   96,  32,  0,   24,  8,   0,   6,   2,   128, 129, 0,   128, 129, 0,   96,  32,  0,   24,
      8,   0,   6,   2,   128, 129, 0,   128, 129, 0,   96,  32,  0,   24,  8,   0,   6,   2,   128, 129, 0,
      128, 129, 0,   96,  32,  0,   24,  8,   0,   6,   2,   128, 129, 0,   128, 129, 0,   96,  32,  0,   24,
      8,   0,   6,   2,   128, 129, 0,   128, 129, 0,   96,  32,  0,   24,  8,   0,   6,   2,   128, 129, 136,
      0,   128, 129, 0,   96,  32,  0,   24,  8,   0,   6,   2,   0,   6,   2,   128, 129, 0,   96,  32,  0,
      24,  8,   0,   6,   2,   0,   6,   2,   128, 129, 0,   96,  32,  0,   24,  8,   0,   6,   2,   0,   6,
      2,   128, 129, 0,   96,  32,  0,   24,  8,   0,   6,   2,   0,   6,   2,   128, 129, 0,   96,  32,  0,
      24,  8,   0,   6,   2,   0,   6,   2,   128, 129, 0,   96,  32,  0,   24,  8,   0,   6,   2,   0,   6,
      2,   128, 129, 0,   96,  32,  0,   24,  8,   0,   6,   2,   0,   6,   2,   128, 129, 0,   96,  32,  0,
      24,  8,   0,   24,  8,   0,   6,   2,   128, 129, 0,   96,  32,  0,   24,  8,   0,   24,  8,   0,   6,
      2,   128, 129, 0,   96,  32,  0,   24,  8,   0,   24,  8,   0,   6,   2,   128, 129, 0,   96,  32,  0,
      24,  8,   0,   24,  8,   0,   6,   2,   128, 129, 0,   96,  32,  0,   24,  8,   0,   24,  8,   0,   6,
      2,   128, 129, 0,   96,  32,  0,   24,  8,   0,   24,  8,   0,   6,   2,   128, 129, 0,   96,  32,  0,
      24,  8,   0,   24,  8,   0,   6,   2,   128, 129, 0,   96,  32,  0,   24,  136, 8,   0,   24,  8,   0,
      6,   2,   128, 129, 0,   96,  32,  0,   96,  32,  0,   24,  8,   0,   6,   2,   128, 129, 0,   96,  32,
      0,   96,  32,  0,   24,  8,   0,   6,   2,   128, 129, 0,   96,  32,  0,   96,  32,  0,   24,  8,   0,
      6,   2,   128, 129, 0,   96,  32,  0,   96,  32,  0,   24,  8,   0,   6,   2,   128, 129, 0,   96,  32,
      0,   96,  32,  0,   24,  8,   0,   6,   2,   128, 129, 0,   96,  32,  0,   96,  32,  0,   24,  8,   0,
      6,   2,   128, 129, 0,   96,  32,  0,   96,  32,  0,   24,  8,   0,   6,   2,   128, 129, 0,   128, 129,
      0,   96,  32,  0,   24,  8,   0,   6,   2,   128, 129, 0,   128, 129, 0,   96,  32,  0,   24,  8,   0,
      6,   2,   128, 129, 0,   128, 129, 0,   96,  32,  0,   24,  8,   0,   6,   2,   128, 129, 0,   128, 129,
      0,   96,  32,  0,   24,  8,   0,   6,   2,   128, 129, 0,   128, 129, 0,   96,  32,  0,   24,  8,   0,
      6,   2,   128, 129, 0,   128, 129, 0,   96,  32,  0,   24,  8,   0,   6,   2,   128, 129, 0,   128, 129,
      0,   96,  32,  0,   24,  8,   0,   6,   2,   128, 129, 8,   1,   128, 129, 0,   96,  32,  0,   24,  8,
      0,   6,   2,   0,   6,   2,   128, 129, 0,   96,  32,  0,   24,  8,   0,   6,   2,   0,   6,   2,   128,
      129, 0,   96,  32,  0,   24,  8,   0,   6,   2,   0,   6,   2,   128, 129, 0,   96,  32,  0,   24,  8,
      0,   6,   2,   0,   6,   2,   128, 129, 0,   96,  32,  0,   24,  8,   0,   6,   2,   0,   6,   2,   128,
      129, 0,   96,  32,  0,   24,  8,   0,   6,   2,   0,   6,   2,   128, 129, 0,   96,  32,  0,   24,  8,
      0,   6,   2,   0,   6,   2,   128, 129, 0,   96,  32,  0,   24,  8,   0,   6,   34,  2,   0,   6,   2,
      128, 129, 0,   96,  32,  0,   24,  8,   0,   24,  8,   0,   6,   2,   128, 129, 0,   96,  32,  0,   24,
      8,   0,   24,  8,   0,   6,   2,   128, 129, 0,   96,  32,  0,   24,  8,   0,   24,  8,   0,   6,   2,
      128, 129, 0,   96,  32,  0,   24,  8,   0,   24,  8,   0,   6,   2,   128, 129, 0,   96,  32,  0,   24,
      8,   0,   24,  8,   0,   6,   2,   128, 129, 0,   96,  32,  0,   24,  8,   0,   24,  8,   0,   6,   2,
      128, 129, 0,   96,  32,  0,   24,  8,   0,   24,  8,   0,   6,   2,   128, 129, 0,   96,  32,  0,   96,
      32,  0,   24,  8,   0,   6,   2,   128, 129, 0,   96,  32,  0,   96,  32,  0,   24,  8,   0,   6,   2,
      128, 129, 0,   96,  32,  0,   96,  32,  0,   24,  8,   0,   6,   2,   128, 129, 0,   96,  32,  0,   96,
      32,  0,   24,  8,   0,   6,   2,   128, 129, 0,   96,  32,  0,   96,  32,  0,   24,  8,   0,   6,   2,
      128, 129, 0,   96,  32,  0,   96,  32,  0,   24,  8,   0,   6,   2,   128, 129, 0,   96,  32,  0,   96,
      32,  0,   24,  8,   0,   6,   2,   128, 129, 0,   96,  32,  34,  0,   96,  32,  0,   24,  8,   0,   6,
      2,   128, 129, 0,   128, 129, 0,   96,  32,  0,   24,  8,   0,   6,   2,   128, 129, 0,   128, 129, 0,
      96,  32,  0,   24,  8,   0,   6,   2,   128, 129, 0,   128, 129, 0,   96,  32,  0,   24,  8,   0,   6,
      2,   128, 129, 0,   128, 129, 0,   96,  32,  0,   24,  8,   0,   6,   2,   128, 129, 0,   128, 129, 0,
      96,  32,  0,   24,  8,   0,   6,   2,   128, 129, 0,   128, 129, 0,   96,  32,  0,   24,  8,   0,   6,
      2,   128, 129, 0,   128, 129, 0,   96,  32,  0,   24,  8,   0,   6,   2,   0,   6,   2,   128, 129, 0,
      96,  32,  0,   24,  8,   0,   6,   2,   0,   6,   2,   128, 129, 0,   96,  32,  0,   24,  8,   0,   6,
      2,   0,   6,   2,   128, 129, 0,   96,  32,  0,   24,  8,   0,   6,   2,   0,   6,   2,   128, 129, 0,
      96,  32,  0,   24,  8,   0,   6,   2,   0,   6,   2,   128, 129, 0,   96,  32,  0,   24,  8,   0,   6,
      2,   0,   6,   2,   128, 129, 0,   96,  32,  0,   24,  8,   0,   95,  45,  57,  38,  249, 60,  121, 38,
      169, 223, 0,   0,   0,   0,   73,  69,  78,  68,  174, 66,  96,  130};
    LoaderFixture loader;
    loader.Strategy.Resources[u8"file:///large.png"] = Span<const uint8>(LargePNG);
    ManualWorkQueue queue;
    PlatformStrategies::Singleton().SetWorkQueue(&queue);

    PageInFrameFixture fixture(u8"<style>body{margin:0}</style><div "
                               u8"style='width:400px;height:400px;background:url(file:///large.png)'>",
                               IntSize(500, 500));
    (void)fixture.Painted();
    RunNetworkingTasks();

    auto drawsImage = [&]
    {
      return fixture.Painted().find(u8"native-image") != utf8_string::npos;
    };
    CHECK_FALSE(drawsImage());
    CHECK(queue.PendingCount() == 1);
    CHECK(queue.RunAll() == 1);
    RunNetworkingTasks();
    CHECK(drawsImage());

    PlatformStrategies::Singleton().SetWorkQueue(nullptr);
  }

  TEST_CASE("BackgroundPainter: clips to the text with a mask", "[Rendering][BackgroundPainter]")
  {
    auto painted = Painted(u8"<style>body{margin:0;font:20px/1 sans-serif}div{width:60px;color:transparent;"
                           u8"background:green;background-clip:text}</style><div>Hi</div>");
    // The text is painted into a layer, and the background into another composited onto it with source-in.
    auto mask = painted.find(u8"(begin-transparency-layer");
    REQUIRE(mask != utf8_string::npos);
    auto maskText = painted.find(u8"(draw-glyphs", mask);
    auto sourceIn = painted.find(u8"(composite-operation source-in)", mask);
    auto background = painted.find(u8"(color #008000)", mask);
    CHECK(maskText < sourceIn);
    CHECK(sourceIn < background);
    CHECK(painted.find(u8"(begin-transparency-layer", mask + 1uz) < background);
    // The text itself is painted afterwards, transparent.
    CHECK(painted.find(u8"(draw-glyphs", background) != utf8_string::npos);
  }

  TEST_CASE("BackgroundPainter: clips to the border area", "[Rendering][BackgroundPainter]")
  {
    // A solid border's area is a path to clip to.
    auto solid = Painted(u8"<style>body{margin:0}div{width:40px;height:20px;border:4px solid "
                         u8"transparent;background:green border-area}</style><div></div>");
    CHECK(solid.find(u8R"((clip-path
  (path add rect at (0,0) size 48x28, add rect at (4,4) size 40x20)
  (wind-rule EVEN-ODD)))")
          != utf8_string::npos);
    CHECK(solid.find(u8"(begin-transparency-layer") == utf8_string::npos);

    // Other borders are painted into a mask.
    auto dotted = Painted(u8"<style>body{margin:0}div{width:40px;height:20px;border:4px dotted "
                          u8"transparent;background:green border-area}</style><div></div>");
    auto mask = dotted.find(u8"(begin-transparency-layer");
    REQUIRE(mask != utf8_string::npos);
    auto border = dotted.find(u8"(stroke-style dotted-stroke)", mask);
    auto sourceIn = dotted.find(u8"(composite-operation source-in)", mask);
    CHECK(border < sourceIn);
    CHECK(sourceIn < dotted.find(u8"(color #008000)", mask));
  }
}
