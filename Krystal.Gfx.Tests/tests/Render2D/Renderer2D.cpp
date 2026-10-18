#include "Krystal.Gfx/Render2D/Renderer2D.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Core/Types/Array.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Span.hpp"
#include "Krystal.Core/Utils/Move.hpp"
#include "Krystal.Gfx.Tests/TestDevice.hpp"
#include "Krystal.Gfx/Render2D/PathRasteriser.hpp"
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
    // A unorm target blends the encoded values: half red over blue.
    UseTarget(2u, 2u);
    Draw([](Renderer2D &renderer) { renderer.FillRect({0.f, 0.f, 2.f, 2.f}, SRGBA<uint8> {255, 0, 0, 128}); },
         Array<float, 4> {0.f, 0.f, 1.f, 1.f});
    CHECK(Near(Read()[0], {128, 0, 127, 255}));

    // An sRGB target blends in linear light: half white over black is half the light, encoded as 188.
    UseTarget(2u, 2u, TextureFormat::RGBA8Srgb);
    Draw([](Renderer2D &renderer)
         { renderer.FillRect({0.f, 0.f, 2.f, 2.f}, SRGBA<uint8> {255, 255, 255, 128}); },
         Array<float, 4> {0.f, 0.f, 0.f, 1.f});
    CHECK(Near(Read()[0], {188, 188, 188, 255}));

    // Opaque colours come back as they went in.
    Draw([](Renderer2D &renderer) { renderer.FillRect({0.f, 0.f, 2.f, 2.f}, SRGBA<uint8> {200, 100, 50}); });
    CHECK(Near(Read()[0], {200, 100, 50, 255}));
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

    SECTION("A clip covers the pixels whose centres are inside it")
    {
      Draw(
        [](Renderer2D &renderer)
        {
          renderer.ClipRect({0.6f, 1.f, 2.f, 2.f});
          renderer.FillRect({0.f, 0.f, 4.f, 4.f}, SRGBA<uint8> {255, 0, 0});
        });

      auto pixels = Read();
      CHECK(At(pixels, 0u, 1u) == Clear);
      CHECK(At(pixels, 1u, 1u) == Red);
      CHECK(At(pixels, 2u, 2u) == Red);
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

  TEST_CASE_METHOD(Renderer2DFixture, "Renderer2D: converts images to the target's encoding",
                   "[Gfx][Render2D][GPU]")
  {
    constexpr Array<Pixel, 1> texels {Pixel {188, 188, 188, 255}};

    SECTION("An sRGB texture into a unorm target")
    {
      UseTarget(2u, 2u);
      auto image = Image(1u, 1u, texels, TextureFormat::RGBA8Srgb);
      Draw([&](Renderer2D &renderer) { renderer.DrawImage(image, {0.f, 0.f, 2.f, 2.f}); });
      CHECK(Near(Read()[0], texels[0]));
    }

    SECTION("A unorm texture into an sRGB target")
    {
      UseTarget(2u, 2u, TextureFormat::RGBA8Srgb);
      auto image = Image(1u, 1u, texels);
      Draw([&](Renderer2D &renderer) { renderer.DrawImage(image, {0.f, 0.f, 2.f, 2.f}); });
      CHECK(Near(Read()[0], texels[0]));
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
}
