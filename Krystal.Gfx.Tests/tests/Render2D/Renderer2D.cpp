#include "Krystal.Gfx/Render2D/Renderer2D.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Core/Types/Array.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Span.hpp"
#include "Krystal.Core/Utils/Move.hpp"
#include "Krystal.Gfx.Tests/TestDevice.hpp"
#include "Krystal.Gfx/Render2D/PathRasteriser.hpp"
#include <algorithm>
#include <cmath>
#include <functional>
#include <limits>
#include <numbers>

namespace krys::gfx::tests
{
  namespace
  {
    using Pixel = Array<uint8, 4>;

    constexpr Pixel Red {255, 0, 0, 255};
    constexpr Pixel Green {0, 255, 0, 255};
    constexpr Pixel Blue {0, 0, 255, 255};
    constexpr Pixel White {255, 255, 255, 255};
    constexpr Pixel Clear {0, 0, 0, 0};

    KRYS_NODISCARD bool Near(const Pixel &pixel, const Pixel &expected, int tolerance = 1)
    {
      for (size_t i = 0u; i < 4u; ++i)
      {
        int difference = int(pixel[i]) - int(expected[i]);
        if (difference > tolerance || difference < -tolerance)
        {
          return false;
        }
      }
      return true;
    }

    class Renderer2DFixture
    {
    protected:
      UniquePtr<Device> _device;
      UniquePtr<Renderer2D> _renderer;
      TextureHandle _target {};
      List<TextureHandle> _images {};
      uint32 _width {4u};
      uint32 _height {4u};
      TextureFormat _format {TextureFormat::RGBA8Unorm};

    public:
      Renderer2DFixture() : _device(CreateTestDevice())
      {
        UseRenderer({});
      }

      void UseRenderer(const Renderer2DDesc &desc)
      {
        _renderer = nullptr;
        auto renderer = Renderer2D::Create(*_device, desc);
        if (!renderer)
        {
          FAIL(renderer.error());
        }
        _renderer = krys::move(*renderer);
      }

      ~Renderer2DFixture()
      {
        _renderer = nullptr;
        for (auto image : _images)
        {
          _device->Destroy(image);
        }
        if (_target.IsValid())
        {
          _device->Destroy(_target);
        }
      }

      /// @brief A texture holding `texels`, rows from the top; destroyed when the test ends.
      KRYS_NODISCARD TextureHandle Image(uint32 width, uint32 height, Span<const Pixel> texels,
                                         TextureFormat format = TextureFormat::RGBA8Unorm)
      {
        auto image = _device->CreateTexture(
          {.Format = format, .Width = width, .Height = height, .DebugName = u8"Image"});
        REQUIRE(image.has_value());
        _device->WriteTexture(
          *image, TextureRegion::Whole2D(width, height),
          Span<const byte>(reinterpret_cast<const byte *>(texels.data()), texels.size_bytes()));
        _images.push_back(*image);
        return *image;
      }

      void UseTarget(uint32 width, uint32 height, TextureFormat format = TextureFormat::RGBA8Unorm)
      {
        if (_target.IsValid())
        {
          _device->Destroy(_target);
        }
        auto target = _device->CreateTexture({.Format = format,
                                              .Width = width,
                                              .Height = height,
                                              .Usage = TextureUsage::RenderTarget | TextureUsage::Sampled,
                                              .DebugName = u8"Target"});
        REQUIRE(target.has_value());
        _target = *target;
        _width = width;
        _height = height;
        _format = format;
      }

      /// @brief Draws one 2D frame into the target; it's cleared first when `clear` is set.
      template <typename TFunc>
      void Draw(TFunc &&draw, Maybe<Array<float, 4>> clear = Array<float, 4> {0.f, 0.f, 0.f, 0.f})
      {
        _device->BeginFrame();
        auto &commands = _device->GetCommandBuffer();
        _renderer->Begin(
          commands,
          {.Texture = _target, .Width = _width, .Height = _height, .Format = _format, .ClearColour = clear});
        draw(*_renderer);
        _renderer->End();
        _device->Submit(commands);
        _device->EndFrame();
      }

      KRYS_NODISCARD List<Pixel> Read()
      {
        List<Pixel> pixels(_width * _height);
        _device->ReadTexture(
          _target, TextureRegion::Whole2D(_width, _height),
          Span<byte>(reinterpret_cast<byte *>(pixels.data()), pixels.size() * sizeof(Pixel)));
        return pixels;
      }

      KRYS_NODISCARD Pixel At(const List<Pixel> &pixels, uint32 x, uint32 y) const
      {
        return pixels[y * _width + x];
      }
    };
  }

  TEST_CASE_METHOD(Renderer2DFixture, "Renderer2D: fills rects in pixels from the top left",
                   "[Gfx][Render2D][GPU]")
  {
    UseTarget(4u, 4u);
    Draw([](Renderer2D &renderer) { renderer.FillRect({1.f, 0.f, 2.f, 1.f}, SRGBA<uint8> {255, 0, 0}); });

    auto pixels = Read();
    CHECK(At(pixels, 0u, 0u) == Clear);
    CHECK(At(pixels, 1u, 0u) == Red);
    CHECK(At(pixels, 2u, 0u) == Red);
    CHECK(At(pixels, 3u, 0u) == Clear);
    CHECK(At(pixels, 1u, 1u) == Clear);
  }

  TEST_CASE_METHOD(Renderer2DFixture, "Renderer2D: draws in painter's order", "[Gfx][Render2D][GPU]")
  {
    UseTarget(4u, 4u);
    Draw(
      [](Renderer2D &renderer)
      {
        renderer.FillRect({0.f, 0.f, 4.f, 4.f}, SRGBA<uint8> {0, 255, 0});
        renderer.FillRect({0.f, 0.f, 2.f, 2.f}, SRGBA<uint8> {255, 0, 0});
        // Empty and fully transparent rects draw nothing.
        renderer.FillRect({0.f, 0.f, 0.f, 4.f}, SRGBA<uint8> {0, 0, 255});
        renderer.FillRect({0.f, 0.f, 4.f, 4.f}, SRGBA<uint8> {0, 0, 255, 0});
      });

    auto pixels = Read();
    CHECK(At(pixels, 0u, 0u) == Red);
    CHECK(At(pixels, 1u, 1u) == Red);
    CHECK(At(pixels, 2u, 0u) == Green);
    CHECK(At(pixels, 3u, 3u) == Green);
  }

  TEST_CASE_METHOD(Renderer2DFixture, "Renderer2D: blends with straight alpha colours",
                   "[Gfx][Render2D][GPU]")
  {
    // Blending works on the encoded values: half red over blue.
    UseTarget(2u, 2u);
    Draw([](Renderer2D &renderer) { renderer.FillRect({0.f, 0.f, 2.f, 2.f}, SRGBA<uint8> {255, 0, 0, 128}); },
         Array<float, 4> {0.f, 0.f, 1.f, 1.f});
    CHECK(Near(Read()[0], {128, 0, 127, 255}));

    // Half white over black is encoded half grey, as browsers blend, not half the light (188).
    Draw([](Renderer2D &renderer)
         { renderer.FillRect({0.f, 0.f, 2.f, 2.f}, SRGBA<uint8> {255, 255, 255, 128}); },
         Array<float, 4> {0.f, 0.f, 0.f, 1.f});
    CHECK(Near(Read()[0], {128, 128, 128, 255}));

    // Opaque colours come back as they went in.
    Draw([](Renderer2D &renderer) { renderer.FillRect({0.f, 0.f, 2.f, 2.f}, SRGBA<uint8> {200, 100, 50}); });
    CHECK(Near(Read()[0], {200, 100, 50, 255}));
  }

  TEST_CASE_METHOD(Renderer2DFixture, "Renderer2D: draws into part of a larger texture unstretched",
                   "[Gfx][Render2D][GPU]")
  {
    // As pooled textures are, rounded up past what's drawn: pixels map one to one, and clips line up with
    // the shapes they clip.
    UseTarget(8u, 8u);
    _width = 4u;
    _height = 4u;
    Draw(
      [](Renderer2D &renderer)
      {
        renderer.FillRect({1.f, 1.f, 2.f, 2.f}, SRGBA<uint8> {255, 0, 0});
        renderer.ClipRoundedRect({{0.f, 3.f, 4.f, 1.f}, {}});
        renderer.FillRect({0.f, 0.f, 4.f, 4.f}, SRGBA<uint8> {0, 0, 255});
      });
    _width = 8u;
    _height = 8u;
    auto pixels = Read();
    CHECK(At(pixels, 0u, 0u) == Clear);
    CHECK(At(pixels, 1u, 1u) == Red);
    CHECK(At(pixels, 2u, 2u) == Red);
    CHECK(At(pixels, 3u, 2u) == Clear);
    CHECK(At(pixels, 1u, 3u) == Blue);
    CHECK(At(pixels, 5u, 5u) == Clear);
  }

  TEST_CASE_METHOD(Renderer2DFixture, "Renderer2D: keeps the target's contents without a clear colour",
                   "[Gfx][Render2D][GPU]")
  {
    UseTarget(4u, 4u);
    Draw([](Renderer2D &renderer) { renderer.FillRect({0.f, 0.f, 4.f, 4.f}, SRGBA<uint8> {0, 255, 0}); });
    Draw([](Renderer2D &renderer) { renderer.FillRect({0.f, 0.f, 1.f, 1.f}, SRGBA<uint8> {255, 0, 0}); },
         Maybe<Array<float, 4>> {});

    auto pixels = Read();
    CHECK(At(pixels, 0u, 0u) == Red);
    CHECK(At(pixels, 3u, 3u) == Green);
  }

  TEST_CASE_METHOD(Renderer2DFixture, "Renderer2D: draws many rects in one frame", "[Gfx][Render2D][GPU]")
  {
    UseTarget(16u, 16u);
    Draw(
      [](Renderer2D &renderer)
      {
        for (uint32 y = 0u; y < 16u; ++y)
        {
          for (uint32 x = 0u; x < 16u; ++x)
          {
            const bool red = (x + y) % 2u == 0u;
            renderer.FillRect({float(x), float(y), 1.f, 1.f},
                              red ? SRGBA<uint8> {255, 0, 0} : SRGBA<uint8> {0, 255, 0});
          }
        }
      });

    auto pixels = Read();
    CHECK(At(pixels, 0u, 0u) == Red);
    CHECK(At(pixels, 1u, 0u) == Green);
    CHECK(At(pixels, 15u, 15u) == Red);
    CHECK(At(pixels, 14u, 15u) == Green);
  }

  TEST_CASE_METHOD(Renderer2DFixture, "Renderer2D: antialiases edges between pixels", "[Gfx][Render2D][GPU]")
  {
    // The left edge at x = 0.5 and the right edge at x = 1.5 each cover half of a pixel.
    UseTarget(4u, 4u);
    Draw([](Renderer2D &renderer) { renderer.FillRect({0.5f, 0.f, 1.f, 4.f}, SRGBA<uint8> {255, 0, 0}); });

    auto pixels = Read();
    CHECK(Near(At(pixels, 0u, 0u), {128, 0, 0, 128}, 2));
    CHECK(Near(At(pixels, 1u, 2u), {128, 0, 0, 128}, 2));
    CHECK(At(pixels, 2u, 0u) == Clear);

    // A corner halfway across a pixel and halfway down it covers a quarter of it.
    Draw([](Renderer2D &renderer) { renderer.FillRect({0.5f, 0.5f, 2.f, 2.f}, SRGBA<uint8> {255, 0, 0}); });
    pixels = Read();
    CHECK(Near(At(pixels, 0u, 0u), {64, 0, 0, 64}, 2));
    CHECK(Near(At(pixels, 1u, 0u), {128, 0, 0, 128}, 2));
    CHECK(At(pixels, 1u, 1u) == Red);
  }

  TEST_CASE_METHOD(Renderer2DFixture, "Renderer2D: fills rounded rects", "[Gfx][Render2D][GPU]")
  {
    UseTarget(8u, 8u);
    Draw(
      [](Renderer2D &renderer)
      {
        renderer.FillRoundedRect({{0.f, 0.f, 8.f, 8.f}, CornerRadii2D::Uniform(4.f)},
                                 SRGBA<uint8> {255, 0, 0});
      });

    auto circle = Read();
    // The corner pixel is outside the curve; the pixel below it is about a fifth covered.
    CHECK(At(circle, 0u, 0u) == Clear);
    CHECK(Near(At(circle, 0u, 1u), {51, 0, 0, 51}, 3));
    CHECK(At(circle, 4u, 4u) == Red);
    CHECK(At(circle, 7u, 7u) == Clear);

    SECTION("Radii that don't fit are scaled down together")
    {
      Draw(
        [](Renderer2D &renderer)
        {
          renderer.FillRoundedRect({{0.f, 0.f, 8.f, 8.f}, CornerRadii2D::Uniform(100.f)},
                                   SRGBA<uint8> {255, 0, 0});
        });
      CHECK(Read() == circle);
    }

    SECTION("Each corner has its own radius; negative radii are square")
    {
      Draw(
        [](Renderer2D &renderer)
        {
          renderer.FillRoundedRect(
            {{0.f, 0.f, 8.f, 8.f}, {.TopLeft = 4.f, .TopRight = 0.f, .BottomRight = -5.f, .BottomLeft = 0.f}},
            SRGBA<uint8> {255, 0, 0});
        });

      auto pixels = Read();
      CHECK(At(pixels, 0u, 0u) == Clear);
      CHECK(At(pixels, 7u, 0u) == Red);
      CHECK(At(pixels, 7u, 7u) == Red);
      CHECK(At(pixels, 0u, 7u) == Red);
    }
  }

  TEST_CASE_METHOD(Renderer2DFixture, "Renderer2D: draws in the current transform's coordinates",
                   "[Gfx][Render2D][GPU]")
  {
    UseTarget(4u, 4u);

    SECTION("Translations and scales")
    {
      Draw(
        [](Renderer2D &renderer)
        {
          renderer.Save();
          renderer.Translate(2.f, 1.f);
          renderer.FillRect({0.f, 0.f, 1.f, 1.f}, SRGBA<uint8> {255, 0, 0});
          renderer.Restore();

          renderer.Scale(2.f, 2.f);
          renderer.FillRect({0.f, 1.f, 1.f, 1.f}, SRGBA<uint8> {0, 255, 0});
        });

      auto pixels = Read();
      CHECK(At(pixels, 2u, 1u) == Red);
      CHECK(At(pixels, 1u, 1u) == Clear);
      CHECK(At(pixels, 3u, 1u) == Clear);
      // Restore undid the translation, so the scaled rect covers (0, 2) to (2, 4).
      CHECK(At(pixels, 0u, 2u) == Green);
      CHECK(At(pixels, 1u, 3u) == Green);
      CHECK(At(pixels, 2u, 2u) == Clear);
    }

    SECTION("Quarter turns")
    {
      // Local (x, y) lands on (4 - y, x): the 2-by-1 rect becomes a 1-by-2 column at the right edge.
      Draw(
        [](Renderer2D &renderer)
        {
          renderer.Translate(4.f, 0.f);
          renderer.Rotate(std::numbers::pi_v<float> / 2.f);
          renderer.FillRect({0.f, 0.f, 2.f, 1.f}, SRGBA<uint8> {255, 0, 0});
        });

      auto pixels = Read();
      CHECK(Near(At(pixels, 3u, 0u), Red, 2));
      CHECK(Near(At(pixels, 3u, 1u), Red, 2));
      CHECK(Near(At(pixels, 2u, 0u), Clear, 2));
      CHECK(Near(At(pixels, 3u, 2u), Clear, 2));
    }

    // Each frame starts untransformed.
    Draw([](Renderer2D &renderer) { CHECK(renderer.GetTransform() == Transform2D::Identity()); },
         Maybe<Array<float, 4>> {});
  }

  TEST_CASE_METHOD(Renderer2DFixture, "Renderer2D: antialiases one target pixel under transforms",
                   "[Gfx][Render2D][GPU]")
  {
    SECTION("Scaled")
    {
      // The left edge lands at x = 0.5 and the right at x = 4.5; scaling doesn't widen the band.
      UseTarget(6u, 1u);
      Draw(
        [](Renderer2D &renderer)
        {
          renderer.Scale(4.f, 1.f);
          renderer.FillRect({0.125f, 0.f, 1.f, 1.f}, SRGBA<uint8> {255, 0, 0});
        });

      auto pixels = Read();
      CHECK(Near(pixels[0], {128, 0, 0, 128}, 2));
      CHECK(pixels[1] == Red);
      CHECK(pixels[3] == Red);
      CHECK(Near(pixels[4], {128, 0, 0, 128}, 2));
      CHECK(pixels[5] == Clear);
    }

    SECTION("Rotated")
    {
      // A 4-by-4 square turned 45 degrees about the target's centre: a diamond whose edge passes just inside
      // the centre of pixel (4, 1), covering about 38% of it.
      UseTarget(8u, 8u);
      Draw(
        [](Renderer2D &renderer)
        {
          renderer.Translate(4.f, 4.f);
          renderer.Rotate(std::numbers::pi_v<float> / 4.f);
          renderer.FillRect({-2.f, -2.f, 4.f, 4.f}, SRGBA<uint8> {255, 0, 0});
        });

      auto pixels = Read();
      CHECK(At(pixels, 4u, 4u) == Red);
      CHECK(At(pixels, 0u, 0u) == Clear);
      CHECK(Near(At(pixels, 4u, 1u), {97, 0, 0, 97}, 6));
    }
  }

  TEST_CASE_METHOD(Renderer2DFixture, "Renderer2D: clips to rects", "[Gfx][Render2D][GPU]")
  {
    UseTarget(4u, 4u);

    SECTION("Clip edges between pixels are antialiased")
    {
      // The clip from x = 0.6 to 2.6 covers 40% of pixel 0 and 60% of pixel 2.
      Draw(
        [](Renderer2D &renderer)
        {
          renderer.ClipRect({0.6f, 1.f, 2.f, 2.f});
          renderer.FillRect({0.f, 0.f, 4.f, 4.f}, SRGBA<uint8> {255, 0, 0});
        });

      auto pixels = Read();
      CHECK(Near(At(pixels, 0u, 1u), {102, 0, 0, 102}, 2));
      CHECK(At(pixels, 1u, 1u) == Red);
      CHECK(Near(At(pixels, 2u, 2u), {153, 0, 0, 153}, 2));
      CHECK(At(pixels, 3u, 2u) == Clear);
      CHECK(At(pixels, 1u, 0u) == Clear);
      CHECK(At(pixels, 1u, 3u) == Clear);
    }

    SECTION("Clips intersect, follow the transform and end with Restore")
    {
      Draw(
        [](Renderer2D &renderer)
        {
          renderer.Save();
          renderer.ClipRect({0.f, 0.f, 3.f, 4.f});
          renderer.Save();
          renderer.Translate(1.f, 0.f);
          renderer.ClipRect({0.f, 0.f, 3.f, 1.f});
          renderer.FillRect({-1.f, 0.f, 4.f, 4.f}, SRGBA<uint8> {255, 0, 0});
          renderer.Restore();
          renderer.Restore();
          // Unclipped again, and drawn after the clipped rect.
          renderer.FillRect({3.f, 0.f, 1.f, 1.f}, SRGBA<uint8> {0, 255, 0});
        });

      auto pixels = Read();
      CHECK(At(pixels, 0u, 0u) == Clear);
      CHECK(At(pixels, 1u, 0u) == Red);
      CHECK(At(pixels, 2u, 0u) == Red);
      CHECK(At(pixels, 3u, 0u) == Green);
      CHECK(At(pixels, 1u, 1u) == Clear);
    }

    SECTION("Nothing draws inside an empty clip")
    {
      Draw(
        [](Renderer2D &renderer)
        {
          renderer.ClipRect({0.f, 0.f, 2.f, 2.f});
          renderer.ClipRect({2.f, 2.f, 2.f, 2.f});
          renderer.FillRect({0.f, 0.f, 4.f, 4.f}, SRGBA<uint8> {255, 0, 0});
        });

      for (const auto &pixel : Read())
      {
        CHECK(pixel == Clear);
      }
    }
  }

  TEST_CASE_METHOD(Renderer2DFixture, "Renderer2D: draws images scaled to fit", "[Gfx][Render2D][GPU]")
  {
    UseTarget(4u, 4u);
    constexpr Array<Pixel, 4> texels {Red, Green, Blue, White};
    auto image = Image(2u, 2u, texels);

    SECTION("Nearest filtering")
    {
      Draw([&](Renderer2D &renderer)
           { renderer.DrawImage(image, {0.f, 0.f, 4.f, 4.f}, {.Filter = SamplerFilter::Nearest}); });

      auto pixels = Read();
      CHECK(At(pixels, 0u, 0u) == Red);
      CHECK(At(pixels, 1u, 1u) == Red);
      CHECK(At(pixels, 3u, 0u) == Green);
      CHECK(At(pixels, 0u, 3u) == Blue);
      CHECK(At(pixels, 3u, 3u) == White);
    }

    SECTION("Linear filtering")
    {
      // Corner pixels sit on texel centres; inner pixels blend their neighbours.
      Draw([&](Renderer2D &renderer) { renderer.DrawImage(image, {0.f, 0.f, 4.f, 4.f}); });

      auto pixels = Read();
      CHECK(At(pixels, 0u, 0u) == Red);
      CHECK(At(pixels, 3u, 3u) == White);
      const auto inner = At(pixels, 1u, 1u);
      CHECK(inner[0] > 128u);
      CHECK(inner[1] > 0u);
      CHECK(inner[2] > 0u);
    }
  }

  TEST_CASE_METHOD(Renderer2DFixture, "Renderer2D: samples only the source rect", "[Gfx][Render2D][GPU]")
  {
    // Linear filtering at the edges of the green texel would blend in its red neighbour without the clamp.
    UseTarget(4u, 4u);
    constexpr Array<Pixel, 2> texels {Red, Green};
    auto image = Image(2u, 1u, texels);
    Draw([&](Renderer2D &renderer)
         { renderer.DrawImage(image, {1.f, 0.f, 1.f, 1.f}, {0.f, 0.f, 4.f, 4.f}); });

    for (const auto &pixel : Read())
    {
      CHECK(pixel == Green);
    }
  }

  TEST_CASE_METHOD(Renderer2DFixture, "Renderer2D: blends images by their alpha and opacity",
                   "[Gfx][Render2D][GPU]")
  {
    UseTarget(2u, 2u);
    constexpr Pixel halfRed {128, 0, 0, 128};

    SECTION("Premultiplied")
    {
      constexpr Array<Pixel, 1> texels {halfRed};
      auto image = Image(1u, 1u, texels);
      Draw([&](Renderer2D &renderer) { renderer.DrawImage(image, {0.f, 0.f, 2.f, 2.f}); });
      CHECK(Near(Read()[0], halfRed));
    }

    SECTION("Straight")
    {
      constexpr Array<Pixel, 1> texels {Pixel {255, 0, 0, 128}};
      auto image = Image(1u, 1u, texels);
      Draw(
        [&](Renderer2D &renderer)
        {
          renderer.DrawImage(image, {0.f, 0.f, 2.f, 2.f}, {.Alpha = AlphaPremultiplication::Unpremultiplied});
        });
      CHECK(Near(Read()[0], halfRed));
    }

    SECTION("Opacity")
    {
      constexpr Array<Pixel, 1> texels {Red};
      auto image = Image(1u, 1u, texels);
      Draw([&](Renderer2D &renderer) { renderer.DrawImage(image, {0.f, 0.f, 2.f, 2.f}, {.Opacity = 0.5f}); });
      CHECK(Near(Read()[0], halfRed));
    }
  }

  TEST_CASE_METHOD(Renderer2DFixture, "Renderer2D: draws images in sRGB formats as stored",
                   "[Gfx][Render2D][GPU]")
  {
    // The GPU decodes sRGB textures; the shader encodes them back, exactly, whatever their alpha.
    UseTarget(2u, 2u);

    SECTION("Opaque")
    {
      constexpr Array<Pixel, 1> texels {Pixel {188, 100, 20, 255}};
      auto image = Image(1u, 1u, texels, TextureFormat::RGBA8Srgb);
      Draw([&](Renderer2D &renderer) { renderer.DrawImage(image, {0.f, 0.f, 2.f, 2.f}); });
      CHECK(Near(Read()[0], texels[0]));
    }

    SECTION("Premultiplied")
    {
      constexpr Array<Pixel, 1> texels {Pixel {100, 50, 20, 128}};
      auto image = Image(1u, 1u, texels, TextureFormat::RGBA8Srgb);
      Draw([&](Renderer2D &renderer) { renderer.DrawImage(image, {0.f, 0.f, 2.f, 2.f}); });
      CHECK(Near(Read()[0], texels[0]));
    }

    SECTION("Straight")
    {
      constexpr Array<Pixel, 1> texels {Pixel {200, 100, 40, 128}};
      auto image = Image(1u, 1u, texels, TextureFormat::RGBA8Srgb);
      Draw(
        [&](Renderer2D &renderer)
        {
          renderer.DrawImage(image, {0.f, 0.f, 2.f, 2.f}, {.Alpha = AlphaPremultiplication::Unpremultiplied});
        });
      CHECK(Near(Read()[0], {100, 50, 20, 128}));
    }
  }

  TEST_CASE_METHOD(Renderer2DFixture, "Renderer2D: interleaves images and rects in painter's order",
                   "[Gfx][Render2D][GPU]")
  {
    UseTarget(4u, 4u);
    constexpr Array<Pixel, 1> greenTexels {Green};
    constexpr Array<Pixel, 1> blueTexels {Blue};
    auto green = Image(1u, 1u, greenTexels);
    auto blue = Image(1u, 1u, blueTexels);

    Draw(
      [&](Renderer2D &renderer)
      {
        renderer.FillRect({0.f, 0.f, 4.f, 4.f}, SRGBA<uint8> {255, 0, 0});
        renderer.DrawImage(green, {1.f, 1.f, 2.f, 2.f});
        renderer.FillRect({2.f, 2.f, 2.f, 2.f}, SRGBA<uint8> {0, 0, 255});
        // Switching between textures keeps the order too.
        renderer.DrawImage(green, {0.f, 3.f, 1.f, 1.f});
        renderer.DrawImage(blue, {0.f, 3.f, 1.f, 1.f});
        renderer.DrawImage(green, {3.f, 0.f, 1.f, 1.f});
        // Images follow the transform.
        renderer.Translate(1.f, -1.f);
        renderer.DrawImage(blue, {0.f, 1.f, 1.f, 1.f});
      });

    auto pixels = Read();
    CHECK(At(pixels, 0u, 0u) == Red);
    CHECK(At(pixels, 1u, 1u) == Green);
    CHECK(At(pixels, 2u, 1u) == Green);
    CHECK(At(pixels, 2u, 2u) == Blue);
    CHECK(At(pixels, 3u, 3u) == Blue);
    CHECK(At(pixels, 0u, 3u) == Blue);
    CHECK(At(pixels, 3u, 0u) == Green);
    CHECK(At(pixels, 1u, 0u) == Blue);
  }

  namespace
  {
    /// @brief A right triangle with its right angle at (x, 0) and legs of `size` along the axes.
    KRYS_NODISCARD Path2D Triangle(float x, float size = 8.f)
    {
      Path2D path;
      path.MoveTo({x, 0.f});
      path.LineTo({x + size, 0.f});
      path.LineTo({x, size});
      path.Close();
      return path;
    }
  }

  TEST_CASE_METHOD(Renderer2DFixture, "Renderer2D: fills paths with the rasteriser's coverage",
                   "[Gfx][Render2D][GPU]")
  {
    UseTarget(8u, 8u);
    const auto triangle = Triangle(0.f);
    Draw([&](Renderer2D &renderer)
         { renderer.FillPath(triangle, FillRule::NonZero, SRGBA<uint8> {255, 0, 0}); });

    PathRasteriser rasteriser;
    const auto expected =
      rasteriser.Rasterise(triangle, {}, FillRule::NonZero, {.X = 0, .Y = 0, .Width = 8u, .Height = 8u});
    auto pixels = Read();
    for (int32 y = 0; y < 8; ++y)
    {
      for (int32 x = 0; x < 8; ++x)
      {
        const auto coverage = expected.At(x, y);
        CHECK(Near(At(pixels, static_cast<uint32>(x), static_cast<uint32>(y)), {coverage, 0, 0, coverage}));
      }
    }
  }

  TEST_CASE_METHOD(Renderer2DFixture, "Renderer2D: fills paths under the drawing state",
                   "[Gfx][Render2D][GPU]")
  {
    UseTarget(8u, 8u);

    SECTION("Fill rules")
    {
      Path2D nested;
      nested.AddRect({0.f, 0.f, 8.f, 8.f});
      nested.AddRect({2.f, 2.f, 4.f, 4.f});
      Draw([&](Renderer2D &renderer)
           { renderer.FillPath(nested, FillRule::EvenOdd, SRGBA<uint8> {255, 0, 0}); });

      auto pixels = Read();
      CHECK(At(pixels, 0u, 0u) == Red);
      CHECK(At(pixels, 4u, 4u) == Clear);
    }

    SECTION("Transforms, including fractions of a pixel")
    {
      Path2D square;
      square.AddRect({0.f, 0.f, 1.f, 1.f});
      Draw(
        [&](Renderer2D &renderer)
        {
          renderer.Save();
          renderer.Translate(2.f, 1.f);
          renderer.Scale(2.f, 2.f);
          renderer.FillPath(square, FillRule::NonZero, SRGBA<uint8> {255, 0, 0});
          renderer.Restore();
          renderer.Translate(0.5f, 6.f);
          renderer.FillPath(square, FillRule::NonZero, SRGBA<uint8> {255, 0, 0});
        });

      auto pixels = Read();
      CHECK(At(pixels, 2u, 1u) == Red);
      CHECK(At(pixels, 3u, 2u) == Red);
      CHECK(At(pixels, 4u, 1u) == Clear);
      CHECK(Near(At(pixels, 0u, 6u), {128, 0, 0, 128}));
      CHECK(Near(At(pixels, 1u, 6u), {128, 0, 0, 128}));
    }

    SECTION("Clips, in painter's order with rects")
    {
      Path2D square;
      square.AddRect({0.f, 0.f, 8.f, 8.f});
      Draw(
        [&](Renderer2D &renderer)
        {
          renderer.FillRect({0.f, 0.f, 8.f, 8.f}, SRGBA<uint8> {0, 0, 255});
          renderer.Save();
          renderer.ClipRect({0.f, 0.f, 2.f, 8.f});
          renderer.FillPath(square, FillRule::NonZero, SRGBA<uint8> {255, 0, 0});
          renderer.Restore();
          renderer.FillRect({0.f, 0.f, 1.f, 1.f}, SRGBA<uint8> {0, 255, 0});
        });

      auto pixels = Read();
      CHECK(At(pixels, 0u, 0u) == Green);
      CHECK(At(pixels, 1u, 4u) == Red);
      CHECK(At(pixels, 2u, 4u) == Blue);
    }
  }

  TEST_CASE_METHOD(Renderer2DFixture, "Renderer2D: caches path masks", "[Gfx][Render2D][GPU]")
  {
    UseTarget(16u, 8u);
    const auto triangle = Triangle(0.f);
    const SRGBA<uint8> red {255, 0, 0};

    // The same path again, or moved by whole pixels, reuses the mask.
    Draw(
      [&](Renderer2D &renderer)
      {
        renderer.FillPath(triangle, FillRule::NonZero, red);
        renderer.Translate(8.f, 0.f);
        renderer.FillPath(triangle, FillRule::NonZero, red);
      });
    CHECK(_renderer->GetStats().MasksRasterised == 1u);
    CHECK(_renderer->GetStats().MaskCacheHits == 1u);
    auto pixels = Read();
    CHECK(At(pixels, 1u, 1u) == Red);
    CHECK(At(pixels, 9u, 1u) == Red);

    // Masks outlive the frame.
    Draw([&](Renderer2D &renderer) { renderer.FillPath(triangle, FillRule::NonZero, red); });
    CHECK(_renderer->GetStats().MasksRasterised == 0u);
    CHECK(_renderer->GetStats().MaskCacheHits == 1u);

    // Fractions of a pixel, another fill rule or another path each need a mask of their own.
    Draw(
      [&](Renderer2D &renderer)
      {
        renderer.FillPath(triangle, FillRule::EvenOdd, red);
        renderer.FillPath(Triangle(0.f, 7.f), FillRule::NonZero, red);
        renderer.Translate(0.5f, 0.f);
        renderer.FillPath(triangle, FillRule::NonZero, red);
      });
    CHECK(_renderer->GetStats().MasksRasterised == 3u);
    CHECK(_renderer->GetStats().MaskCacheHits == 0u);
  }

  TEST_CASE_METHOD(Renderer2DFixture, "Renderer2D: makes room for masks", "[Gfx][Render2D][GPU]")
  {
    // One 16-by-16 page holds four of the 8-by-8 triangles.
    UseRenderer({.Masks = {.PageSize = 16u, .MaxPages = 1u, .MaxPackedSize = 8u}});
    UseTarget(48u, 16u);
    const SRGBA<uint8> red {255, 0, 0};

    SECTION("A full page in use this frame sends masks to textures of their own")
    {
      Draw(
        [&](Renderer2D &renderer)
        {
          for (int i = 0; i < 6; ++i)
          {
            renderer.FillPath(Triangle(static_cast<float>(i) * 8.f), FillRule::NonZero, red);
          }
        });
      CHECK(_renderer->GetStats().MasksRasterised == 6u);
      auto pixels = Read();
      for (uint32 i = 0u; i < 6u; ++i)
      {
        CHECK(At(pixels, i * 8u + 1u, 1u) == Red);
      }
    }

    SECTION("A page unused this frame is cleared for new masks")
    {
      Draw(
        [&](Renderer2D &renderer)
        {
          for (int i = 0; i < 4; ++i)
          {
            renderer.FillPath(Triangle(static_cast<float>(i) * 8.f), FillRule::NonZero, red);
          }
        });
      // The fifth triangle clears the page, so the first has to be rasterised again.
      Draw([&](Renderer2D &renderer) { renderer.FillPath(Triangle(32.f), FillRule::NonZero, red); });
      Draw([&](Renderer2D &renderer) { renderer.FillPath(Triangle(0.f), FillRule::NonZero, red); });
      CHECK(_renderer->GetStats().MasksRasterised == 1u);
      CHECK(At(Read(), 1u, 1u) == Red);
    }

    SECTION("Masks larger than a page entry get textures of their own")
    {
      Draw([&](Renderer2D &renderer) { renderer.FillPath(Triangle(0.f, 16.f), FillRule::NonZero, red); });
      auto pixels = Read();
      CHECK(At(pixels, 1u, 1u) == Red);
      CHECK(At(pixels, 1u, 13u) == Red);
      CHECK(At(pixels, 15u, 15u) == Clear);
    }
  }

  TEST_CASE_METHOD(Renderer2DFixture, "Renderer2D: fills rects with rounded holes", "[Gfx][Render2D][GPU]")
  {
    const SRGBA<uint8> red {255, 0, 0};

    SECTION("Square corners on whole pixels are exact")
    {
      UseTarget(6u, 6u);
      Draw([&](Renderer2D &renderer)
           { renderer.FillRectWithRoundedHole({0.f, 0.f, 6.f, 6.f}, {{1.f, 1.f, 4.f, 4.f}, {}}, red); });

      auto pixels = Read();
      CHECK(At(pixels, 0u, 0u) == Red);
      CHECK(At(pixels, 0u, 3u) == Red);
      CHECK(At(pixels, 5u, 5u) == Red);
      CHECK(At(pixels, 1u, 1u) == Clear);
      CHECK(At(pixels, 4u, 4u) == Clear);
    }

    SECTION("Rounded corners are antialiased")
    {
      // A 6-by-6 hole with radius 3 is a circle of radius 3 centred on (4, 4).
      UseTarget(8u, 8u);
      Draw(
        [&](Renderer2D &renderer)
        {
          renderer.FillRectWithRoundedHole({0.f, 0.f, 8.f, 8.f},
                                           {{1.f, 1.f, 6.f, 6.f}, CornerRadii2D::Uniform(3.f)}, red);
        });

      auto circle = Read();
      CHECK(At(circle, 0u, 0u) == Red);
      CHECK(At(circle, 1u, 1u) == Red);
      CHECK(At(circle, 4u, 4u) == Clear);
      // The circle passes just outside the centre of (1, 2) and well outside the centre of (4, 1).
      CHECK(Near(At(circle, 1u, 2u), {106, 0, 0, 106}, 2));
      CHECK(Near(At(circle, 4u, 1u), {13, 0, 0, 13}, 2));

      // Radii that don't fit are scaled down together.
      Draw(
        [&](Renderer2D &renderer)
        {
          renderer.FillRectWithRoundedHole({0.f, 0.f, 8.f, 8.f},
                                           {{1.f, 1.f, 6.f, 6.f}, CornerRadii2D::Uniform(100.f)}, red);
        });
      CHECK(Read() == circle);
    }

    SECTION("Holes follow the transform")
    {
      UseTarget(6u, 6u);
      Draw(
        [&](Renderer2D &renderer)
        {
          renderer.Scale(2.f, 2.f);
          renderer.FillRectWithRoundedHole({0.f, 0.f, 3.f, 3.f}, {{1.f, 1.f, 1.f, 1.f}, {}}, red);
        });

      auto pixels = Read();
      CHECK(At(pixels, 1u, 1u) == Red);
      CHECK(At(pixels, 2u, 2u) == Clear);
      CHECK(At(pixels, 3u, 3u) == Clear);
      CHECK(At(pixels, 4u, 4u) == Red);
    }

    SECTION("An empty hole fills the whole rect")
    {
      UseTarget(4u, 4u);
      Draw([&](Renderer2D &renderer)
           { renderer.FillRectWithRoundedHole({0.f, 0.f, 4.f, 4.f}, {{2.f, 2.f, 0.f, 0.f}, {}}, red); });
      for (const auto &pixel : Read())
      {
        CHECK(pixel == Red);
      }
    }
  }

  TEST_CASE_METHOD(Renderer2DFixture, "Renderer2D: strokes paths", "[Gfx][Render2D][GPU]")
  {
    // A 2-wide stroke round the square from 2 to 10 covers 1 to 11, less the inside from 3 to 9.
    UseTarget(12u, 12u);
    Path2D square;
    square.AddRect({2.f, 2.f, 8.f, 8.f});
    Draw([&](Renderer2D &renderer)
         { renderer.StrokePath(square, {.Width = 2.f}, SRGBA<uint8> {255, 0, 0}); });

    auto pixels = Read();
    CHECK(At(pixels, 0u, 0u) == Clear);
    CHECK(At(pixels, 1u, 1u) == Red);
    CHECK(At(pixels, 2u, 5u) == Red);
    CHECK(At(pixels, 10u, 10u) == Red);
    CHECK(At(pixels, 3u, 3u) == Clear);
    CHECK(At(pixels, 5u, 5u) == Clear);
  }

  TEST_CASE_METHOD(Renderer2DFixture, "Renderer2D: strokes straight lines analytically",
                   "[Gfx][Render2D][GPU]")
  {
    const SRGBA<uint8> red {255, 0, 0};

    SECTION("Butt caps end at the end points")
    {
      UseTarget(8u, 8u);
      Draw([&](Renderer2D &renderer) { renderer.StrokeLine({1.f, 4.f}, {7.f, 4.f}, {.Width = 2.f}, red); });

      auto pixels = Read();
      CHECK(At(pixels, 1u, 3u) == Red);
      CHECK(At(pixels, 6u, 4u) == Red);
      CHECK(At(pixels, 0u, 3u) == Clear);
      CHECK(At(pixels, 7u, 3u) == Clear);
      CHECK(At(pixels, 3u, 2u) == Clear);
      CHECK(At(pixels, 3u, 5u) == Clear);
    }

    SECTION("Hairlines and transforms")
    {
      UseTarget(8u, 8u);
      Draw(
        [&](Renderer2D &renderer)
        {
          renderer.StrokeLine({0.f, 4.5f}, {8.f, 4.5f}, {.Width = 0.f}, red);
          renderer.Scale(2.f, 2.f);
          renderer.StrokeLine({0.f, 1.f}, {4.f, 1.f}, {.Width = 1.f}, red);
        });

      auto pixels = Read();
      CHECK(At(pixels, 0u, 4u) == Red);
      CHECK(At(pixels, 7u, 4u) == Red);
      CHECK(At(pixels, 3u, 3u) == Clear);
      CHECK(At(pixels, 3u, 5u) == Clear);
      CHECK(At(pixels, 0u, 1u) == Red);
      CHECK(At(pixels, 7u, 2u) == Red);
      CHECK(At(pixels, 3u, 0u) == Clear);
    }

    SECTION("Dots")
    {
      UseTarget(12u, 8u);
      Draw(
        [&](Renderer2D &renderer)
        {
          renderer.StrokeLine({1.f, 4.f}, {9.f, 4.f},
                              {.Width = 2.f, .Cap = LineCap::Round, .Dashes = {0.f, 4.f}}, red);
        });

      // Circles at x = 1, 5 and 9, and nothing between them.
      auto pixels = Read();
      CHECK(At(pixels, 1u, 4u)[3] > 128u);
      CHECK(At(pixels, 5u, 3u)[3] > 128u);
      CHECK(At(pixels, 9u, 4u)[3] > 128u);
      CHECK(At(pixels, 3u, 4u) == Clear);
      CHECK(At(pixels, 7u, 3u) == Clear);
      CHECK(At(pixels, 11u, 4u) == Clear);
    }
  }

  TEST_CASE_METHOD(Renderer2DFixture, "Renderer2D: strokes lines and paths alike", "[Gfx][Render2D][GPU]")
  {
    // The analytic line and the stroker's outline should agree, up to how each antialiases: on curves and
    // diagonals, the shader's ramp over the distance differs from the rasteriser's area by up to about 0.1.
    struct Case
    {
      Vec2f From;
      Vec2f To;
      StrokeStyle Style;
      int Tolerance;
    };
    const Case cases[] = {
      {{2.f, 8.f}, {14.f, 8.f}, {.Width = 3.f, .Cap = LineCap::Butt}, 2},
      {{2.f, 8.f}, {14.f, 8.f}, {.Width = 3.f, .Cap = LineCap::Square}, 2},
      {{3.f, 8.f}, {13.f, 8.f}, {.Width = 4.f, .Cap = LineCap::Round}, 24},
      {{1.f, 8.f}, {15.f, 8.f}, {.Width = 2.f, .Dashes = {3.f, 2.f}, .DashOffset = 1.f}, 2},
      {{2.f, 8.f}, {14.f, 8.f}, {.Width = 2.f, .Cap = LineCap::Round, .Dashes = {0.f, 3.f}}, 24},
      {{2.f, 3.f}, {13.f, 12.f}, {.Width = 3.f, .Cap = LineCap::Round}, 28},
    };

    UseTarget(16u, 16u);
    const SRGBA<uint8> red {255, 0, 0};
    for (const auto &item : cases)
    {
      Draw([&](Renderer2D &renderer) { renderer.StrokeLine(item.From, item.To, item.Style, red); });
      auto line = Read();

      Path2D path;
      path.MoveTo(item.From);
      path.LineTo(item.To);
      Draw([&](Renderer2D &renderer) { renderer.StrokePath(path, item.Style, red); });
      auto stroked = Read();

      for (size_t i = 0u; i < line.size(); ++i)
      {
        if (!Near(line[i], stroked[i], item.Tolerance))
        {
          FAIL_CHECK("Pixel " << i << " differs: " << int(line[i][3]) << " against " << int(stroked[i][3]));
        }
      }
    }
  }

  namespace
  {
    /// @brief Whether two readbacks match pixel by pixel within `tolerance`.
    KRYS_NODISCARD bool SameImage(const List<Pixel> &a, const List<Pixel> &b, int tolerance = 1)
    {
      if (a.size() != b.size())
      {
        return false;
      }
      for (size_t i = 0u; i < a.size(); ++i)
      {
        if (!Near(a[i], b[i], tolerance))
        {
          return false;
        }
      }
      return true;
    }
  }

  TEST_CASE_METHOD(Renderer2DFixture, "Renderer2D: clips to shapes like it fills them",
                   "[Gfx][Render2D][GPU]")
  {
    UseTarget(8u, 8u);
    const SRGBA<uint8> red {255, 0, 0};

    SECTION("Rounded rects")
    {
      Draw(
        [&](Renderer2D &renderer)
        {
          renderer.ClipRoundedRect({{0.f, 0.f, 8.f, 8.f}, CornerRadii2D::Uniform(4.f)});
          renderer.FillRect({0.f, 0.f, 8.f, 8.f}, red);
        });
      auto clipped = Read();
      Draw([&](Renderer2D &renderer)
           { renderer.FillRoundedRect({{0.f, 0.f, 8.f, 8.f}, CornerRadii2D::Uniform(4.f)}, red); });
      CHECK(SameImage(clipped, Read()));
      CHECK(At(clipped, 0u, 0u) == Clear);
    }

    SECTION("Rects under rotations")
    {
      Draw(
        [&](Renderer2D &renderer)
        {
          renderer.Translate(4.f, 4.f);
          renderer.Rotate(std::numbers::pi_v<float> / 4.f);
          renderer.ClipRect({-2.f, -2.f, 4.f, 4.f});
          renderer.SetTransform({});
          renderer.FillRect({0.f, 0.f, 8.f, 8.f}, red);
        });
      auto clipped = Read();
      Draw(
        [&](Renderer2D &renderer)
        {
          renderer.Translate(4.f, 4.f);
          renderer.Rotate(std::numbers::pi_v<float> / 4.f);
          renderer.FillRect({-2.f, -2.f, 4.f, 4.f}, red);
        });
      CHECK(SameImage(clipped, Read()));
      CHECK(At(clipped, 0u, 0u) == Clear);
      CHECK(At(clipped, 4u, 4u) == Red);
    }

    SECTION("Clipping out a rounded rect is a rect with a rounded hole")
    {
      Draw(
        [&](Renderer2D &renderer)
        {
          renderer.ClipOutRoundedRect({{1.f, 1.f, 6.f, 6.f}, CornerRadii2D::Uniform(3.f)});
          renderer.FillRect({0.f, 0.f, 8.f, 8.f}, red);
        });
      auto clipped = Read();
      Draw(
        [&](Renderer2D &renderer)
        {
          renderer.FillRectWithRoundedHole({0.f, 0.f, 8.f, 8.f},
                                           {{1.f, 1.f, 6.f, 6.f}, CornerRadii2D::Uniform(3.f)}, red);
        });
      CHECK(SameImage(clipped, Read()));
    }

    SECTION("Clipping out a rect")
    {
      Draw(
        [&](Renderer2D &renderer)
        {
          renderer.ClipOutRect({2.f, 2.f, 4.f, 4.f});
          renderer.FillRect({0.f, 0.f, 8.f, 8.f}, red);
        });
      auto pixels = Read();
      CHECK(At(pixels, 0u, 0u) == Red);
      CHECK(At(pixels, 1u, 2u) == Red);
      CHECK(At(pixels, 2u, 2u) == Clear);
      CHECK(At(pixels, 5u, 5u) == Clear);
      CHECK(At(pixels, 6u, 6u) == Red);
    }
  }

  TEST_CASE_METHOD(Renderer2DFixture, "Renderer2D: nests shape clips", "[Gfx][Render2D][GPU]")
  {
    UseTarget(8u, 8u);
    constexpr Array<Pixel, 1> greenTexels {Green};
    auto green = Image(1u, 1u, greenTexels);

    Draw(
      [&](Renderer2D &renderer)
      {
        renderer.Save();
        renderer.ClipRoundedRect({{0.f, 0.f, 8.f, 8.f}, CornerRadii2D::Uniform(4.f)});
        renderer.Save();
        renderer.ClipOutRect({3.f, 3.f, 2.f, 2.f});
        renderer.FillRect({0.f, 0.f, 8.f, 8.f}, SRGBA<uint8> {255, 0, 0});
        renderer.Restore();
        // Still inside the rounded clip, so the corner of the image is cut off.
        renderer.DrawImage(green, {0.f, 0.f, 3.f, 3.f});
        renderer.Restore();
        renderer.FillRect({7.f, 7.f, 1.f, 1.f}, SRGBA<uint8> {0, 0, 255});
      });

    auto pixels = Read();
    CHECK(At(pixels, 4u, 1u) == Red);
    CHECK(At(pixels, 3u, 3u) == Clear);
    CHECK(At(pixels, 4u, 4u) == Clear);
    CHECK(At(pixels, 0u, 0u) == Clear);
    CHECK(At(pixels, 2u, 2u) == Green);
    CHECK(At(pixels, 7u, 7u) == Blue);
  }

  TEST_CASE_METHOD(Renderer2DFixture, "Renderer2D: clips paths and lines too", "[Gfx][Render2D][GPU]")
  {
    UseTarget(8u, 8u);
    const SRGBA<uint8> red {255, 0, 0};
    Draw(
      [&](Renderer2D &renderer)
      {
        renderer.ClipOutRect({0.f, 0.f, 8.f, 4.f});
        Path2D square;
        square.AddRect({0.f, 0.f, 4.f, 8.f});
        renderer.FillPath(square, FillRule::NonZero, red);
        renderer.StrokeLine({4.f, 0.f}, {4.f, 8.f}, {.Width = 2.f}, red);
      });

    auto pixels = Read();
    CHECK(At(pixels, 1u, 1u) == Clear);
    CHECK(At(pixels, 1u, 5u) == Red);
    CHECK(At(pixels, 4u, 2u) == Clear);
    CHECK(At(pixels, 4u, 6u) == Red);
  }

  TEST_CASE_METHOD(Renderer2DFixture, "Renderer2D: handles empty shape clips", "[Gfx][Render2D][GPU]")
  {
    UseTarget(4u, 4u);
    const SRGBA<uint8> red {255, 0, 0};

    // Clipping to nothing leaves nothing; clipping nothing out changes nothing.
    Draw(
      [&](Renderer2D &renderer)
      {
        renderer.ClipRoundedRect({{1.f, 1.f, 0.f, 2.f}, CornerRadii2D::Uniform(1.f)});
        renderer.FillRect({0.f, 0.f, 4.f, 4.f}, red);
      });
    for (const auto &pixel : Read())
    {
      CHECK(pixel == Clear);
    }

    Draw(
      [&](Renderer2D &renderer)
      {
        renderer.ClipOutRoundedRect({{1.f, 1.f, 0.f, 2.f}, CornerRadii2D::Uniform(1.f)});
        renderer.FillRect({0.f, 0.f, 4.f, 4.f}, red);
      });
    for (const auto &pixel : Read())
    {
      CHECK(pixel == Red);
    }
  }

  TEST_CASE_METHOD(Renderer2DFixture, "Renderer2D: clips to paths through their masks",
                   "[Gfx][Render2D][GPU]")
  {
    UseTarget(8u, 8u);
    const SRGBA<uint8> red {255, 0, 0};
    constexpr PixelRect2D target {.X = 0, .Y = 0, .Width = 8u, .Height = 8u};
    PathRasteriser rasteriser;

    // A triangle and its mirror image, overlapping in the middle.
    const auto left = Triangle(0.f);
    Path2D right;
    right.MoveTo({8.f, 0.f});
    right.LineTo({8.f, 8.f});
    right.LineTo({0.f, 0.f});
    right.Close();
    const auto leftCoverage = rasteriser.Rasterise(left, {}, FillRule::NonZero, target);
    const auto rightCoverage = rasteriser.Rasterise(right, {}, FillRule::NonZero, target);
    auto coverage = [](const CoverageMask &mask, int32 x, int32 y) -> int
    {
      const auto &bounds = mask.Bounds;
      const bool inside = x >= bounds.X && y >= bounds.Y && x < bounds.X + static_cast<int32>(bounds.Width)
                          && y < bounds.Y + static_cast<int32>(bounds.Height);
      return inside ? mask.At(x, y) : 0;
    };

    // Checks every pixel against the red channel `expected` gives it.
    auto checkPixels = [&](auto &&expected, int tolerance)
    {
      auto pixels = Read();
      for (int32 y = 0; y < 8; ++y)
      {
        for (int32 x = 0; x < 8; ++x)
        {
          const auto value = static_cast<uint8>(expected(x, y));
          const auto pixel = At(pixels, static_cast<uint32>(x), static_cast<uint32>(y));
          if (!Near(pixel, {value, 0, 0, value}, tolerance))
          {
            FAIL_CHECK("Pixel (" << x << ", " << y << ") is " << int(pixel[3]) << ", not " << int(value));
          }
        }
      }
    };

    SECTION("To a path")
    {
      Draw(
        [&](Renderer2D &renderer)
        {
          renderer.ClipPath(left, FillRule::NonZero);
          renderer.FillRect({0.f, 0.f, 8.f, 8.f}, red);
        });
      checkPixels([&](int32 x, int32 y) { return coverage(leftCoverage, x, y); }, 1);
    }

    SECTION("Out of a path")
    {
      Draw(
        [&](Renderer2D &renderer)
        {
          renderer.ClipOutPath(left, FillRule::NonZero);
          renderer.FillRect({0.f, 0.f, 8.f, 8.f}, red);
        });
      checkPixels([&](int32 x, int32 y) { return 255 - coverage(leftCoverage, x, y); }, 1);
    }

    SECTION("Nested path clips multiply")
    {
      Draw(
        [&](Renderer2D &renderer)
        {
          renderer.ClipPath(left, FillRule::NonZero);
          renderer.ClipPath(right, FillRule::NonZero);
          renderer.FillRect({0.f, 0.f, 8.f, 8.f}, red);
        });
      checkPixels([&](int32 x, int32 y)
                  { return coverage(leftCoverage, x, y) * coverage(rightCoverage, x, y) / 255; }, 2);

      Draw(
        [&](Renderer2D &renderer)
        {
          renderer.ClipPath(left, FillRule::NonZero);
          renderer.ClipOutPath(right, FillRule::NonZero);
          renderer.FillRect({0.f, 0.f, 8.f, 8.f}, red);
        });
      checkPixels([&](int32 x, int32 y)
                  { return coverage(leftCoverage, x, y) * (255 - coverage(rightCoverage, x, y)) / 255; }, 2);
    }
  }

  TEST_CASE_METHOD(Renderer2DFixture, "Renderer2D: combines path clips with the drawing state",
                   "[Gfx][Render2D][GPU]")
  {
    UseTarget(8u, 8u);
    const SRGBA<uint8> red {255, 0, 0};

    SECTION("Shape clips, fill rules and transforms")
    {
      Path2D ring;
      ring.AddRect({0.f, 0.f, 4.f, 4.f});
      ring.AddRect({1.f, 1.f, 2.f, 2.f});
      Draw(
        [&](Renderer2D &renderer)
        {
          renderer.ClipRoundedRect({{0.f, 0.f, 8.f, 8.f}, CornerRadii2D::Uniform(4.f)});
          renderer.Scale(2.f, 2.f);
          renderer.ClipPath(ring, FillRule::EvenOdd);
          renderer.FillRect({0.f, 0.f, 4.f, 4.f}, red);
        });

      // The ring is 0 to 8 with a hole from 2 to 6; the rounded clip cuts its corners.
      auto pixels = Read();
      CHECK(At(pixels, 4u, 1u) == Red);
      CHECK(At(pixels, 1u, 4u) == Red);
      CHECK(At(pixels, 3u, 3u) == Clear);
      CHECK(At(pixels, 0u, 0u) == Clear);
    }

    SECTION("Restore ends a path clip")
    {
      Draw(
        [&](Renderer2D &renderer)
        {
          renderer.Save();
          renderer.ClipPath(Triangle(0.f), FillRule::NonZero);
          renderer.Restore();
          renderer.FillRect({0.f, 0.f, 8.f, 8.f}, red);
        });
      for (const auto &pixel : Read())
      {
        CHECK(pixel == Red);
      }
    }

    SECTION("Empty paths")
    {
      Path2D line;
      line.MoveTo({1.f, 1.f});
      line.LineTo({6.f, 6.f});
      Draw(
        [&](Renderer2D &renderer)
        {
          renderer.ClipPath(line, FillRule::NonZero);
          renderer.FillRect({0.f, 0.f, 8.f, 8.f}, red);
        });
      for (const auto &pixel : Read())
      {
        CHECK(pixel == Clear);
      }

      Draw(
        [&](Renderer2D &renderer)
        {
          renderer.ClipOutPath(Path2D {}, FillRule::NonZero);
          renderer.ClipOutPath(line, FillRule::NonZero);
          renderer.FillRect({0.f, 0.f, 8.f, 8.f}, red);
        });
      for (const auto &pixel : Read())
      {
        CHECK(pixel == Red);
      }
    }
  }

  TEST_CASE_METHOD(Renderer2DFixture, "Renderer2D: composites layers as one image", "[Gfx][Render2D][GPU]")
  {
    UseTarget(8u, 8u);
    const SRGBA<uint8> red {255, 0, 0};
    const SRGBA<uint8> blue {0, 0, 255};

    SECTION("Opacity applies to the group, not to each draw")
    {
      // Where the rects overlap, only the blue on top shows, at half opacity.
      Draw(
        [&](Renderer2D &renderer)
        {
          renderer.BeginLayer(0.5f);
          renderer.FillRect({0.f, 0.f, 6.f, 6.f}, red);
          renderer.FillRect({2.f, 2.f, 6.f, 6.f}, blue);
          renderer.EndLayer();
        });

      auto pixels = Read();
      CHECK(Near(At(pixels, 0u, 0u), {128, 0, 0, 128}));
      CHECK(Near(At(pixels, 3u, 3u), {0, 0, 128, 128}));
      CHECK(Near(At(pixels, 7u, 7u), {0, 0, 128, 128}));
      CHECK(At(pixels, 7u, 0u) == Clear);
    }

    SECTION("Layers nest, and keep painter's order")
    {
      Draw(
        [&](Renderer2D &renderer)
        {
          renderer.FillRect({0.f, 0.f, 8.f, 8.f}, blue);
          renderer.BeginLayer(0.5f);
          renderer.BeginLayer(0.5f);
          renderer.Translate(2.f, 0.f);
          renderer.FillRect({0.f, 0.f, 2.f, 2.f}, red);
          renderer.EndLayer();
          renderer.EndLayer();
          // Drawn after the layers, over them.
          renderer.FillRect({3.f, 0.f, 1.f, 1.f}, SRGBA<uint8> {0, 255, 0});
        });

      // A quarter of the red over the blue.
      auto pixels = Read();
      CHECK(Near(At(pixels, 2u, 0u), {64, 0, 191, 255}));
      CHECK(At(pixels, 3u, 0u) == Green);
      CHECK(At(pixels, 0u, 0u) == Blue);
      CHECK(At(pixels, 4u, 0u) == Blue);
    }

    SECTION("Nothing shows from empty or transparent layers")
    {
      Draw(
        [&](Renderer2D &renderer)
        {
          renderer.BeginLayer(1.f);
          renderer.EndLayer();
          renderer.BeginLayer(0.f);
          renderer.FillRect({0.f, 0.f, 8.f, 8.f}, red);
          renderer.EndLayer();
        });
      for (const auto &pixel : Read())
      {
        CHECK(pixel == Clear);
      }
    }
  }

  TEST_CASE_METHOD(Renderer2DFixture, "Renderer2D: draws with the Porter-Duff operators",
                   "[Gfx][Render2D][GPU]")
  {
    UseTarget(4u, 1u);
    const SRGBA<uint8> blue {0, 0, 255};
    const SRGBA<uint8> halfRed {255, 0, 0, 128};

    // The destination is opaque blue at 0 and 1; the source, half-transparent red, covers 1 and 2. Draws are
    // bounded: 0, which the source doesn't cover, keeps the blue, and 3 stays clear.
    struct Expected
    {
      CompositeOperator2D Operator;
      Pixel OverBlue;
      Pixel OverClear;
    };
    const Pixel source {128, 0, 0, 128};
    const Pixel none {0, 0, 0, 0};
    const Pixel opaqueBlue {0, 0, 255, 255};
    const Pixel halfBlue {0, 0, 128, 128};
    const Expected cases[] = {
      {CompositeOperator2D::SourceOver, {128, 0, 128, 255}, source},
      {CompositeOperator2D::Clear, none, none},
      {CompositeOperator2D::Copy, source, source},
      {CompositeOperator2D::SourceIn, source, none},
      {CompositeOperator2D::SourceOut, none, source},
      {CompositeOperator2D::SourceAtop, {128, 0, 128, 255}, none},
      {CompositeOperator2D::DestinationOver, opaqueBlue, source},
      {CompositeOperator2D::DestinationIn, halfBlue, none},
      {CompositeOperator2D::DestinationOut, halfBlue, none},
      {CompositeOperator2D::DestinationAtop, halfBlue, source},
      {CompositeOperator2D::Xor, halfBlue, source},
      {CompositeOperator2D::PlusLighter, {128, 0, 255, 255}, source},
    };
    for (const auto &expected : cases)
    {
      INFO("operator " << static_cast<int>(expected.Operator));
      Draw(
        [&](Renderer2D &renderer)
        {
          renderer.FillRect({0.f, 0.f, 2.f, 1.f}, blue);
          renderer.SetCompositeOperator(expected.Operator);
          renderer.FillRect({1.f, 0.f, 2.f, 1.f}, halfRed);
        });
      auto pixels = Read();
      CHECK(At(pixels, 0u, 0u) == opaqueBlue);
      CHECK(Near(At(pixels, 1u, 0u), expected.OverBlue));
      CHECK(Near(At(pixels, 2u, 0u), expected.OverClear));
      CHECK(At(pixels, 3u, 0u) == Clear);
    }
  }

  TEST_CASE_METHOD(Renderer2DFixture, "Renderer2D: composites a layer with an operator over its clip",
                   "[Gfx][Render2D][GPU]")
  {
    UseTarget(4u, 1u);
    const SRGBA<uint8> blue {0, 0, 255};
    const SRGBA<uint8> red {255, 0, 0};

    // A mask: blue kept where the layer draws, and cleared where it doesn't, as far as the clip reaches.
    auto mask = [&](bool clipped)
    {
      Draw(
        [&](Renderer2D &renderer)
        {
          renderer.FillRect({0.f, 0.f, 4.f, 1.f}, blue);
          if (clipped)
          {
            renderer.ClipRect({0.f, 0.f, 3.f, 1.f});
          }
          renderer.SetCompositeOperator(CompositeOperator2D::DestinationIn);
          renderer.BeginLayer(1.f);
          // Inside the layer, draws are source over again.
          CHECK(renderer.GetCompositeOperator() == CompositeOperator2D::SourceOver);
          renderer.FillRect({1.f, 0.f, 1.f, 1.f}, red);
          renderer.EndLayer();
        });
      return Read();
    };

    auto whole = mask(false);
    CHECK(At(whole, 0u, 0u) == Clear);
    CHECK(At(whole, 1u, 0u) == Blue);
    CHECK(At(whole, 2u, 0u) == Clear);
    CHECK(At(whole, 3u, 0u) == Clear);

    auto clipped = mask(true);
    CHECK(At(clipped, 0u, 0u) == Clear);
    CHECK(At(clipped, 1u, 0u) == Blue);
    CHECK(At(clipped, 3u, 0u) == Blue);
  }

  TEST_CASE_METHOD(Renderer2DFixture, "Renderer2D: turns a layer's luminance into alpha",
                   "[Gfx][Render2D][GPU]")
  {
    UseTarget(2u, 1u);
    Draw(
      [&](Renderer2D &renderer)
      {
        renderer.BeginLayer(1.f, BlendMode::Normal, LayerFilter2D::LuminanceToAlpha());
        renderer.FillRect({0.f, 0.f, 1.f, 1.f}, SRGBA<uint8> {255, 255, 255, 128});
        renderer.FillRect({1.f, 0.f, 1.f, 1.f}, SRGBA<uint8> {0, 255, 0});
        renderer.EndLayer();
      });

    // Half-transparent white: luminance 1 times alpha one half. Opaque green: green's luminance weight.
    auto pixels = Read();
    CHECK(Near(At(pixels, 0u, 0u), {0, 0, 0, 128}));
    CHECK(Near(At(pixels, 1u, 0u), {0, 0, 0, 182}));
  }

  TEST_CASE_METHOD(Renderer2DFixture, "Renderer2D: blurs a layer with duplicated edges",
                   "[Gfx][Render2D][GPU]")
  {
    UseTarget(16u, 16u);
    const SRGBA<uint8> red {255, 0, 0};

    // A solid red layer, clipped to the middle and blurred: with transparent edges the blur fades towards the
    // clip's edges, and with duplicated ones it stays solid up to them. Nothing shows outside the clip.
    auto draw = [&](EdgeMode2D edgeMode)
    {
      Draw(
        [&](Renderer2D &renderer)
        {
          renderer.ClipRect({4.f, 4.f, 8.f, 8.f});
          renderer.BeginLayer(1.f, BlendMode::Normal, LayerFilter2D::Blur(4.f, edgeMode));
          renderer.FillRect({0.f, 0.f, 16.f, 16.f}, red);
          renderer.EndLayer();
        });
      return Read();
    };

    auto faded = draw(EdgeMode2D::None);
    CHECK(At(faded, 4u, 4u)[3] < 200u);
    auto solid = draw(EdgeMode2D::Duplicate);
    CHECK(Near(At(solid, 4u, 4u), {255, 0, 0, 255}));
    CHECK(Near(At(solid, 11u, 7u), {255, 0, 0, 255}));
    CHECK(At(solid, 3u, 4u) == Clear);
    CHECK(At(solid, 12u, 12u) == Clear);
  }

  TEST_CASE_METHOD(Renderer2DFixture, "Renderer2D: draws the same through an opaque layer",
                   "[Gfx][Render2D][GPU]")
  {
    // An opaque layer changes nothing, including antialiased clip edges, which the composite doesn't fade
    // a second time.
    auto scene = [](Renderer2D &renderer, bool layer)
    {
      renderer.FillRect({0.f, 0.f, 8.f, 8.f}, SRGBA<uint8> {0, 0, 255});
      renderer.Save();
      renderer.ClipRoundedRect({{1.f, 1.f, 6.f, 6.f}, CornerRadii2D::Uniform(3.f)});
      if (layer)
      {
        renderer.BeginLayer(1.f);
      }
      renderer.FillRect({0.f, 0.f, 8.f, 8.f}, SRGBA<uint8> {255, 0, 0, 200});
      renderer.FillRoundedRect({{2.f, 2.f, 4.f, 4.f}, CornerRadii2D::Uniform(1.5f)},
                               SRGBA<uint8> {0, 255, 0, 160});
      if (layer)
      {
        renderer.EndLayer();
      }
      renderer.Restore();
    };

    UseTarget(8u, 8u);
    Draw([&](Renderer2D &renderer) { scene(renderer, false); });
    auto direct = Read();
    Draw([&](Renderer2D &renderer) { scene(renderer, true); });
    CHECK(SameImage(direct, Read(), 2));
  }

  namespace
  {
    struct Rgb
    {
      double R;
      double G;
      double B;
    };

    KRYS_NODISCARD double Lum(Rgb c)
    {
      return 0.3 * c.R + 0.59 * c.G + 0.11 * c.B;
    }

    KRYS_NODISCARD Rgb ClipColour(Rgb c)
    {
      const double l = Lum(c);
      const double n = std::min({c.R, c.G, c.B});
      const double x = std::max({c.R, c.G, c.B});
      if (n < 0.0)
      {
        c = {l + (c.R - l) * l / (l - n), l + (c.G - l) * l / (l - n), l + (c.B - l) * l / (l - n)};
      }
      if (x > 1.0)
      {
        c = {l + (c.R - l) * (1.0 - l) / (x - l), l + (c.G - l) * (1.0 - l) / (x - l),
             l + (c.B - l) * (1.0 - l) / (x - l)};
      }
      return c;
    }

    KRYS_NODISCARD Rgb SetLum(Rgb c, double l)
    {
      const double d = l - Lum(c);
      return ClipColour({c.R + d, c.G + d, c.B + d});
    }

    KRYS_NODISCARD double Sat(Rgb c)
    {
      return std::max({c.R, c.G, c.B}) - std::min({c.R, c.G, c.B});
    }

    // As the specification writes it: the largest channel becomes s, the smallest 0, the middle one scaled
    // between them.
    KRYS_NODISCARD Rgb SetSat(Rgb c, double s)
    {
      Array<double *, 3> channels {&c.R, &c.G, &c.B};
      std::sort(channels.begin(), channels.end(), [](double *a, double *b) { return *a < *b; });
      double &min = *channels[0];
      double &mid = *channels[1];
      double &max = *channels[2];
      if (max > min)
      {
        mid = (mid - min) * s / (max - min);
        max = s;
      }
      else
      {
        mid = 0.0;
        max = 0.0;
      }
      min = 0.0;
      return c;
    }

    KRYS_NODISCARD double Separable(BlendMode mode, double b, double s)
    {
      auto hardLight = [](double b, double s)
      {
        return s <= 0.5 ? b * 2.0 * s : b + (2.0 * s - 1.0) - b * (2.0 * s - 1.0);
      };
      switch (mode)
      {
        case BlendMode::Multiply:   return b * s;
        case BlendMode::Screen:     return b + s - b * s;
        case BlendMode::Darken:     return std::min(b, s);
        case BlendMode::Lighten:    return std::max(b, s);
        case BlendMode::Overlay:    return hardLight(s, b);
        case BlendMode::ColorDodge: return b == 0.0 ? 0.0 : s == 1.0 ? 1.0 : std::min(1.0, b / (1.0 - s));
        case BlendMode::ColorBurn:
          return b == 1.0 ? 1.0 : s == 0.0 ? 0.0 : 1.0 - std::min(1.0, (1.0 - b) / s);
        case BlendMode::HardLight: return hardLight(b, s);
        case BlendMode::SoftLight:
        {
          if (s <= 0.5)
          {
            return b - (1.0 - 2.0 * s) * b * (1.0 - b);
          }
          const double d = b <= 0.25 ? ((16.0 * b - 12.0) * b + 4.0) * b : std::sqrt(b);
          return b + (2.0 * s - 1.0) * (d - b);
        }
        case BlendMode::Difference: return std::abs(b - s);
        case BlendMode::Exclusion:  return b + s - 2.0 * b * s;
        default:                    return s;
      }
    }

    /// @brief The blend function of Compositing and Blending Level 1.
    KRYS_NODISCARD Rgb BlendFunction(BlendMode mode, Rgb b, Rgb s)
    {
      switch (mode)
      {
        case BlendMode::Hue:        return SetLum(SetSat(s, Sat(b)), Lum(b));
        case BlendMode::Saturation: return SetLum(SetSat(b, Sat(s)), Lum(b));
        case BlendMode::Color:      return SetLum(s, Lum(b));
        case BlendMode::Luminosity: return SetLum(b, Lum(s));
        default:                    return {Separable(mode, b.R, s.R), Separable(mode, b.G, s.G), Separable(mode, b.B, s.B)};
      }
    }

    /// @brief A straight colour blended with a premultiplied backdrop pixel, then composited over it, as the
    /// specification's formulas give it.
    KRYS_NODISCARD Pixel Blended(BlendMode mode, const Pixel &backdrop, const Pixel &source,
                                 double coverage = 1.0)
    {
      const double ab = backdrop[3] / 255.0;
      auto straight = [&](uint8 value)
      {
        return ab > 0.0 ? std::min(value / 255.0 / ab, 1.0) : 0.0;
      };
      const Rgb b {straight(backdrop[0]), straight(backdrop[1]), straight(backdrop[2])};
      const Rgb s {source[0] / 255.0, source[1] / 255.0, source[2] / 255.0};
      const double as = source[3] / 255.0 * coverage;
      const Rgb blended = BlendFunction(mode, b, s);

      auto channel = [&](double cs, double blendedChannel, uint8 premultipliedBackdrop)
      {
        const double mixed = (1.0 - ab) * cs + ab * std::clamp(blendedChannel, 0.0, 1.0);
        const double result = as * mixed + (1.0 - as) * premultipliedBackdrop / 255.0;
        return static_cast<uint8>(std::clamp(result, 0.0, 1.0) * 255.0 + 0.5);
      };
      return {channel(s.R, blended.R, backdrop[0]), channel(s.G, blended.G, backdrop[1]),
              channel(s.B, blended.B, backdrop[2]), static_cast<uint8>((as + ab * (1.0 - as)) * 255.0 + 0.5)};
    }

    KRYS_NODISCARD SRGBA<uint8> ToColour(const Pixel &pixel)
    {
      return {pixel[0], pixel[1], pixel[2], pixel[3]};
    }
  }

  TEST_CASE_METHOD(Renderer2DFixture, "Renderer2D: blends with every blend mode", "[Gfx][Render2D][GPU]")
  {
    // Four pairs, three pixels apart so each draw's reach misses the others' and they share one copy.
    constexpr Array<Pixel, 4> backdrops {Pixel {200, 60, 30, 255}, Pixel {40, 180, 220, 255},
                                         Pixel {200, 60, 30, 255}, Pixel {30, 120, 150, 160}};
    constexpr Array<Pixel, 4> sources {Pixel {90, 200, 140, 255}, Pixel {230, 30, 120, 180},
                                       Pixel {20, 240, 250, 100}, Pixel {120, 140, 10, 255}};
    UseTarget(12u, 1u);

    for (uint8 value = static_cast<uint8>(BlendMode::Multiply);
         value <= static_cast<uint8>(BlendMode::Luminosity); ++value)
    {
      const auto mode = static_cast<BlendMode>(value);
      CAPTURE(BlendModeName(mode));
      Draw(
        [&](Renderer2D &renderer)
        {
          for (size_t i = 0u; i < backdrops.size(); ++i)
          {
            renderer.FillRect({3.f * static_cast<float>(i), 0.f, 1.f, 1.f}, ToColour(backdrops[i]));
          }
        });
      const auto before = Read();

      Draw(
        [&](Renderer2D &renderer)
        {
          renderer.SetBlendMode(mode);
          for (size_t i = 0u; i < sources.size(); ++i)
          {
            renderer.FillRect({3.f * static_cast<float>(i), 0.f, 1.f, 1.f}, ToColour(sources[i]));
          }
        },
        Maybe<Array<float, 4>> {});
      CHECK(_renderer->GetStats().BackdropCopies == 1u);

      const auto after = Read();
      for (uint32 i = 0u; i < 4u; ++i)
      {
        CAPTURE(i);
        CHECK(Near(At(after, 3u * i, 0u), Blended(mode, At(before, 3u * i, 0u), sources[i]), 2));
        // Pixels the draws don't cover don't change.
        CHECK(At(after, 3u * i + 1u, 0u) == Clear);
      }
    }
  }

  TEST_CASE_METHOD(Renderer2DFixture, "Renderer2D: copies the backdrop once per overlapping draw",
                   "[Gfx][Render2D][GPU]")
  {
    UseTarget(8u, 2u);
    const SRGBA<uint8> grey {128, 128, 128};

    SECTION("Overlapping draws blend with each other")
    {
      Draw(
        [&](Renderer2D &renderer)
        {
          renderer.FillRect({0.f, 0.f, 8.f, 2.f}, SRGBA<uint8> {255, 255, 255});
          renderer.SetBlendMode(BlendMode::Multiply);
          renderer.FillRect({0.f, 0.f, 5.f, 2.f}, grey);
          renderer.FillRect({3.f, 0.f, 5.f, 2.f}, grey);
        });
      CHECK(_renderer->GetStats().BackdropCopies == 2u);
      auto pixels = Read();
      CHECK(Near(At(pixels, 0u, 0u), {128, 128, 128, 255}));
      CHECK(Near(At(pixels, 4u, 0u), {64, 64, 64, 255}));
      CHECK(Near(At(pixels, 7u, 1u), {128, 128, 128, 255}));
    }

    SECTION("Antialiased edges blend by their coverage")
    {
      const Pixel backdrop {200, 100, 50, 255};
      const Pixel source {128, 255, 128, 255};
      Draw(
        [&](Renderer2D &renderer)
        {
          renderer.FillRect({0.f, 0.f, 8.f, 2.f}, ToColour(backdrop));
          renderer.SetBlendMode(BlendMode::Multiply);
          renderer.FillRect({0.5f, 0.f, 3.f, 2.f}, ToColour(source));
        });
      auto pixels = Read();
      CHECK(Near(At(pixels, 0u, 0u), Blended(BlendMode::Multiply, backdrop, source, 0.5), 2));
      CHECK(Near(At(pixels, 1u, 0u), Blended(BlendMode::Multiply, backdrop, source), 2));
      CHECK(Near(At(pixels, 3u, 0u), Blended(BlendMode::Multiply, backdrop, source, 0.5), 2));
      CHECK(At(pixels, 5u, 0u) == backdrop);
    }

    SECTION("Plus lighter adds, without a copy")
    {
      Draw(
        [&](Renderer2D &renderer)
        {
          renderer.FillRect({0.f, 0.f, 8.f, 2.f}, SRGBA<uint8> {100, 50, 200});
          renderer.SetBlendMode(BlendMode::PlusLighter);
          renderer.FillRect({0.f, 0.f, 4.f, 2.f}, SRGBA<uint8> {100, 250, 100});
        });
      CHECK(_renderer->GetStats().BackdropCopies == 0u);
      auto pixels = Read();
      CHECK(Near(At(pixels, 0u, 0u), {200, 255, 255, 255}));
      CHECK(Near(At(pixels, 4u, 0u), {100, 50, 200, 255}));
    }

    SECTION("The blend mode is drawing state")
    {
      Draw(
        [&](Renderer2D &renderer)
        {
          renderer.FillRect({0.f, 0.f, 8.f, 2.f}, SRGBA<uint8> {255, 255, 255});
          renderer.Save();
          renderer.SetBlendMode(BlendMode::Multiply);
          CHECK(renderer.GetBlendMode() == BlendMode::Multiply);
          renderer.Restore();
          CHECK(renderer.GetBlendMode() == BlendMode::Normal);
          renderer.FillRect({0.f, 0.f, 8.f, 2.f}, SRGBA<uint8> {128, 128, 128, 128});
        });
      CHECK(_renderer->GetStats().BackdropCopies == 0u);
      CHECK(Near(Read()[0], {191, 191, 191, 255}));
    }
  }

  TEST_CASE_METHOD(Renderer2DFixture,
                   "Renderer2D: blends inside layers and composites layers with blend modes",
                   "[Gfx][Render2D][GPU]")
  {
    UseTarget(8u, 2u);

    SECTION("Inside a layer, draws blend with the layer's contents")
    {
      // Red times green is black; the blue beneath the layer isn't involved. The layer starts away from the
      // target's corner, so its pixels are offset from the target's.
      Draw(
        [&](Renderer2D &renderer)
        {
          renderer.FillRect({0.f, 0.f, 8.f, 2.f}, SRGBA<uint8> {0, 0, 255});
          renderer.BeginLayer(1.f);
          renderer.FillRect({4.f, 0.f, 4.f, 2.f}, SRGBA<uint8> {255, 0, 0});
          renderer.SetBlendMode(BlendMode::Multiply);
          renderer.FillRect({5.f, 0.f, 2.f, 2.f}, SRGBA<uint8> {0, 255, 0});
          renderer.EndLayer();
        });
      auto pixels = Read();
      CHECK(At(pixels, 4u, 0u) == Red);
      CHECK(Near(At(pixels, 5u, 1u), {0, 0, 0, 255}));
      CHECK(At(pixels, 7u, 0u) == Red);
      CHECK(At(pixels, 0u, 0u) == Blue);
    }

    SECTION("A layer composites with the blend mode it began with")
    {
      const Pixel backdrop {200, 100, 50, 255};
      const Pixel source {128, 255, 128, 255};
      Draw(
        [&](Renderer2D &renderer)
        {
          renderer.FillRect({0.f, 0.f, 8.f, 2.f}, ToColour(backdrop));
          renderer.SetBlendMode(BlendMode::Multiply);
          renderer.BeginLayer(1.f);
          // Inside, draws are normal again: the two rects don't multiply each other.
          CHECK(renderer.GetBlendMode() == BlendMode::Normal);
          renderer.FillRect({2.f, 0.f, 4.f, 2.f}, SRGBA<uint8> {0, 0, 0});
          renderer.FillRect({2.f, 0.f, 4.f, 2.f}, ToColour(source));
          renderer.EndLayer();
          CHECK(renderer.GetBlendMode() == BlendMode::Multiply);
          renderer.SetBlendMode(BlendMode::Normal);
          renderer.BeginLayer(0.5f, BlendMode::Screen);
          renderer.FillRect({6.f, 0.f, 2.f, 2.f}, ToColour(source));
          renderer.EndLayer();
        });
      auto pixels = Read();
      CHECK(At(pixels, 0u, 0u) == backdrop);
      CHECK(Near(At(pixels, 3u, 1u), Blended(BlendMode::Multiply, backdrop, source), 2));
      CHECK(Near(At(pixels, 7u, 0u), Blended(BlendMode::Screen, backdrop, {128, 255, 128, 128}), 2));
    }
  }

  namespace
  {
    using GradientShape2D = decltype(Gradient2D::Shape);

    /// @brief Black at offset 0 to white at 1.
    KRYS_NODISCARD Gradient2D Greys(GradientShape2D shape,
                                    GradientSpreadMethod spread = GradientSpreadMethod::Pad)
    {
      return {.Shape = shape,
              .Stops = {{.Offset = 0.f, .Colour = {0.f, 0.f, 0.f, 1.f}},
                        {.Offset = 1.f, .Colour = {1.f, 1.f, 1.f, 1.f}}},
              .Spread = spread};
    }

    KRYS_NODISCARD double Spread(double t, GradientSpreadMethod spread)
    {
      switch (spread)
      {
        case GradientSpreadMethod::Repeat: return t - std::floor(t);
        case GradientSpreadMethod::Reflect:
        {
          const double m = t - 2.0 * std::floor(t / 2.0);
          return m > 1.0 ? 2.0 - m : m;
        }
        default: return t;
      }
    }

    KRYS_NODISCARD Pixel Grey(double t)
    {
      const auto value = static_cast<uint8>(std::clamp(t, 0.0, 1.0) * 255.0 + 0.5);
      return {value, value, value, 255};
    }

    /// @brief How many pixels differ by more than a step from the grey at the offset `offset(x, y)` gives
    /// their centres; empty offsets expect transparent pixels, and NaN skips the pixel.
    template <typename TFunc>
    KRYS_NODISCARD uint32 GreyMismatches(const List<Pixel> &pixels, uint32 width, GradientSpreadMethod spread,
                                         TFunc &&offset)
    {
      uint32 mismatches = 0u;
      for (uint32 i = 0u; i < pixels.size(); ++i)
      {
        const double x = static_cast<double>(i % width) + 0.5;
        const double y = static_cast<double>(i / width) + 0.5;
        const Maybe<double> t = offset(x, y);
        if (t && std::isnan(*t))
        {
          continue;
        }
        const Pixel expected = t ? Grey(Spread(*t, spread)) : Clear;
        if (!Near(pixels[i], expected))
        {
          if (mismatches == 0u)
          {
            UNSCOPED_INFO("first mismatch at " << x << ", " << y << ": " << int(pixels[i][0]) << " for "
                                               << int(expected[0]));
          }
          ++mismatches;
        }
      }
      return mismatches;
    }

    /// @brief The largest offset whose circle, with a radius that isn't negative, passes through the point.
    KRYS_NODISCARD Maybe<double> ConicalOffset(double x, double y, Vec2f c0, float r0, Vec2f c1, float r1)
    {
      const double cdx = c1.x - c0.x;
      const double cdy = c1.y - c0.y;
      const double px = x - c0.x;
      const double py = y - c0.y;
      const double dr = static_cast<double>(r1) - r0;
      const double a = cdx * cdx + cdy * cdy - dr * dr;
      const double b = px * cdx + py * cdy + r0 * dr;
      const double c = px * px + py * py - static_cast<double>(r0) * r0;
      List<double> roots;
      if (a == 0.0)
      {
        if (b != 0.0)
        {
          roots.push_back(c / (2.0 * b));
        }
      }
      else if (b * b - a * c >= 0.0)
      {
        roots.push_back((b + std::sqrt(b * b - a * c)) / a);
        roots.push_back((b - std::sqrt(b * b - a * c)) / a);
      }
      std::sort(roots.begin(), roots.end(), std::greater<double> {});
      for (const double t : roots)
      {
        if (r0 + t * dr >= 0.0)
        {
          return t;
        }
      }
      return {};
    }

    constexpr double Skip = std::numeric_limits<double>::quiet_NaN();
  }

  TEST_CASE_METHOD(Renderer2DFixture, "Renderer2D: fills linear gradients with each spread method",
                   "[Gfx][Render2D][GPU]")
  {
    UseTarget(16u, 2u);
    for (const auto spread :
         {GradientSpreadMethod::Pad, GradientSpreadMethod::Repeat, GradientSpreadMethod::Reflect})
    {
      INFO("spread " << int(spread));
      // The line runs over a quarter of the target, so the rest is padded or repeated.
      const auto gradient = Greys(LinearGradient2D {{0.f, 0.f}, {4.f, 0.f}}, spread);
      Draw([&](Renderer2D &renderer) { renderer.FillRect({0.f, 0.f, 16.f, 2.f}, gradient); });
      CHECK(GreyMismatches(Read(), _width, spread, [](double x, double) -> Maybe<double> { return x / 4.0; })
            == 0u);
    }

    // Down the target, and backwards along a diagonal.
    const auto down = Greys(LinearGradient2D {{0.f, 0.f}, {0.f, 2.f}});
    Draw([&](Renderer2D &renderer) { renderer.FillRect({0.f, 0.f, 16.f, 2.f}, down); });
    CHECK(GreyMismatches(Read(), _width, GradientSpreadMethod::Pad,
                         [](double, double y) -> Maybe<double> { return y / 2.0; })
          == 0u);
    const auto diagonal = Greys(LinearGradient2D {{16.f, 2.f}, {0.f, 0.f}});
    Draw([&](Renderer2D &renderer) { renderer.FillRect({0.f, 0.f, 16.f, 2.f}, diagonal); });
    CHECK(GreyMismatches(Read(), _width, GradientSpreadMethod::Pad, [](double x, double y) -> Maybe<double>
                         { return ((16.0 - x) * 16.0 + (2.0 - y) * 2.0) / (16.0 * 16.0 + 2.0 * 2.0); })
          == 0u);
  }

  TEST_CASE_METHOD(Renderer2DFixture, "Renderer2D: keeps gradients' hard stops sharp", "[Gfx][Render2D][GPU]")
  {
    UseTarget(8u, 1u);
    const Gradient2D gradient {.Shape = LinearGradient2D {{0.f, 0.f}, {8.f, 0.f}},
                               .Stops = {{.Offset = 0.f, .Colour = {1.f, 0.f, 0.f, 1.f}},
                                         {.Offset = 0.5f, .Colour = {1.f, 0.f, 0.f, 1.f}},
                                         {.Offset = 0.5f, .Colour = {0.f, 0.f, 1.f, 1.f}},
                                         {.Offset = 1.f, .Colour = {0.f, 0.f, 1.f, 1.f}}}};
    Draw([&](Renderer2D &renderer) { renderer.FillRect({0.f, 0.f, 8.f, 1.f}, gradient); });
    auto pixels = Read();
    CHECK(At(pixels, 0u, 0u) == Red);
    CHECK(At(pixels, 3u, 0u) == Red);
    CHECK(At(pixels, 4u, 0u) == Blue);
    CHECK(At(pixels, 7u, 0u) == Blue);
  }

  TEST_CASE_METHOD(Renderer2DFixture, "Renderer2D: fills radial gradients", "[Gfx][Render2D][GPU]")
  {
    UseTarget(16u, 16u);

    SECTION("One centre, growing from nothing, as CSS draws them")
    {
      for (const auto spread : {GradientSpreadMethod::Pad, GradientSpreadMethod::Repeat})
      {
        INFO("spread " << int(spread));
        const auto gradient = Greys(RadialGradient2D {{8.f, 8.f}, {8.f, 8.f}, 0.f, 6.f}, spread);
        Draw([&](Renderer2D &renderer) { renderer.FillRect({0.f, 0.f, 16.f, 16.f}, gradient); });
        CHECK(GreyMismatches(Read(), _width, spread, [](double x, double y) -> Maybe<double>
                             { return std::hypot(x - 8.0, y - 8.0) / 6.0; })
              == 0u);
      }
    }

    SECTION("Elliptical, squashed vertically about the centre")
    {
      const auto gradient = Greys(RadialGradient2D {{8.f, 8.f}, {8.f, 8.f}, 0.f, 8.f, 2.f});
      Draw([&](Renderer2D &renderer) { renderer.FillRect({0.f, 0.f, 16.f, 16.f}, gradient); });
      CHECK(GreyMismatches(Read(), _width, GradientSpreadMethod::Pad, [](double x, double y) -> Maybe<double>
                           { return std::hypot(x - 8.0, (y - 8.0) * 2.0) / 8.0; })
            == 0u);
    }

    SECTION("Two circles: a focal point inside the end circle")
    {
      const Vec2f c0 {5.f, 8.f};
      const Vec2f c1 {8.f, 8.f};
      const auto gradient = Greys(RadialGradient2D {c0, c1, 0.f, 7.f});
      Draw([&](Renderer2D &renderer) { renderer.FillRect({0.f, 0.f, 16.f, 16.f}, gradient); });
      CHECK(GreyMismatches(Read(), _width, GradientSpreadMethod::Pad,
                           [&](double x, double y) { return ConicalOffset(x, y, c0, 0.f, c1, 7.f); })
            == 0u);
    }

    SECTION("Two circles: a start circle larger than the end")
    {
      const Vec2f c0 {6.f, 6.f};
      const Vec2f c1 {9.f, 10.f};
      const auto gradient = Greys(RadialGradient2D {c0, c1, 7.f, 2.f}, GradientSpreadMethod::Reflect);
      Draw([&](Renderer2D &renderer) { renderer.FillRect({0.f, 0.f, 16.f, 16.f}, gradient); });
      CHECK(GreyMismatches(Read(), _width, GradientSpreadMethod::Reflect,
                           [&](double x, double y) { return ConicalOffset(x, y, c0, 7.f, c1, 2.f); })
            == 0u);
    }

    SECTION("Two circles of one size: a strip, with nothing beside it")
    {
      const Vec2f c0 {2.f, 8.f};
      const Vec2f c1 {14.f, 8.f};
      const auto gradient = Greys(RadialGradient2D {c0, c1, 2.f, 2.f});
      Draw([&](Renderer2D &renderer) { renderer.FillRect({0.f, 0.f, 16.f, 16.f}, gradient); });
      auto pixels = Read();
      CHECK(At(pixels, 8u, 1u) == Clear);
      CHECK(At(pixels, 8u, 14u) == Clear);
      CHECK(GreyMismatches(pixels, _width, GradientSpreadMethod::Pad,
                           [&](double x, double y) -> Maybe<double>
                           {
                             // The strip's edge isn't antialiased, so pixels next to it can go either way.
                             const double across = std::abs(y - 8.0);
                             if (across > 1.5 && across < 2.5)
                             {
                               return Skip;
                             }
                             return ConicalOffset(x, y, c0, 2.f, c1, 2.f);
                           })
            == 0u);
    }
  }

  TEST_CASE_METHOD(Renderer2DFixture, "Renderer2D: fills conic gradients clockwise from their angle",
                   "[Gfx][Render2D][GPU]")
  {
    UseTarget(16u, 16u);
    for (const float angle : {0.f, std::numbers::pi_v<float> / 2.f, -1.f})
    {
      INFO("angle " << angle);
      const auto gradient = Greys(ConicGradient2D {{8.f, 8.f}, angle});
      Draw([&](Renderer2D &renderer) { renderer.FillRect({0.f, 0.f, 16.f, 16.f}, gradient); });
      CHECK(GreyMismatches(Read(), _width, GradientSpreadMethod::Pad,
                           [&](double x, double y) -> Maybe<double>
                           {
                             // Up is 0, and angles go clockwise, as y points down. Near the centre and the
                             // seam, a pixel's colour changes too fast to compare.
                             const double dx = x - 8.0;
                             const double dy = y - 8.0;
                             double t = (std::atan2(dx, -dy) - angle) / (2.0 * std::numbers::pi);
                             t -= std::floor(t);
                             if (std::hypot(dx, dy) < 2.0 || t < 0.02 || t > 0.98)
                             {
                               return Skip;
                             }
                             return t;
                           })
            == 0u);
    }
  }

  TEST_CASE_METHOD(Renderer2DFixture, "Renderer2D: maps gradients through the transform and gradient space",
                   "[Gfx][Render2D][GPU]")
  {
    UseTarget(16u, 2u);
    // Local x is target x less 2, halved; the gradient's coordinates are a further 1 along.
    const auto gradient = Greys(LinearGradient2D {{0.f, 0.f}, {4.f, 0.f}});
    Draw(
      [&](Renderer2D &renderer)
      {
        renderer.Translate(2.f, 0.f);
        renderer.Scale(2.f, 1.f);
        renderer.FillRect({-1.f, 0.f, 8.f, 2.f}, Paint2D {gradient, Transform2D::Translation(1.f, 0.f)});
      });
    CHECK(GreyMismatches(Read(), _width, GradientSpreadMethod::Pad,
                         [](double x, double) -> Maybe<double> { return ((x - 2.0) / 2.0 - 1.0) / 4.0; })
          == 0u);

    // A gradient space that flattens the plane shows nothing.
    Draw([&](Renderer2D &renderer)
         { renderer.FillRect({0.f, 0.f, 16.f, 2.f}, Paint2D {gradient, Transform2D::Scale(0.f, 1.f)}); });
    CHECK(Read()[0] == Clear);
  }

  TEST_CASE_METHOD(Renderer2DFixture, "Renderer2D: paints gradients through masks, strokes, clips and layers",
                   "[Gfx][Render2D][GPU]")
  {
    UseTarget(16u, 2u);
    const auto gradient = Greys(LinearGradient2D {{0.f, 0.f}, {16.f, 0.f}});
    auto alongX = [](double x, double) -> Maybe<double>
    {
      return x / 16.0;
    };

    SECTION("Paths")
    {
      Path2D path;
      path.AddRect({0.f, 0.f, 16.f, 2.f});
      Draw([&](Renderer2D &renderer) { renderer.FillPath(path, FillRule::NonZero, gradient); });
      CHECK(GreyMismatches(Read(), _width, GradientSpreadMethod::Pad, alongX) == 0u);
    }

    SECTION("Strokes, in the current coordinates rather than the line's")
    {
      Draw([&](Renderer2D &renderer)
           { renderer.StrokeLine({16.f, 1.f}, {0.f, 1.f}, {.Width = 2.f}, gradient); });
      CHECK(GreyMismatches(Read(), _width, GradientSpreadMethod::Pad, alongX) == 0u);
      Path2D path;
      path.MoveTo({16.f, 1.f});
      path.LineTo({0.f, 1.f});
      Draw([&](Renderer2D &renderer) { renderer.StrokePath(path, {.Width = 2.f}, gradient); });
      CHECK(GreyMismatches(Read(), _width, GradientSpreadMethod::Pad, alongX) == 0u);
    }

    SECTION("Clips and layers away from the target's corner")
    {
      Draw(
        [&](Renderer2D &renderer)
        {
          renderer.ClipRoundedRect({{4.f, 0.f, 12.f, 2.f}, CornerRadii2D::Uniform(0.5f)});
          renderer.BeginLayer(1.f);
          renderer.FillRect({0.f, 0.f, 16.f, 2.f}, gradient);
          renderer.EndLayer();
        });
      CHECK(GreyMismatches(Read(), _width, GradientSpreadMethod::Pad,
                           [](double x, double) -> Maybe<double>
                           {
                             if (x < 4.0)
                             {
                               return {};
                             }
                             // The clip's rounded corners.
                             return x < 5.0 || x > 15.0 ? Skip : x / 16.0;
                           })
            == 0u);
    }
  }

  TEST_CASE_METHOD(Renderer2DFixture, "Renderer2D: fills degenerate gradients with one colour",
                   "[Gfx][Render2D][GPU]")
  {
    UseTarget(16u, 16u);
    auto fill = [&](const Gradient2D &gradient)
    {
      Draw([&](Renderer2D &renderer) { renderer.FillRect({0.f, 0.f, 16.f, 16.f}, gradient); });
      return Read();
    };

    // A line of no length is the last colour when padded and the average otherwise.
    auto pixels = fill(Greys(LinearGradient2D {{4.f, 4.f}, {4.f, 4.f}}));
    CHECK(At(pixels, 0u, 0u) == White);
    CHECK(At(pixels, 15u, 15u) == White);
    pixels = fill(Greys(LinearGradient2D {{4.f, 4.f}, {4.f, 4.f}}, GradientSpreadMethod::Repeat));
    CHECK(Near(At(pixels, 0u, 0u), {128, 128, 128, 255}));

    // Coinciding circles, padded: the first colour inside them and the last outside.
    pixels = fill(Greys(RadialGradient2D {{8.f, 8.f}, {8.f, 8.f}, 4.f, 4.f}));
    CHECK(At(pixels, 8u, 8u) == Pixel {0, 0, 0, 255});
    CHECK(At(pixels, 1u, 1u) == White);
    pixels = fill(Greys(RadialGradient2D {{8.f, 8.f}, {8.f, 8.f}, 4.f, 4.f}, GradientSpreadMethod::Reflect));
    CHECK(Near(At(pixels, 8u, 8u), {128, 128, 128, 255}));

    // One stop is a solid fill, none is nothing.
    pixels = fill({.Shape = LinearGradient2D {{0.f, 0.f}, {16.f, 0.f}},
                   .Stops = {{.Offset = 0.5f, .Colour = {0.f, 0.f, 1.f, 1.f}}}});
    CHECK(At(pixels, 0u, 0u) == Blue);
    CHECK(At(pixels, 15u, 15u) == Blue);
    pixels = fill({.Shape = LinearGradient2D {{0.f, 0.f}, {16.f, 0.f}}});
    CHECK(At(pixels, 0u, 0u) == Clear);
  }

  TEST_CASE_METHOD(Renderer2DFixture, "Renderer2D: batches draws with the same gradient",
                   "[Gfx][Render2D][GPU]")
  {
    UseTarget(16u, 2u);
    const auto gradient = Greys(LinearGradient2D {{0.f, 0.f}, {16.f, 0.f}});
    const auto other = Greys(LinearGradient2D {{16.f, 0.f}, {0.f, 0.f}});
    Draw(
      [&](Renderer2D &renderer)
      {
        renderer.FillRect({0.f, 0.f, 4.f, 2.f}, gradient);
        renderer.FillRect({4.f, 0.f, 4.f, 2.f}, SRGBA<uint8> {255, 0, 0});
        renderer.FillRect({8.f, 0.f, 4.f, 2.f}, gradient);
      });
    CHECK(_renderer->GetStats().Batches == 1u);
    auto pixels = Read();
    CHECK(At(pixels, 5u, 0u) == Red);
    CHECK(Near(At(pixels, 9u, 0u), Grey(9.5 / 16.0)));

    Draw(
      [&](Renderer2D &renderer)
      {
        renderer.FillRect({0.f, 0.f, 8.f, 2.f}, gradient);
        renderer.FillRect({8.f, 0.f, 8.f, 2.f}, other);
      });
    CHECK(_renderer->GetStats().Batches == 2u);
    pixels = Read();
    CHECK(Near(At(pixels, 1u, 0u), Grey(1.5 / 16.0)));
    CHECK(Near(At(pixels, 9u, 0u), Grey(6.5 / 16.0)));
  }

  TEST_CASE_METHOD(Renderer2DFixture, "Renderer2D: dithers gradients on 8-bit targets",
                   "[Gfx][Render2D][GPU]")
  {
    // A quarter of a step above 100: undithered, every pixel would be 100; dithered by an 8-by-8 ordered
    // pattern, about a quarter of them are 101 (16 exactly, but GPUs don't all round alike so near the
    // threshold).
    UseTarget(8u, 8u);
    const float level = 100.25f / 255.f;
    const Gradient2D gradient {.Shape = LinearGradient2D {{0.f, 0.f}, {8.f, 0.f}},
                               .Stops = {{.Offset = 0.f, .Colour = {level, level, level, 1.f}},
                                         {.Offset = 1.f, .Colour = {level, level, level, 1.f}}}};
    Draw([&](Renderer2D &renderer) { renderer.FillRect({0.f, 0.f, 8.f, 8.f}, gradient); });
    uint32 lower = 0u;
    uint32 upper = 0u;
    for (const auto &pixel : Read())
    {
      lower += pixel[0] == 100u ? 1u : 0u;
      upper += pixel[0] == 101u ? 1u : 0u;
    }
    CHECK(lower + upper == 64u);
    CHECK(upper >= 10u);
    CHECK(upper <= 22u);

    // Solid colours aren't dithered.
    Draw([&](Renderer2D &renderer)
         { renderer.FillRect({0.f, 0.f, 8.f, 8.f}, SRGBA<uint8> {100, 100, 100}); });
    const auto pixels = Read();
    CHECK(std::all_of(pixels.begin(), pixels.end(), [](const Pixel &pixel) { return pixel[0] == 100u; }));
  }

  namespace
  {
    /// @brief How much of a Gaussian with standard deviation `sigma` centred on `p` falls between `low` and
    /// `high`.
    KRYS_NODISCARD double GaussianMass(double low, double high, double p, double sigma)
    {
      const double k = 1.0 / (sigma * std::sqrt(2.0));
      return 0.5 * (std::erf((high - p) * k) - std::erf((low - p) * k));
    }

    /// @brief A rect with corners of radius `radius`, blurred, at (px, py): across each of many thin rows of
    /// the shape the blur is exact, and the rows are summed.
    KRYS_NODISCARD double BlurredRoundedRect(double px, double py, const Rect2D &rect, double radius,
                                             double sigma)
    {
      constexpr int rows = 512;
      const double height = rect.Height;
      double total = 0.0;
      for (int i = 0; i < rows; ++i)
      {
        const double y0 = rect.Y + height * i / rows;
        const double y1 = rect.Y + height * (i + 1) / rows;
        const double y = (y0 + y1) * 0.5;
        const double fromEdge = std::min(y - rect.Y, rect.Y + height - y);
        const double inset =
          fromEdge < radius ? radius - std::sqrt(radius * radius - (radius - fromEdge) * (radius - fromEdge))
                            : 0.0;
        total += GaussianMass(y0, y1, py, sigma)
                 * GaussianMass(rect.X + inset, rect.X + rect.Width - inset, px, sigma);
      }
      return total;
    }

    /// @brief How many pixels' alpha differs from `expected(x, y)` (from 0 to 1, at their centres) by more
    /// than `tolerance` steps; NaN skips a pixel.
    template <typename TFunc>
    KRYS_NODISCARD uint32 AlphaMismatches(const List<Pixel> &pixels, uint32 width, int tolerance,
                                          TFunc &&expected)
    {
      uint32 mismatches = 0u;
      for (uint32 i = 0u; i < pixels.size(); ++i)
      {
        const double x = static_cast<double>(i % width) + 0.5;
        const double y = static_cast<double>(i / width) + 0.5;
        const double value = expected(x, y);
        if (std::isnan(value))
        {
          continue;
        }
        const int alpha = static_cast<int>(value * 255.0 + 0.5);
        if (std::abs(int(pixels[i][3]) - alpha) > tolerance)
        {
          if (mismatches == 0u)
          {
            UNSCOPED_INFO("first mismatch at " << x << ", " << y << ": " << int(pixels[i][3]) << " for "
                                               << alpha);
          }
          ++mismatches;
        }
      }
      return mismatches;
    }

    /// @brief Draws only the shadow of a fill, as `box-shadow` does: the fill is moved out of the clip and
    /// the shadow's offset brings it back.
    void ShadowOnly(Renderer2D &renderer, float blur, auto &&fill)
    {
      renderer.Save();
      renderer.ClipRect({0.f, 0.f, 32.f, 32.f});
      renderer.SetDropShadow({.Offset = {-100.f, 0.f}, .BlurRadius = blur, .Colour = SRGBA<uint8> {0, 0, 0}});
      renderer.Translate(100.f, 0.f);
      fill(renderer);
      renderer.Restore();
    }
  }

  TEST_CASE_METHOD(Renderer2DFixture, "Renderer2D: blurs rect and rounded rect shadows analytically",
                   "[Gfx][Render2D][GPU]")
  {
    UseTarget(32u, 32u);
    const Rect2D rect {8.f, 6.f, 16.f, 20.f};

    SECTION("A rect is exact")
    {
      for (const float blur : {1.f, 4.f, 12.f})
      {
        INFO("blur " << blur);
        Draw(
          [&](Renderer2D &renderer)
          {
            ShadowOnly(renderer, blur, [&](Renderer2D &r) { r.FillRect(rect, SRGBA<uint8> {255, 0, 0}); });
          });
        const double sigma = blur / 2.0;
        CHECK(
          AlphaMismatches(Read(), _width, 1, [&](double x, double y)
                          { return GaussianMass(8.0, 24.0, x, sigma) * GaussianMass(6.0, 26.0, y, sigma); })
          == 0u);
      }
    }

    SECTION("A rounded rect is close")
    {
      for (const float blur : {2.f, 6.f, 16.f})
      {
        INFO("blur " << blur);
        Draw(
          [&](Renderer2D &renderer)
          {
            ShadowOnly(renderer, blur, [&](Renderer2D &r)
                       { r.FillRoundedRect({rect, CornerRadii2D::Uniform(6.f)}, SRGBA<uint8> {255, 0, 0}); });
          });
        CHECK(AlphaMismatches(Read(), _width, 2, [&](double x, double y)
                              { return BlurredRoundedRect(x, y, rect, 6.0, blur / 2.0); })
              == 0u);
      }
    }
  }

  TEST_CASE_METHOD(Renderer2DFixture, "Renderer2D: casts inset shadows through rounded holes",
                   "[Gfx][Render2D][GPU]")
  {
    // As `box-shadow: inset` draws them: a rect with the box as its hole, moved away, inside a clip to the
    // box. The shadow is the rect's blur less the hole's.
    UseTarget(32u, 32u);
    const Rect2D box {6.f, 6.f, 20.f, 20.f};
    Draw(
      [&](Renderer2D &renderer)
      {
        renderer.ClipRoundedRect({box, CornerRadii2D::Uniform(5.f)});
        renderer.SetDropShadow({.Offset = {-97.f, 3.f}, .BlurRadius = 6.f, .Colour = SRGBA<uint8> {0, 0, 0}});
        renderer.Translate(100.f, 0.f);
        renderer.FillRectWithRoundedHole({-20.f, -20.f, 72.f, 72.f}, {box, CornerRadii2D::Uniform(5.f)},
                                         SRGBA<uint8> {255, 0, 0});
      });
    CHECK(AlphaMismatches(Read(), _width, 2,
                          [&](double x, double y)
                          {
                            // Inside the box, away from its antialiased edge.
                            if (x < 7.0 || x > 25.0 || y < 7.0 || y > 25.0
                                || (std::min(x - 6.0, 26.0 - x) < 5.0 && std::min(y - 6.0, 26.0 - y) < 5.0))
                            {
                              return std::numeric_limits<double>::quiet_NaN();
                            }
                            // The hole as the shadow sees it: moved by the offset less the translation.
                            const Rect2D hole {box.X + 3.f, box.Y + 3.f, box.Width, box.Height};
                            const double sigma = 3.0;
                            const double outer =
                              GaussianMass(-17.0, 55.0, x, sigma) * GaussianMass(-17.0, 55.0, y, sigma);
                            return std::max(outer - BlurredRoundedRect(x, y, hole, 5.0, sigma), 0.0);
                          })
          == 0u);
  }

  TEST_CASE_METHOD(Renderer2DFixture, "Renderer2D: places shadows by their offset and the transform",
                   "[Gfx][Render2D][GPU]")
  {
    UseTarget(16u, 4u);
    const SRGBA<uint8> blue {0, 0, 255};

    SECTION("Sharp shadows, beneath their shapes")
    {
      Draw(
        [&](Renderer2D &renderer)
        {
          renderer.SetDropShadow({.Offset = {2.f, 0.f}, .Colour = blue});
          renderer.FillRect({2.f, 0.f, 4.f, 2.f}, SRGBA<uint8> {255, 0, 0});
        });
      auto pixels = Read();
      CHECK(At(pixels, 1u, 0u) == Clear);
      CHECK(At(pixels, 5u, 0u) == Red);
      CHECK(At(pixels, 7u, 0u) == Blue);
      CHECK(At(pixels, 8u, 0u) == Clear);
      CHECK(At(pixels, 7u, 2u) == Clear);
      CHECK(_renderer->GetStats().Instances == 2u);
      CHECK(_renderer->GetStats().Batches == 1u);
    }

    SECTION("A translucent paint's shadow is as translucent")
    {
      Draw(
        [&](Renderer2D &renderer)
        {
          renderer.SetDropShadow({.Offset = {4.f, 0.f}, .Colour = SRGBA<uint8> {0, 0, 255, 128}});
          renderer.FillRect({0.f, 0.f, 4.f, 2.f}, SRGBA<uint8> {255, 0, 0, 128});
        });
      CHECK(Near(At(Read(), 5u, 0u), {0, 0, 64, 64}));
    }

    SECTION("In the current coordinates, or in target pixels")
    {
      for (const bool ignore : {false, true})
      {
        INFO("ignoring transforms " << ignore);
        Draw(
          [&](Renderer2D &renderer)
          {
            renderer.Scale(2.f, 2.f);
            renderer.SetShadowsIgnoreTransforms(ignore);
            renderer.SetDropShadow({.Offset = {2.f, 0.f}, .Colour = blue});
            renderer.FillRect({0.f, 0.f, 2.f, 1.f}, SRGBA<uint8> {255, 0, 0});
          });
        // The shape covers 0 to 4; the shadow is 4 or 2 pixels along.
        auto pixels = Read();
        CHECK(At(pixels, 3u, 0u) == Red);
        CHECK(At(pixels, 5u, 0u) == Blue);
        CHECK(At(pixels, 7u, 0u) == (ignore ? Clear : Blue));
      }
    }

    SECTION("Kept by Save and Restore; nothing without an offset or blur")
    {
      Draw(
        [&](Renderer2D &renderer)
        {
          renderer.Save();
          renderer.SetDropShadow({.Offset = {1.f, 0.f}, .Colour = blue});
          CHECK(renderer.GetDropShadow().has_value());
          renderer.Restore();
          CHECK_FALSE(renderer.GetDropShadow().has_value());
          renderer.SetDropShadow({.Offset = {}, .BlurRadius = 0.f, .Colour = blue});
          renderer.FillRect({0.f, 0.f, 2.f, 2.f}, SRGBA<uint8> {255, 0, 0});
          renderer.SetDropShadow({.Offset = {2.f, 0.f}, .Colour = blue});
          renderer.ClearDropShadow();
          renderer.FillRect({4.f, 0.f, 2.f, 2.f}, SRGBA<uint8> {255, 0, 0});
        });
      CHECK(_renderer->GetStats().Instances == 2u);
      CHECK(At(Read(), 6u, 0u) == Clear);
    }

    SECTION("Images cast none")
    {
      const Pixel texels[] {White};
      const auto image = Image(1u, 1u, texels);
      Draw(
        [&](Renderer2D &renderer)
        {
          renderer.SetDropShadow({.Offset = {2.f, 0.f}, .Colour = blue});
          renderer.DrawImage(image, {0.f, 0.f, 2.f, 2.f});
        });
      CHECK(At(Read(), 3u, 0u) == Clear);
    }
  }

  TEST_CASE_METHOD(Renderer2DFixture, "Renderer2D: blurs paths' and strokes' shadows through their masks",
                   "[Gfx][Render2D][GPU]")
  {
    UseTarget(32u, 32u);
    const Rect2D rect {8.f, 6.f, 16.f, 20.f};
    Path2D path;
    path.AddRect(rect);

    // Masks are blurred in whole pixels, so they're compared a little more loosely.
    for (const float blur : {2.f, 8.f})
    {
      INFO("blur " << blur);
      Draw(
        [&](Renderer2D &renderer)
        {
          ShadowOnly(renderer, blur,
                     [&](Renderer2D &r) { r.FillPath(path, FillRule::NonZero, SRGBA<uint8> {255, 0, 0}); });
        });
      const double sigma = blur / 2.0;
      CHECK(AlphaMismatches(Read(), _width, 2, [&](double x, double y)
                            { return GaussianMass(8.0, 24.0, x, sigma) * GaussianMass(6.0, 26.0, y, sigma); })
            == 0u);
    }

    // A stroke's shadow is its outline's; a straight line's is its rect's.
    Path2D line;
    line.MoveTo({8.f, 16.f});
    line.LineTo({24.f, 16.f});
    Draw(
      [&](Renderer2D &renderer)
      {
        ShadowOnly(renderer, 4.f,
                   [&](Renderer2D &r) { r.StrokePath(line, {.Width = 4.f}, SRGBA<uint8> {255, 0, 0}); });
      });
    const auto stroked = Read();
    Draw(
      [&](Renderer2D &renderer)
      {
        ShadowOnly(renderer, 4.f, [&](Renderer2D &r)
                   { r.StrokeLine({8.f, 16.f}, {24.f, 16.f}, {.Width = 4.f}, SRGBA<uint8> {255, 0, 0}); });
      });
    const auto straight = Read();
    CHECK(AlphaMismatches(straight, _width, 1, [](double x, double y)
                          { return GaussianMass(8.0, 24.0, x, 2.0) * GaussianMass(14.0, 18.0, y, 2.0); })
          == 0u);
    uint32 differences = 0u;
    for (size_t i = 0u; i < stroked.size(); ++i)
    {
      differences += Near(stroked[i], straight[i], 2) ? 0u : 1u;
    }
    CHECK(differences == 0u);
  }

  TEST_CASE_METHOD(Renderer2DFixture, "Renderer2D: clips shadows and draws them in layers",
                   "[Gfx][Render2D][GPU]")
  {
    UseTarget(16u, 4u);
    Draw(
      [&](Renderer2D &renderer)
      {
        renderer.ClipRect({0.f, 0.f, 9.f, 4.f});
        renderer.BeginLayer(1.f);
        renderer.SetDropShadow({.Offset = {6.f, 0.f}, .Colour = SRGBA<uint8> {0, 0, 255}});
        renderer.FillRect({2.f, 0.f, 4.f, 2.f}, SRGBA<uint8> {255, 0, 0});
        renderer.EndLayer();
      });
    auto pixels = Read();
    CHECK(At(pixels, 3u, 0u) == Red);
    CHECK(At(pixels, 8u, 0u) == Blue);
    CHECK(At(pixels, 9u, 0u) == Clear);
  }

  TEST_CASE_METHOD(Renderer2DFixture, "Renderer2D: fills with repeated images", "[Gfx][Render2D][GPU]")
  {
    UseTarget(8u, 8u);
    constexpr Array<Pixel, 4> texels {Red, Green, Blue, White};
    auto image = Image(2u, 2u, texels);

    SECTION("Tiles repeat from pattern space's origin, as the image drawn tile by tile")
    {
      const ImagePattern2D pattern {
        .Texture = image, .TileSize = {2.f, 2.f}, .Filter = SamplerFilter::Nearest};
      Draw([&](Renderer2D &renderer) { renderer.FillRect({0.f, 0.f, 8.f, 8.f}, pattern); });
      const auto patterned = Read();
      Draw(
        [&](Renderer2D &renderer)
        {
          for (float y = 0.f; y < 8.f; y += 2.f)
          {
            for (float x = 0.f; x < 8.f; x += 2.f)
            {
              renderer.DrawImage(image, {x, y, 2.f, 2.f}, {.Filter = SamplerFilter::Nearest});
            }
          }
        });
      CHECK(patterned == Read());
    }

    SECTION("Pattern space moves and scales the tiles")
    {
      const ImagePattern2D pattern {.Texture = image,
                                    .PatternSpace =
                                      Transform2D::Translation(1.f, 0.f) * Transform2D::Scale(2.f, 2.f),
                                    .TileSize = {2.f, 2.f},
                                    .Filter = SamplerFilter::Nearest};
      Draw([&](Renderer2D &renderer) { renderer.FillRect({0.f, 0.f, 8.f, 8.f}, pattern); });
      auto pixels = Read();
      // Each texel covers two by two pixels; the tile starts at x = 1, so x = 0 is the tile before's last
      // column.
      CHECK(At(pixels, 1u, 0u) == Red);
      CHECK(At(pixels, 2u, 1u) == Red);
      CHECK(At(pixels, 0u, 0u) == Green);
      CHECK(At(pixels, 3u, 0u) == Green);
      CHECK(At(pixels, 1u, 2u) == Blue);
      CHECK(At(pixels, 3u, 3u) == White);
      CHECK(At(pixels, 5u, 0u) == Red);
    }

    SECTION("Spacing leaves transparent gaps between tiles")
    {
      const ImagePattern2D pattern {
        .Texture = image, .TileSize = {2.f, 2.f}, .Spacing = {2.f, 0.f}, .Filter = SamplerFilter::Nearest};
      Draw([&](Renderer2D &renderer) { renderer.FillRect({0.f, 0.f, 8.f, 8.f}, pattern); });
      auto pixels = Read();
      CHECK(At(pixels, 0u, 0u) == Red);
      CHECK(At(pixels, 1u, 1u) == White);
      CHECK(At(pixels, 2u, 0u) == Clear);
      CHECK(At(pixels, 3u, 3u) == Clear);
      CHECK(At(pixels, 4u, 0u) == Red);
      CHECK(At(pixels, 4u, 2u) == Red);
    }

    SECTION("Filtering wraps across tile edges, so tiles join without seams")
    {
      // A uniform tile at a fractional scale and offset: tile by tile, its antialiased or clamped edges would
      // show; as a pattern, every pixel is the tile's colour.
      constexpr Array<Pixel, 4> blue {Blue, Blue, Blue, Blue};
      auto uniform = Image(2u, 2u, blue);
      const ImagePattern2D pattern {.Texture = uniform,
                                    .PatternSpace =
                                      Transform2D::Translation(0.3f, 0.7f) * Transform2D::Scale(0.76f, 0.61f),
                                    .TileSize = {2.f, 2.f}};
      Draw([&](Renderer2D &renderer) { renderer.FillRect({0.f, 0.f, 8.f, 8.f}, pattern); });
      for (const auto &pixel : Read())
      {
        CHECK(Near(pixel, Blue));
      }
    }

    SECTION("A tile that isn't repeated clamps filtering to its edges")
    {
      // Scaled to fill the row exactly, the first pixel samples the tile's left edge: wrapping blends in the
      // right edge, clamping doesn't.
      constexpr Array<Pixel, 2> redBlue {Red, Blue};
      auto row = Image(2u, 1u, redBlue);
      auto first = [&](bool repeat)
      {
        const ImagePattern2D pattern {.Texture = row,
                                      .PatternSpace = Transform2D::Scale(4.f, 8.f),
                                      .TileSize = {2.f, 1.f},
                                      .RepeatX = repeat,
                                      .RepeatY = repeat};
        Draw([&](Renderer2D &renderer) { renderer.FillRect({0.f, 0.f, 8.f, 8.f}, pattern); });
        return At(Read(), 0u, 0u);
      };
      CHECK(first(false) == Red);
      CHECK(first(true)[2] > 0u);
    }

    SECTION("Patterns paint paths and fade by their opacity")
    {
      const ImagePattern2D pattern {
        .Texture = image, .TileSize = {2.f, 2.f}, .Filter = SamplerFilter::Nearest, .Opacity = 0.5f};
      Path2D path;
      path.AddRect({0.f, 0.f, 4.f, 4.f});
      Draw([&](Renderer2D &renderer) { renderer.FillPath(path, FillRule::NonZero, pattern); });
      auto pixels = Read();
      CHECK(Near(At(pixels, 0u, 0u), {128, 0, 0, 128}));
      CHECK(Near(At(pixels, 3u, 3u), {128, 128, 128, 128}));
      CHECK(At(pixels, 5u, 5u) == Clear);
    }
  }

  TEST_CASE_METHOD(Renderer2DFixture, "Renderer2D: draws what another renderer draws in the same frame",
                   "[Gfx][Render2D][GPU]")
  {
    UseTarget(4u, 4u);
    auto offscreen = _device->CreateTexture({.Format = TextureFormat::RGBA8Unorm,
                                             .Width = 2u,
                                             .Height = 2u,
                                             .Usage = TextureUsage::RenderTarget | TextureUsage::Sampled,
                                             .DebugName = u8"Offscreen"});
    REQUIRE(offscreen.has_value());
    _images.push_back(*offscreen);
    auto other = Renderer2D::Create(*_device);
    REQUIRE(other.has_value());

    Draw(
      [&](Renderer2D &renderer)
      {
        // Drawn after this frame began, but recorded at its own End, before this frame's passes.
        REQUIRE(renderer.GetCommandBuffer() != nullptr);
        (*other)->Begin(*renderer.GetCommandBuffer(), {.Texture = *offscreen,
                                                       .Width = 2u,
                                                       .Height = 2u,
                                                       .ClearColour = Array<float, 4> {0.f, 0.f, 0.f, 0.f}});
        (*other)->FillRect({0.f, 0.f, 1.f, 2.f}, SRGBA<uint8> {255, 0, 0, 255});
        (*other)->FillRect({1.f, 0.f, 1.f, 2.f}, SRGBA<uint8> {0, 0, 255, 255});
        (*other)->End();
        renderer.DrawImage(*offscreen, {0.f, 0.f, 4.f, 4.f}, {.Filter = SamplerFilter::Nearest});
      });
    auto pixels = Read();
    CHECK(At(pixels, 0u, 0u) == Red);
    CHECK(At(pixels, 1u, 3u) == Red);
    CHECK(At(pixels, 2u, 0u) == Blue);
    CHECK(At(pixels, 3u, 3u) == Blue);
  }
}
