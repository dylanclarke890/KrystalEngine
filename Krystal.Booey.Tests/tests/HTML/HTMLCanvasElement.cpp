#include "Krystal.Booey/HTML/HTMLCanvasElement.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/HTML/FakeEngineTexture.hpp"
#include "Krystal.Booey.Tests/Loader/PageInFrameFixture.hpp"
#include "Krystal.Booey/HTML/Algorithms/FocusAlgorithms.hpp"
#include "Krystal.Booey/HTML/Canvas/EngineTextureRenderingContext.hpp"
#include "Krystal.Booey/Rendering/RenderHTMLCanvas.hpp"

namespace krys::boo::tests
{
  namespace
  {
    struct CanvasFixture : PageInFrameFixture
    {
      using PageInFrameFixture::PageInFrameFixture;

      KRYS_NODISCARD html::HTMLCanvasElement &Canvas(dom::DOMStringView id) noexcept
      {
        return Downcast<html::HTMLCanvasElement>(ById(id));
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

  TEST_CASE("HTMLCanvasElement: the width and height and the canvas's size", "[HTML][HTMLCanvasElement]")
  {
    CanvasFixture fixture(u8"<canvas id=plain></canvas><canvas id=sized width=40 height=20></canvas>"
                          u8"<canvas id=invalid width=abc height=-5></canvas>");

    // Without attributes, a canvas is 300 by 150.
    auto &plain = fixture.Canvas(u8"plain");
    CHECK(plain.Width() == 300u);
    CHECK(plain.Height() == 150u);
    CHECK(plain.Size() == IntSize(300, 150));

    auto &sized = fixture.Canvas(u8"sized");
    CHECK(sized.Width() == 40u);
    CHECK(sized.Height() == 20u);
    CHECK(sized.Size() == IntSize(40, 20));

    // Values that don't parse as non-negative integers are the defaults.
    auto &invalid = fixture.Canvas(u8"invalid");
    CHECK(invalid.Width() == 300u);
    CHECK(invalid.Height() == 150u);
    CHECK(invalid.Size() == IntSize(300, 150));

    SECTION("setting the attributes resizes the canvas")
    {
      plain.Width(64);
      CHECK(plain.GetAttribute(u8"width") == u8"64");
      CHECK(plain.Size() == IntSize(64, 150));
      REQUIRE_FALSE(plain.SetAttribute(u8"height", u8"32").HasException());
      CHECK(plain.Height() == 32u);
      CHECK(plain.Size() == IntSize(64, 32));
      plain.RemoveAttribute(u8"width");
      CHECK(plain.Size() == IntSize(300, 32));
    }

    SECTION("values too large for a long are the defaults")
    {
      plain.Width(3'000'000'000u);
      CHECK(plain.Width() == 300u);
      CHECK(plain.Size() == IntSize(300, 150));
    }
  }

  TEST_CASE("HTMLCanvasElement: contexts", "[HTML][HTMLCanvasElement]")
  {
    CanvasFixture fixture(u8"<canvas id=canvas></canvas>");
    auto &canvas = fixture.Canvas(u8"canvas");

    // WebGL, WebGPU and bitmap renderers aren't planned.
    CHECK(canvas.RenderingContext() == nullptr);
    for (auto type : {u8"2d", u8"webgl", u8"webgl2", u8"webgpu", u8"bitmaprenderer", u8"unknown"})
    {
      CHECK(canvas.GetContext(type) == nullptr);
    }
    CHECK(canvas.RenderingContext() == nullptr);

    // The engine texture context is made once, and then it's the only context there is.
    auto context = canvas.GetContext(u8"krystal-texture");
    REQUIRE(context);
    CHECK(context->IsEngineTexture());
    CHECK(context->IsGPUBased());
    CHECK(context->DelegatesDisplay());
    CHECK(&context->GetCanvasBase() == &canvas);
    CHECK(canvas.RenderingContext() == context.get());
    CHECK(canvas.GetContext(u8"krystal-texture") == context);
    CHECK(canvas.GetContext(u8"2d") == nullptr);
  }

  TEST_CASE("HTMLCanvasElement: a texture canvas painted rather than composited", "[HTML][HTMLCanvasElement]")
  {
    CanvasFixture fixture(u8"<canvas id=canvas width=40 height=20 style='width:80px'></canvas>");
    auto &canvas = fixture.Canvas(u8"canvas");
    auto &context = Downcast<html::EngineTextureRenderingContext>(*canvas.GetContext(u8"krystal-texture"));

    // Without a texture, the canvas is transparent black and nothing is drawn.
    CHECK(
      context.IsSurfaceBufferTransparentBlack(html::CanvasRenderingContext::SurfaceBuffer::DisplayBuffer));
    CHECK(Count(fixture.Painted(), u8"draw-native-image") == 0u);

    // Its texture is drawn as an image, scaled to the canvas's box.
    auto texture = AdoptRef(*new FakeEngineTexture({40, 20}));
    context.SetTexture(texture);
    CHECK(context.Texture() == texture.get());
    auto painted = fixture.Painted();
    CHECK(Count(painted, u8"draw-native-image") == 1u);
    CHECK(Count(painted, u8"(source-rect at (0,0) size 40x20)") == 1u);
    CHECK(Count(painted, u8"(dest-rect at (8,8) size 80x40)") == 1u);
  }

  TEST_CASE("HTMLCanvasElement: always replaced, its children fallback content", "[HTML][HTMLCanvasElement]")
  {
    CanvasFixture fixture(
      u8"<canvas id=canvas><p id=fallback>Fallback <button id=button>Go</button></p></canvas>"
      u8"<canvas id=hidden style='display:none'><button id=hiddenButton>Go</button></canvas>"
      u8"<canvas id=invisible style='visibility:hidden'><button id=invisibleButton "
      u8"style='visibility:visible'>Go</button></canvas>");
    auto &canvas = fixture.Canvas(u8"canvas");
    CHECK(canvas.IsReplaced());
    (void)fixture.Document->UpdateLayout();

    // The canvas has a canvas renderer, and its children have none.
    CHECK(Is<RenderHTMLCanvas>(canvas.Renderer()));
    CHECK(fixture.ById(u8"fallback").Renderer() == nullptr);
    CHECK(fixture.ById(u8"button").Renderer() == nullptr);

    // Fallback content is focusable while its canvas is rendered.
    CHECK(html::FocusAlgorithms::IsFocusableArea(fixture.ById(u8"button")));
    CHECK_FALSE(html::FocusAlgorithms::IsFocusableArea(fixture.ById(u8"hiddenButton")));
    CHECK_FALSE(html::FocusAlgorithms::IsFocusableArea(fixture.ById(u8"invisibleButton")));

    SECTION("not once the canvas isn't rendered")
    {
      REQUIRE_FALSE(canvas.SetAttribute(u8"style", u8"display:none").HasException());
      fixture.Document->UpdateStyleIfNeeded();
      CHECK_FALSE(html::FocusAlgorithms::IsFocusableArea(fixture.ById(u8"button")));
    }

    SECTION("nor is fallback content that isn't rendered itself")
    {
      REQUIRE_FALSE(fixture.ById(u8"button").SetAttribute(u8"style", u8"display:none").HasException());
      fixture.Document->UpdateStyleIfNeeded();
      CHECK_FALSE(html::FocusAlgorithms::IsFocusableArea(fixture.ById(u8"button")));
    }
  }
}
