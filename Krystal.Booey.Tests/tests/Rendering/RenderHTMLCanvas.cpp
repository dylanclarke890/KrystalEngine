#include "Krystal.Booey/Rendering/RenderHTMLCanvas.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Loader/PageInFrameFixture.hpp"
#include "Krystal.Booey/HTML/HTMLCanvasElement.hpp"
#include "Krystal.Booey/Rendering/RenderLayer.hpp"
#include "Krystal.Booey/Rendering/RenderLayerBacking.hpp"
#include "Krystal.Booey/Rendering/RenderTreeAsText.hpp"

namespace krys::boo::tests
{
  namespace
  {
    struct CanvasFixture : PageInFrameFixture
    {
      explicit CanvasFixture(utf8_string &&markup) noexcept
          : PageInFrameFixture(krys::move(markup), {800, 600})
      {
      }

      KRYS_NODISCARD html::HTMLCanvasElement &Canvas(dom::DOMStringView id) noexcept
      {
        return Downcast<html::HTMLCanvasElement>(ById(id));
      }

      KRYS_NODISCARD RenderHTMLCanvas &Renderer(dom::DOMStringView id) noexcept
      {
        (void)Document->UpdateLayout();
        auto *renderer = DynamicDowncast<RenderHTMLCanvas>(ById(id).Renderer());
        REQUIRE(renderer);
        return *renderer;
      }
    };

    KRYS_NODISCARD size_t Count(const utf8_string &text, utf8_stringview part) noexcept
    {
      size_t count = 0;
      for (auto position = text.find(part); position != utf8_string::npos;
           position = text.find(part, position + 1))
      {
        ++count;
      }
      return count;
    }
  }

  TEST_CASE("RenderHTMLCanvas: the canvas's size is its natural size", "[Rendering][RenderHTMLCanvas]")
  {
    CanvasFixture fixture(u8"<canvas id=plain></canvas><canvas id=sized width=40 height=20></canvas>"
                          u8"<canvas id=wide width=40 height=20 style='width:80px'></canvas>"
                          u8"<canvas id=tall width=40 height=20 style='height:60px'></canvas>"
                          u8"<canvas id=both width=40 height=20 style='width:30px;height:30px'></canvas>"
                          u8"<canvas id=zoomed width=40 height=20 style='zoom:2'></canvas>");

    CHECK(fixture.RendererSize(u8"plain") == IntSize(300, 150));
    CHECK(fixture.RendererSize(u8"sized") == IntSize(40, 20));
    // Its natural aspect ratio sizes the other dimension.
    CHECK(fixture.RendererSize(u8"wide") == IntSize(80, 40));
    CHECK(fixture.RendererSize(u8"tall") == IntSize(120, 60));
    CHECK(fixture.RendererSize(u8"both") == IntSize(30, 30));
    CHECK(fixture.RendererSize(u8"zoomed") == IntSize(80, 40));
    CHECK(fixture.Renderer(u8"sized").HasIntrinsicAspectRatio());

    SECTION("resizing the canvas lays it out again")
    {
      fixture.Canvas(u8"sized").Width(60);
      CHECK(fixture.RendererSize(u8"sized") == IntSize(60, 20));
      fixture.Canvas(u8"wide").Height(80);
      CHECK(fixture.RendererSize(u8"wide") == IntSize(80, 160));
    }
  }

  TEST_CASE("RenderHTMLCanvas: object-fit places the canvas in its box", "[Rendering][RenderHTMLCanvas]")
  {
    CanvasFixture fixture(
      u8"<canvas id=fill width=40 height=20 style='width:100px;height:100px'></canvas>"
      u8"<canvas id=contain width=40 height=20 style='width:100px;height:100px;object-fit:contain'></canvas>"
      u8"<canvas id=none width=40 height=20 style='width:100px;height:100px;object-fit:none;"
      u8"object-position:0 0'></canvas>");

    CHECK(fixture.Renderer(u8"fill").ReplacedContentRect() == LayoutRect(0, 0, 100, 100));
    CHECK(fixture.Renderer(u8"contain").ReplacedContentRect() == LayoutRect(0, 25, 100, 50));
    CHECK(fixture.Renderer(u8"none").ReplacedContentRect() == LayoutRect(0, 0, 40, 20));
  }

  TEST_CASE("RenderHTMLCanvas: the render tree", "[Rendering][RenderHTMLCanvas]")
  {
    CanvasFixture fixture(u8"<body style='margin:0'>x<canvas id=canvas width=40 height=20>Fallback "
                          u8"<b>text</b></canvas></body>");
    (void)fixture.Document->UpdateLayout();

    // The canvas sits on the baseline, and its fallback content has no renderers.
    CHECK(ExternalRepresentation(fixture.Frame.get()) == u8R"(layer at (0,0) size 800x600
  RenderView at (0,0) size 800x600
layer at (0,0) size 800x24
  RenderBlock {HTML} at (0,0) size 800x24
    RenderBody {BODY} at (0,0) size 800x24
      RenderText {#text} at (0,6) size 8x17
        text run at (0,6) width 8: "x"
      RenderHTMLCanvas {CANVAS} at (8,0) size 40x20
)");

    // Nothing is painted without a context, and the fallback content isn't.
    auto painted = fixture.Painted();
    CHECK(Count(painted, u8"draw-native-image") == 0u);
    CHECK(Count(painted, u8"draw-glyphs") == 1u);
  }

  TEST_CASE("RenderHTMLCanvas: layers", "[Rendering][RenderHTMLCanvas]")
  {
    CanvasFixture fixture(
      u8"<canvas id=plain></canvas><canvas id=positioned style='position:relative'></canvas>"
      u8"<canvas id=bordered style='position:relative;border:1px solid'></canvas>"
      u8"<canvas id=faded style='opacity:0.5'></canvas>"
      u8"<canvas id=filtered style='opacity:0.5;filter:blur(1px)'></canvas>");

    // A canvas without a context paints into its enclosing layer, so it needs no layer of its own.
    auto &plain = fixture.Renderer(u8"plain");
    CHECK_FALSE(plain.HasLayer());
    CHECK(GetCanvasCompositingStrategy(plain) == CanvasCompositingStrategy::CanvasPaintedToEnclosingLayer);

    // When it has one, the layer is self-painting and paints only the canvas, unless it has decorations.
    auto *positioned = fixture.Renderer(u8"positioned").Layer();
    REQUIRE(positioned);
    CHECK(positioned->IsSelfPaintingLayer());
    CHECK(positioned->IsBitmapOnly());
    auto *bordered = fixture.Renderer(u8"bordered").Layer();
    REQUIRE(bordered);
    CHECK_FALSE(bordered->IsBitmapOnly());

    // Its opacity is applied as an alpha, unless there's a filter.
    auto *faded = fixture.Renderer(u8"faded").Layer();
    REQUIRE(faded);
    CHECK(faded->CanPaintTransparencyWithSetOpacity());
    auto *filtered = fixture.Renderer(u8"filtered").Layer();
    REQUIRE(filtered);
    CHECK_FALSE(filtered->CanPaintTransparencyWithSetOpacity());
  }
}
