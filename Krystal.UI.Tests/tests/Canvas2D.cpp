#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/Graphics/ImageBuffer.hpp"
#include "Krystal.Booey/Graphics/ImageFrame.hpp"
#include "Krystal.Booey/HTML/Canvas/CanvasRenderingContext2D.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/HTML/HTMLCanvasElement.hpp"
#include "Krystal.Booey/HTML/HTMLImageElement.hpp"
#include "Krystal.Booey/HTML/ImageData.hpp"
#include "Krystal.Booey/Platform/ImageDecoders/ScalableImageDecoder.hpp"
#include "Krystal.Booey/Platform/SharedBuffer.hpp"
#include "Krystal.Booey/SVG/SVGImageElement.hpp"
#include "Krystal.Core/Text/Base64.hpp"
#include "Krystal.UI.Tests/RepaintFixture.hpp"
#include "Krystal.UI.Tests/ViewTestUtils.hpp"
#include "Krystal.UI/View.hpp"
#include <chrono>
#include <string>

namespace krys::ui::tests
{
  using namespace std::chrono_literals;

  namespace
  {
    using Pixel = Array<uint8, 4>;

    constexpr Pixel Black {0, 0, 0, 255};
    constexpr Pixel Red {255, 0, 0, 255};
    constexpr Pixel Blue {0, 0, 255, 255};
    constexpr Pixel Lime {0, 255, 0, 255};
    constexpr Pixel Transparent {0, 0, 0, 0};

    /// @brief A 100 by 50 canvas at the top left of a black page, drawn by a 2D context, shown by a host that
    /// updates and paints the view each frame; @p extra is more of the page.
    class Canvas2DFixture
    {
      KRYS_NON_COPY_MOVABLE(Canvas2DFixture);

    private:
      RepaintFixture _paint {100u, 50u};
      View _view {{100, 50}};
      MonotonicTime::duration _time;
      RefPtr<boo::html::CanvasRenderingContext2D> _context;

    public:
      explicit Canvas2DFixture(std::u8string_view extra = {})
      {
        std::u8string html(
          u8"<!DOCTYPE html><style>html { background: black } body { margin: 0 }"
          u8" canvas { display: block }</style><canvas id=canvas width=100 height=50></canvas>");
        html += extra;
        _view.LoadHTML(krys::move(html));
        WaitForLoad(_view);
        _time = boo::html::EventLoop::Current().Now() + 1s;
        _context = Canvas().GetContext2d();
        REQUIRE(_context);
      }

      KRYS_NODISCARD View &GetView() noexcept
      {
        return _view;
      }
      KRYS_NODISCARD RepaintFixture &Paint() noexcept
      {
        return _paint;
      }
      KRYS_NODISCARD boo::html::HTMLCanvasElement &Canvas(boo::dom::DOMStringView id = u8"canvas") noexcept
      {
        return Downcast<boo::html::HTMLCanvasElement>(*_view.Document().GetElementById(id));
      }
      KRYS_NODISCARD boo::html::CanvasRenderingContext2D &Context() noexcept
      {
        return *_context;
      }

      /// @brief The next frame: the rendering update, then the page composited.
      KRYS_NODISCARD GoldenImage Frame()
      {
        _time += 16ms;
        (void)_view.Update(_time);
        return _paint.Painted(_view);
      }
    };

    KRYS_NODISCARD std::string ToString(const Pixel &pixel)
    {
      return std::to_string(pixel[0]) + "," + std::to_string(pixel[1]) + "," + std::to_string(pixel[2]) + ","
             + std::to_string(pixel[3]);
    }

    /// @brief The pixel at @p x, @p y of what @p context reads back, unpremultiplied.
    KRYS_NODISCARD Pixel ReadPixel(boo::html::CanvasRenderingContext2D &context, int32 x, int32 y)
    {
      auto read = context.GetImageData(x, y, 1, 1, {});
      REQUIRE_FALSE(read.HasException());
      auto bytes = read.Value()->Data().AsUint8ClampedArray();
      return {bytes->Item(0), bytes->Item(1), bytes->Item(2), bytes->Item(3)};
    }

    /// @brief Checks @p actual is @p expected, within 2 a channel.
    void CheckRead(const Pixel &actual, const Pixel &expected)
    {
      INFO("read " << ToString(actual) << ", expected " << ToString(expected));
      bool close = true;
      for (size_t channel = 0; channel < 4u; ++channel)
      {
        close = close && std::abs(static_cast<int32>(actual[channel]) - expected[channel]) <= 2;
      }
      CHECK(close);
    }

    KRYS_NODISCARD Pixel At(const GoldenImage &image, uint32 x, uint32 y) noexcept
    {
      return image.Pixels[static_cast<size_t>(y) * image.Width + x];
    }

    /// @brief Checks the pixel at @p x, @p y, as WPT's `_assertPixel` does, within 2 a channel.
    void CheckPixel(const GoldenImage &image, uint32 x, uint32 y, const Pixel &expected)
    {
      const auto actual = At(image, x, y);
      INFO("pixel " << x << "," << y << " is " << ToString(actual) << ", expected " << ToString(expected));
      bool close = true;
      for (size_t channel = 0; channel < 4u; ++channel)
      {
        close = close && std::abs(static_cast<int32>(actual[channel]) - expected[channel]) <= 2;
      }
      CHECK(close);
    }
  }

  TEST_CASE("Canvas 2D: draws into a texture it keeps", "[UI][GPU][Canvas]")
  {
    Canvas2DFixture fixture;
    auto &context = fixture.Context();

    context.SetFillStyle(boo::dom::DOMString(u8"red"));
    context.FillRect(0, 0, 50, 50);
    auto *buffer = fixture.Canvas().Buffer();
    REQUIRE(buffer);
    CHECK(buffer->GetRenderingMode() == boo::RenderingMode::Accelerated);
    CHECK(context.DelegatesDisplay());

    auto image = fixture.Frame();
    CheckPixel(image, 25, 25, Red);
    CheckPixel(image, 75, 25, Black);

    // What was drawn before stays; only what's new is rendered.
    context.SetFillStyle(boo::dom::DOMString(u8"blue"));
    context.FillRect(50, 0, 50, 50);
    image = fixture.Frame();
    CheckPixel(image, 25, 25, Red);
    CheckPixel(image, 75, 25, Blue);

    // A frame with nothing drawn shows the same.
    image = fixture.Frame();
    CheckPixel(image, 25, 25, Red);
    CheckPixel(image, 75, 25, Blue);

    // Clearing makes the canvas transparent, showing the page beneath.
    context.ClearRect(40, 0, 20, 50);
    image = fixture.Frame();
    CheckPixel(image, 45, 25, Black);
    CheckPixel(image, 55, 25, Black);
    CheckPixel(image, 35, 25, Red);
    CheckPixel(image, 65, 25, Blue);

    // Resetting the context clears it all.
    context.Reset();
    image = fixture.Frame();
    CheckPixel(image, 25, 25, Black);
    CheckPixel(image, 75, 25, Black);
  }

  TEST_CASE("Canvas 2D: the state carries over from one frame to the next", "[UI][GPU][Canvas]")
  {
    Canvas2DFixture fixture;
    auto &context = fixture.Context();

    // A clip and a transform made in one frame apply to what's drawn in the next.
    context.Save();
    context.Rect(0, 0, 20, 20);
    context.Clip();
    context.Translate(10, 0);
    context.SetFillStyle(boo::dom::DOMString(u8"lime"));
    (void)fixture.Frame();

    context.FillRect(0, 0, 100, 50);
    auto image = fixture.Frame();
    CheckPixel(image, 5, 5, Black);
    CheckPixel(image, 15, 15, Lime);
    CheckPixel(image, 25, 5, Black);
    CheckPixel(image, 15, 25, Black);

    // Restoring in a later frame brings back the state saved before.
    context.Restore();
    (void)fixture.Frame();
    context.FillRect(60, 30, 10, 10);
    image = fixture.Frame();
    CheckPixel(image, 65, 35, Black);
    CheckPixel(image, 15, 15, Lime);

    context.SetFillStyle(boo::dom::DOMString(u8"red"));
    context.FillRect(60, 30, 10, 10);
    image = fixture.Frame();
    CheckPixel(image, 65, 35, Red);

    // Line styles, too.
    context.SetLineWidth(10);
    context.SetStrokeStyle(boo::dom::DOMString(u8"blue"));
    (void)fixture.Frame();
    context.BeginPath();
    context.MoveTo(80, 0);
    context.LineTo(80, 50);
    context.Stroke();
    image = fixture.Frame();
    CheckPixel(image, 76, 25, Blue);
    CheckPixel(image, 84, 25, Blue);
    CheckPixel(image, 88, 25, Black);
  }

  TEST_CASE("Canvas 2D: painted as the page is composited", "[UI][GPU][Canvas]")
  {
    Canvas2DFixture fixture;
    auto &context = fixture.Context();
    context.SetFillStyle(boo::dom::DOMString(u8"red"));
    context.FillRect(0, 0, 50, 50);
    (void)fixture.Frame();
    context.SetFillStyle(boo::dom::DOMString(u8"blue"));
    context.FillRect(25, 10, 50, 20);

    // Painting the page again draws the canvas's texture as an image, with everything drawn so far.
    auto repainted = fixture.Paint().Repainted(fixture.GetView());
    CheckPixel(repainted, 10, 40, Red);
    CheckPixel(repainted, 50, 20, Blue);
    CheckPixel(repainted, 90, 40, Black);
    auto painted = fixture.Frame();
    auto comparison = CompareImages(painted, repainted, 2u);
    CHECK(comparison.PixelsOver == 0u);
  }

  TEST_CASE("Canvas 2D: reads back what's drawn", "[UI][GPU][Canvas]")
  {
    SECTION("with the device given before the first paint")
    {
      Canvas2DFixture fixture;
      fixture.GetView().SetDevice(fixture.Paint().Device());
      auto &context = fixture.Context();
      context.SetFillStyle(boo::dom::DOMString(u8"red"));
      context.FillRect(0, 0, 50, 50);
      context.SetFillStyle(boo::dom::DOMString(u8"rgba(0, 0, 255, 0.5)"));
      context.FillRect(50, 0, 50, 50);

      CheckRead(ReadPixel(context, 25, 25), Red);
      // Unpremultiplied.
      CheckRead(ReadPixel(context, 75, 25), {0, 0, 255, 128});
      // Outside the canvas is transparent black.
      auto read = context.GetImageData(-1, 0, 2, 1, {});
      auto bytes = read.Value()->Data().AsUint8ClampedArray();
      CHECK(bytes->Item(3) == 0);
      CHECK(bytes->Item(4) == 255);
      CHECK(bytes->Item(7) == 255);

      // Drawing after a read is read too, and shown.
      context.SetFillStyle(boo::dom::DOMString(u8"lime"));
      context.FillRect(0, 0, 10, 10);
      CheckRead(ReadPixel(context, 5, 5), Lime);
      auto image = fixture.Frame();
      CheckPixel(image, 5, 5, Lime);
      CheckPixel(image, 25, 25, Red);
    }

    SECTION("without the device, once the view has painted")
    {
      Canvas2DFixture fixture;
      auto &context = fixture.Context();
      context.SetFillStyle(boo::dom::DOMString(u8"red"));
      context.FillRect(0, 0, 50, 50);
      CheckRead(ReadPixel(context, 25, 25), Transparent);
      (void)fixture.Frame();
      CheckRead(ReadPixel(context, 25, 25), Red);
    }
  }

  TEST_CASE("Canvas 2D: puts pixels in order with what's drawn", "[UI][GPU][Canvas]")
  {
    Canvas2DFixture fixture;
    fixture.GetView().SetDevice(fixture.Paint().Device());
    auto &context = fixture.Context();
    context.SetFillStyle(boo::dom::DOMString(u8"red"));
    context.FillRect(0, 0, 100, 50);

    // Green, put whatever the transform, alpha and compositing, then drawn over.
    auto green = context.CreateImageData(10, 10, {});
    auto greenBytes = green.Value()->Data().AsUint8ClampedArray();
    for (size_t pixel = 0; pixel < 100u; ++pixel)
    {
      greenBytes->Set(pixel * 4 + 1, 255);
      greenBytes->Set(pixel * 4 + 3, 255);
    }
    context.Translate(30, 30);
    context.SetGlobalAlpha(0.5);
    context.SetGlobalCompositeOperation(u8"lighter");
    context.PutImageData(*green.Value(), 20, 20);
    context.ResetTransform();
    context.SetGlobalAlpha(1);
    context.SetGlobalCompositeOperation(u8"source-over");
    context.SetFillStyle(boo::dom::DOMString(u8"blue"));
    context.FillRect(25, 25, 10, 10);

    // Half-transparent white replaces what's there rather than blending with it.
    auto white = context.CreateImageData(10, 10, {});
    auto whiteBytes = white.Value()->Data().AsUint8ClampedArray();
    for (size_t channel = 0; channel < 400u; ++channel)
    {
      whiteBytes->Set(channel, channel % 4 == 3 ? 128 : 255);
    }
    context.PutImageData(*white.Value(), 60, 20);
    // Only the dirty rectangle of it is put.
    context.PutImageData(*green.Value(), 80, 20, 0, 0, 5, 5);

    CheckRead(ReadPixel(context, 22, 22), Lime);
    CheckRead(ReadPixel(context, 30, 30), Blue);
    CheckRead(ReadPixel(context, 5, 5), Red);
    CheckRead(ReadPixel(context, 65, 25), {255, 255, 255, 128});
    CheckRead(ReadPixel(context, 82, 22), Lime);
    CheckRead(ReadPixel(context, 87, 27), Red);

    auto image = fixture.Frame();
    CheckPixel(image, 22, 22, Lime);
    CheckPixel(image, 30, 30, Blue);
    CheckPixel(image, 65, 25, {128, 128, 128, 255});
    CheckPixel(image, 87, 27, Red);
  }

  TEST_CASE("Canvas 2D: draws canvases as they were when drawn", "[UI][GPU][Canvas]")
  {
    Canvas2DFixture fixture(u8"<canvas id=source width=20 height=20 style='display: none'></canvas>");
    fixture.GetView().SetDevice(fixture.Paint().Device());
    auto &context = fixture.Context();
    auto source = fixture.Canvas(u8"source").GetContext2d();
    REQUIRE(source);
    auto &sourceCanvas = fixture.Canvas(u8"source");

    source->SetFillStyle(boo::dom::DOMString(u8"red"));
    source->FillRect(0, 0, 20, 20);
    REQUIRE_FALSE(context.DrawImage(ShareRefPtr(&sourceCanvas), 0, 0).HasException());
    // Drawing into the source after doesn't change what was drawn from it.
    source->SetFillStyle(boo::dom::DOMString(u8"blue"));
    source->FillRect(0, 0, 20, 20);
    REQUIRE_FALSE(context.DrawImage(ShareRefPtr(&sourceCanvas), 30, 0).HasException());

    // A canvas drawn into itself, copying the red to the right.
    REQUIRE_FALSE(
      context.DrawImage(ShareRefPtr(&fixture.Canvas()), 0, 0, 20, 20, 60, 0, 20, 20).HasException());

    // A pattern of a canvas.
    auto pattern = context.CreatePattern(ShareRefPtr(&sourceCanvas), u8"repeat");
    REQUIRE_FALSE(pattern.HasException());
    REQUIRE(pattern.Value());
    source->SetFillStyle(boo::dom::DOMString(u8"lime"));
    source->FillRect(0, 0, 20, 20);
    context.SetFillStyle(Ref<boo::html::CanvasPattern>(pattern.Value()));
    context.FillRect(0, 30, 100, 20);

    auto image = fixture.Frame();
    CheckPixel(image, 10, 10, Red);
    CheckPixel(image, 40, 10, Blue);
    CheckPixel(image, 70, 10, Red);
    CheckPixel(image, 90, 10, Black);
    CheckPixel(image, 5, 40, Blue);
    CheckPixel(image, 95, 40, Blue);
    CheckRead(ReadPixel(context, 70, 10), Red);
  }

  TEST_CASE("Canvas 2D: encodes what's drawn", "[UI][GPU][Canvas]")
  {
    Canvas2DFixture fixture;
    fixture.GetView().SetDevice(fixture.Paint().Device());
    auto &context = fixture.Context();
    context.SetFillStyle(boo::dom::DOMString(u8"red"));
    context.FillRect(0, 0, 50, 50);
    context.SetFillStyle(boo::dom::DOMString(u8"rgba(0, 0, 255, 0.5)"));
    context.FillRect(50, 0, 50, 50);

    // The pixels, decoded from @p url, unpremultiplied ARGB.
    auto decode = [](const boo::dom::DOMString &url) -> List<uint32>
    {
      const auto comma = url.find(u8',');
      REQUIRE(comma != boo::dom::DOMString::npos);
      auto bytes = Base64Decode(
        Span<const uint8>(reinterpret_cast<const uint8 *>(url.data()) + comma + 1, url.size() - comma - 1));
      REQUIRE(bytes);
      auto data = boo::SharedBuffer::Create(Span<const uint8>(*bytes));
      auto decoder = boo::ScalableImageDecoder::Create(*data, boo::AlphaOption::NotPremultiplied,
                                                       boo::GammaAndColorProfileOption::Applied);
      REQUIRE(decoder);
      decoder->SetData(*data, true);
      auto image = decoder->CreateFrameImageAtIndex(0);
      REQUIRE(image);
      auto pixels = image->Pixels();
      return {pixels.begin(), pixels.end()};
    };

    auto png = decode(fixture.Canvas().ToDataURL());
    REQUIRE(png.size() == 100u * 50u);
    CHECK(png[25 * 100 + 25] == 0xFF'FF'00'00u);
    // Unpremultiplied, give or take rounding.
    const auto blue = png[25 * 100 + 75];
    CHECK((blue >> 24) == 128u);
    CHECK((blue & 0xFFu) >= 254u);

    // JPEG is drawn over black: the half-transparent blue is half as bright.
    auto jpeg = decode(fixture.Canvas().ToDataURL(u8"image/jpeg"));
    REQUIRE(jpeg.size() == 100u * 50u);
    const auto darkBlue = jpeg[25 * 100 + 75];
    CHECK((darkBlue >> 24) == 255u);
    CHECK(std::abs(static_cast<int32>(darkBlue & 0xFFu) - 128) <= 6);
  }
}
